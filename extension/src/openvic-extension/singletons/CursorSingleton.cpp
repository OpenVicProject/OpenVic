#include "CursorSingleton.hpp"

#include <vector>

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

#include <openvic-extension/utility/Utilities.hpp>
#include <openvic-extension/utility/ClassBindings.hpp>

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_to_godot_string;

using namespace godot;
using namespace OpenVic;
using namespace OpenVic::NodeTools;

void CursorSingleton::_bind_methods() {
	OV_BIND_METHOD(CursorSingleton::load_cursors);
	OV_BIND_METHOD(CursorSingleton::get_frames,"normal",0);
	OV_BIND_METHOD(CursorSingleton::get_hotspots,"normal",0);
	OV_BIND_METHOD(CursorSingleton::get_animationLength,"normal");
	OV_BIND_METHOD(CursorSingleton::get_displayRates,"normal");
	OV_BIND_METHOD(CursorSingleton::get_sequence,"normal");
	OV_BIND_METHOD(CursorSingleton::get_resolutions,"normal");
	OV_BIND_METHOD(CursorSingleton::generate_resolution,"normal",0,Vector2i(64,64));
}

CursorSingleton* CursorSingleton::get_singleton() {
	return _singleton;
}

CursorSingleton::CursorSingleton() {
	ERR_FAIL_COND(_singleton != nullptr);
	_singleton = this;
}

CursorSingleton::~CursorSingleton() {
	ERR_FAIL_COND(_singleton != this);
	_singleton = nullptr;
}


Array CursorSingleton::get_frames(String const& name, int res_index){
	if(res_index < cursors[name].images.size()){
		return cursors[name].images[res_index];
	}
	return Array();
}

TypedArray<Vector2i> CursorSingleton::get_hotspots(String const& name, int res_index){
	if(res_index < cursors[name].images.size()){
		return cursors[name].hotspots[res_index];
	}
	return TypedArray<Vector2i>();
}

int CursorSingleton::get_animationLength(String const& name){
	return cursors[name].animationLength;
}

TypedArray<Vector2i> CursorSingleton::get_resolutions(String const& name){
	return cursors[name].resolutions;
}

TypedArray<float> CursorSingleton::get_displayRates(String const& name){
	if(cursors[name].displayRates.has_value()){
		return cursors[name].displayRates.value();
	}
	return TypedArray<float>();
}

TypedArray<int> CursorSingleton::get_sequence(String const& name){
	if(cursors[name].displayRates.has_value()){
		return cursors[name].sequence.value();
	}
	return TypedArray<int>();
}

void CursorSingleton::generate_resolution(String const& name, int base_res_index, Vector2i target_res){
	cursor_asset_t* cursor = &cursors[name];
	Array new_frameset;
	TypedArray<Vector2i> new_hotspots;

	for(int i=0;i<cursor->images[base_res_index].size();i++){

		Ref<ImageTexture> texture = static_cast<Ref<ImageTexture>>(cursor->images[base_res_index][i]);
		
		Ref<Image> image;
		image.instantiate();
		image->create(target_res.x, target_res.y, false, Image::Format::FORMAT_RGBA8);
		image->copy_from(texture->get_image());
		image->resize(target_res.x,target_res.y,godot::Image::INTERPOLATE_BILINEAR);


		Ref<ImageTexture> new_texture = Ref<ImageTexture>();
		new_texture.instantiate();
		new_texture->set_image(image);

		new_frameset.push_back(new_texture);

		Vector2i base_hotspot = cursor->hotspots[base_res_index][i];
		Vector2i scale_ratio = target_res / static_cast<Vector2i>(cursor->resolutions[base_res_index]);
		new_hotspots.push_back(base_hotspot*scale_ratio);

	}

	cursor->images.push_back(new_frameset);
	cursor->hotspots.push_back(new_hotspots);
	cursor->resolutions.push_back(target_res);
}

//, std::string_view const& base_folder
String CursorSingleton::to_define_file_name(String const& path) const {
	String name = path.replace("\\","/");
    return name.get_slice("gfx/cursors/",1).get_slice(".",0);
}

bool CursorSingleton::load_cursors() {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V_MSG(game_singleton, false, vformat("Error retrieving GameSingleton"));
	
	static constexpr std::string_view cursor_directory = "gfx/cursors";
	bool ret = true;

	//there is also a png file in the folder we don't want to bother loading
	//so don't just load everything in the directory

	//We need to load both ".cur" and ".CUR" files
	Dataloader::path_vector_t cursor_files = game_singleton->get_dataloader()
		.lookup_files_in_dir_recursive(cursor_directory, ".cur");

	Dataloader::path_vector_t CURsor_files = game_singleton->get_dataloader()
		.lookup_files_in_dir_recursive(cursor_directory, ".CUR");
	cursor_files.insert(std::end(cursor_files),std::begin(CURsor_files),std::end(CURsor_files));

	Dataloader::path_vector_t animated_cursor_files = game_singleton->get_dataloader()
		.lookup_files_in_dir_recursive(cursor_directory, ".ani");
	
	if(cursor_files.size() < 1 && animated_cursor_files.size() < 1){
		Logger::error("failed to load cursors: no files in cursors directory");
		ret = false;
	}

	for(std::filesystem::path const& file_name : cursor_files) {
		String file = std_to_godot_string(file_name.string()); //.stem()
		String name = to_define_file_name(file);

		if(!_load_cursor_cur(name,file)){
			Logger::error("failed to load normal cursor at path ",file_name);
			ret = false;
		}
	}
	for(std::filesystem::path const& file_name : animated_cursor_files) {
		String file = std_to_godot_string(file_name.string()); //.stem()
		String name = to_define_file_name(file);

		if(!_load_cursor_ani(name,file)){
			Logger::error("failed to load animated cursor at path ",file_name);
			ret = false;
		}
	}

	return ret;
}

bool CursorSingleton::_load_cursor_ani(String const& name, String const& path) {
	const Ref<FileAccess> file = FileAccess::open(path, FileAccess::ModeFlags::READ);

	//todo make the size min(file's size, file's declared size) just like sound
	//read the RIFF container
	String riff_id = read_riff_str(file);
	int riff_size = std::min(static_cast<uint64_t>(file->get_32()), file->get_length());;
	String form_type = read_riff_str(file);

	//important variables
	std::vector<Array> frames_by_resolution;
	std::vector<TypedArray<Vector2i>> hotspots_by_resolution;

	TypedArray<Vector2i> resolutions;
	TypedArray<float> displayRates;
	TypedArray<int> sequence;

	//ani header variables
	int numFrames;
	int numSteps;
	Vector2i dimensions;
	int bitCount;
	int numPlanes; //???
	int displayRate; //how long each frame should last
	int flags;
	bool iconFlag;
	bool sequenceFlag;


	while(file->get_position() < riff_size){
		String id = read_riff_str(file);
		int size = file->get_32();
		if(id == "LIST"){
			String list_type = read_riff_str(file);
		}
		else if(id == "anih"){
			//hack for some files, there's likely a better way
			if(size == 36){ 
				int headerSize = file->get_32();
			}
			numFrames = file->get_32();
			numSteps = file->get_32();
			dimensions = Vector2i(file->get_32(),file->get_32());
			bitCount = file->get_32();
			numPlanes = file->get_32();
			displayRate = file->get_32();
			flags = file->get_32();
			iconFlag = flags & 0x1;
			sequenceFlag = flags & 0x2;
		}
		else if(id == "icon"){

			int file_access_offset = file->get_position();

			image_hotspot_pair_asset_t pair = _load_pair(file);
			//basically pushback an array

			//only store the resolutions from one frame
			if(resolutions.is_empty()){
				for(int i=0;i<pair.images.size();i++){
					
					TypedArray<Vector2i> hotspots;
					Array images;
					images.push_back(pair.images[i]);
					hotspots.push_back(pair.hotspots[i]);
					frames_by_resolution.push_back(images);
					hotspots_by_resolution.push_back(hotspots);

					resolutions.push_back(Vector2i(pair.images[i]->get_width(),pair.images[i]->get_height()));
				}

			}
			else {
				for(int i=0;i<pair.images.size();i++){
					frames_by_resolution[i].push_back(pair.images[i]);
					hotspots_by_resolution[i].push_back(pair.hotspots[i]);
				}
			}

			//cursor could have been anywhere in the file, come back to a known position
			file->seek(file_access_offset + size);

		}
		else if(id == "seq "){
			for(int i=0;i<numSteps;i++){
				sequence.push_back(file->get_32());
			}
		}
		else if(id == "rate"){
			for(int i=0;i<numSteps;i++){
				displayRates.push_back(file->get_32()/60.0);
			}
		}
		else {
			//Various junk (JUNK, metadata we don't care about, ...)
			file->get_buffer(size);
		}
		//align to even bytes
		if(file->get_position() % 2 != 0){
			file->get_8();
		}
	}

	//not all ani files have the sequence and rate chunks, if not, fill out these properties
	//manually
	if(sequence.size() == 0){
		for(int i=0; i<numSteps;i++){
			sequence.push_back(i);
		}
	}
	if(displayRates.size() == 0){
		for(int i=0; i<numSteps;i++){
			displayRates.push_back(displayRate/60.0);
		}
	}

    cursor_asset_t cursor = {
		hotspots_by_resolution,
		frames_by_resolution,
		resolutions,
		static_cast<int>(sequence.size())
	};

	cursor.displayRates = displayRates;
	cursor.sequence = sequence;
	cursors.emplace(std::move(name),cursor);
	
	return true;
}

//set size if its an info string, otherwise leaving
String CursorSingleton::read_riff_str(Ref<FileAccess> const& file, int size) const {
	return file->get_buffer(size).get_string_from_ascii();
}

bool CursorSingleton::_load_cursor_cur(String const& name, String const& path) {
    const Ref<FileAccess> file = FileAccess::open(path, FileAccess::ModeFlags::READ);
	image_hotspot_pair_asset_t pair = _load_pair(file);
	
	std::vector<Array> frames_by_resolution;
	std::vector<TypedArray<Vector2i>> hotspots_by_resolution;

	TypedArray<Vector2i> resolutions;

	for(int i=0;i<pair.images.size();i++){
		resolutions.push_back(Vector2i(pair.images[i]->get_width(),pair.images[i]->get_height()));
		
		Array frames;
		frames.push_back(pair.images[i]);
		frames_by_resolution.push_back(frames);

		TypedArray<Vector2i> hotspots;
		hotspots.push_back(pair.hotspots[i]);
		hotspots_by_resolution.push_back(hotspots);
	}

    cursor_asset_t cursor = {
		hotspots_by_resolution,
		frames_by_resolution,
		resolutions,
		1
		};
    cursors.emplace(std::move(name),cursor);
	return true;
}

//used to load a .cur file from a file (could be the a whole .cur file, or a .cur within a .ani file)
CursorSingleton::image_hotspot_pair_asset_t CursorSingleton::_load_pair(Ref<FileAccess> const& file) {
	image_hotspot_pair_asset_t pairs = {};

	//.cur's within .anis won't start of the beginning of the file, so save where they start
	int baseOffset = file->get_position();

	//.cur header
	int reserved = file->get_16();
	int type = file->get_16(); //1=ico, 2=cur
	int imagesCount = file->get_16();
	
	//all the images
	for(int i=0; i<imagesCount; i++){
		Vector2i dimensions = Vector2i(_load_int_256(file),_load_int_256(file)); //TODO
		int palette = file->get_8();
		int imgReserved = file->get_8();
		
		Vector2i hotspot = Vector2i();
		hotspot.x = file->get_16();
		hotspot.y = file->get_16();

		int dataSize = std::min(static_cast<uint64_t>(file->get_32()), file->get_length() - file->get_position());
		int dataOffset = file->get_32();

		//This image header information is sequential in the data, but the images aren't necessarily
		// so save the current position, get the image data and return so we're ready for the next image header
		int endOfImageHeader = file->get_position();

		file->seek(dataOffset+baseOffset);
		PackedByteArray const& imageData = file->get_buffer(dataSize);
		file->seek(endOfImageHeader);

		Ref<Image> image = Ref<Image>();
		image.instantiate();
	
		//PNGs are stored in their entirety, so use Godot's internal loader
		if(imageData.slice(1,4).get_string_from_ascii() == "PNG") {
			image->load_png_from_buffer(imageData);
		}
		else { //BMP based cursor, have to load this manually
			int dibHeaderSize = imageData.decode_u32(0);

			//this is the combined sized of the picture and the transparency bitmask
			// (ex. 32x32 dimension image becomes 32x64 here)
			Vector2i combinedDimensions = Vector2i(imageData.decode_u32(4),imageData.decode_u32(8));
			int colourPlanes = imageData.decode_u16(12);
			int bitsPerPixel = imageData.decode_u16(14);
			if(bitsPerPixel <= 8 || bitsPerPixel == 24){
				Logger::warning("Attempting to import ", bitsPerPixel, "bit cursor, this may not be supported");
			}
			else if(bitsPerPixel != 32){
				Logger::error("Invalid or Unsupported bits per pixel while loading cursor image, bpp: ", bitsPerPixel, "loading blank, transparent image instead");
			}

			int size = imageData.decode_u32(20);
			Vector2i resolution = Vector2i(imageData.decode_s32(24),imageData.decode_s32(28));
			int paletteSize = imageData.decode_u32(32);

			if(paletteSize == 0 && bitsPerPixel <= 8){
				paletteSize =  static_cast<int>(pow(2, bitsPerPixel));
			}
			int importantColours = imageData.decode_u32(36);

			//for BMPs with 8 bits per pixel or less, the pixel data is actually a lookup to this table here
			PackedByteArray const& palette = imageData.slice(40,40+(4*paletteSize));

			// this is where the image data starts
			int offset = 40 + paletteSize*4;

			//where the transparency AND mask starts
			int maskOffset = offset + _get_row_start(dimensions,dimensions.y,bitsPerPixel); //TODO

			PackedByteArray pixelData = PackedByteArray();

			int i=0;
			for(int row=0; row < dimensions.y; row++) {
				for(int col=0; col < dimensions.x; col++) {
					Vector2i coord = Vector2i(col,row);
					bool transparent = _read_AND_mask(
						imageData,coord,dimensions,maskOffset
					);
					if(bitsPerPixel <= 8){
						//mostly legacy files, these ones all use a lookup into the colour palette
						pixelData.append_array(_pixel_palette_lookup(
							imageData,palette,coord,dimensions,offset,transparent,bitsPerPixel
						));
					}/*
					else if(bitsPerPixel == 16) { //TODO
						//Unsupported, error
					}*/
					else if(bitsPerPixel == 24) {
						//Support Questionable, based on 1 example on the internet as opposed to the actual spec
						pixelData.append_array(_read_24bit_pixel(
							i,offset,imageData,transparent
						));
					}
					else if(bitsPerPixel == 32) {
						//What vic actually uses
						pixelData.append_array(_read_32bit_pixel(
							i,offset,imageData,transparent
						));
					}
					else {
						//Invalid bitsPerPixel,
						//just give a blank pixel, we already emitted the appropriate error message
						pixelData.append(0);
						pixelData.append(0);
						pixelData.append(0);
						pixelData.append(0);
					}
					i++;
				}
			}
			
			image = image->create_from_data(dimensions.x,dimensions.y,false, Image::FORMAT_RGBA8,pixelData);
			//bmp images are stored bottom to top
			image->flip_y();
		}
		Ref<ImageTexture> imageTexture = Ref<ImageTexture>();
		imageTexture.instantiate();

        imageTexture = imageTexture->create_from_image(image);
		
		if(imageTexture.is_null()){
			Logger::error("Image Texture ",godot_to_std_string(file->get_path())," was null!");
		}

		pairs.hotspots.push_back(hotspot);
		pairs.images.push_back(imageTexture);
		
	}
	return pairs;

}

bool CursorSingleton::_read_AND_mask(PackedByteArray const& data, Vector2i pixelCoords, Vector2i dimensions, int offset){
	int rowStart = _get_row_start(dimensions, pixelCoords.y, 1);
	int andBit = _select_bits(data, rowStart + offset,pixelCoords.x, 1);
	return !andBit;
}

PackedByteArray CursorSingleton::_pixel_palette_lookup(PackedByteArray const& data, PackedByteArray const& palette, Vector2i coord, Vector2i dimensions, int offset, bool transparent, int bitsPerPixel){
	
	int rowStart = _get_row_start(dimensions, coord.y, bitsPerPixel);
	int pixelBits = _select_bits(data, rowStart + offset, coord.x*bitsPerPixel, bitsPerPixel);
	if((pixelBits+1)*4 > palette.size()){
		Logger::error("attempted to select invalid colour palette entry, ", pixelBits);
	}
	
	//pixel bits serves as an index into the colour palette. We need to multiply the index by the number of bytes per colour (4)
	PackedByteArray pixel = palette.slice(pixelBits*4,(pixelBits+1)*4);
	pixel[3] = 0xFF * transparent;
	return pixel;
}

/*
24bit pixel support here is questionable:
the spec (per daubnet) says we should pad bytes to end things on 32bit boundaries
but the singular example of a 24bit cursor found on the internet does things like this.
So emit a warning when trying to load one of these
*/
PackedByteArray CursorSingleton::_read_24bit_pixel(int i, int offset, PackedByteArray const& imageData, bool notTransparent) {
	PackedByteArray pixel = PackedByteArray();
	
	int b = imageData[offset + (i*3) + 0];
	int g = imageData[offset + (i*3) + 1];
	int r = imageData[offset + (i*3) + 2];

	pixel.append(r);
	pixel.append(g);
	pixel.append(b);
	pixel.append(0xFF*notTransparent);

	return pixel;
}

PackedByteArray CursorSingleton::_read_32bit_pixel(int i, int offset, PackedByteArray const& imageData, bool notTransparent) {
	PackedByteArray pixel = PackedByteArray();
	
	int b = imageData[offset + (i*4) + 0];
	int g = imageData[offset + (i*4) + 1];
	int r = imageData[offset + (i*4) + 2];
	int a = imageData[offset + (i*4) + 3] * notTransparent;

	pixel.append(r);
	pixel.append(g);
	pixel.append(b);
	pixel.append(a);

	return pixel;
}

int CursorSingleton::_get_row_start(Vector2i dimensions, int y_coord, int bitsPerPixel){
	int rowCount = dimensions.x * bitsPerPixel / 32;
	bool hasExtraRow = ((dimensions.x * bitsPerPixel) % 32) != 0;
	rowCount += 1*hasExtraRow;
	return rowCount*y_coord*4; //4 bytes per row * rows down
}

int CursorSingleton::_select_bits(PackedByteArray data, int rowStart, int firstBit, int bitCount){
	int bitmasks[] = {0b1 , 0b11, 0b111, 0b1111, 0b11111, 0b111111, 0b1111111, 0b11111111};
	int byteIndex = firstBit / 8;
	int bitInByteIndex = firstBit % 8;
	if(bitInByteIndex + bitCount > 8){
		Logger::error("Attempted to select bits outside of a byte.");
		return 0;
	}
	int byte = _reverse_bits(data[rowStart + byteIndex]);
	int selected = (byte >> bitInByteIndex) & bitmasks[bitCount-1];

	//TODO: questionable hack, nothing in the spec suggests we should need to do this
	if(bitCount > 1 && selected != 0){
		return _rotate_right(selected,4);
	}
	return selected;
}

int CursorSingleton::_reverse_bits(int byte, int bitsPerPixel){
	int reverser_lookup[] = {
		0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
		0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
	};
	int a = reverser_lookup[(byte & 0b1111)] << 4;
	int b = reverser_lookup[byte >> 4];
	int c = b | a;
	return c >> (8-bitsPerPixel);
}

int CursorSingleton::_rotate_right(int byte, int size){
	return ((byte & 0b1) << (size-1)) | (byte >> 1);
}

int CursorSingleton::_load_int_256(Ref<FileAccess> const& file){
	int value = file->get_8();
	if(value == 0) value = 256;
	return value;
}