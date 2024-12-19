#include "CursorSingleton.hpp"

#include <cstdint>
#include <vector>

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <openvic-simulation/utility/Logger.hpp>
#include <openvic-extension/utility/ClassBindings.hpp>
#include <openvic-extension/utility/Utilities.hpp>

using namespace godot;
using namespace OpenVic;

void CursorSingleton::_bind_methods() {
	OV_BIND_METHOD(CursorSingleton::load_cursors);
	OV_BIND_METHOD(CursorSingleton::get_frames,{"cursor_name","resolution_index"},"normal",0);
	OV_BIND_METHOD(CursorSingleton::get_hotspots,{"cursor_name","resolution_index"},"normal",0);
	OV_BIND_METHOD(CursorSingleton::get_animationLength,{"cursor_name"},"normal");
	OV_BIND_METHOD(CursorSingleton::get_displayRates,{"cursor_name"},"normal");
	OV_BIND_METHOD(CursorSingleton::get_sequence,{"cursor_name"},"normal");
	OV_BIND_METHOD(CursorSingleton::get_resolutions,{"cursor_name"},"normal");
	OV_BIND_METHOD(CursorSingleton::generate_resolution,{"cursor_name","base_resolution_index", "target_resolution"},"normal",0,Vector2(64,64));
	OV_BIND_METHOD(CursorSingleton::get_cursor_names);

	ADD_PROPERTY(PropertyInfo(
		Variant::ARRAY,
		"cursor_names", PROPERTY_HINT_ARRAY_TYPE,
		"StringName"),
	"", "get_cursor_names");
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

TypedArray<StringName> CursorSingleton::get_cursor_names() const {
	return cursor_names;
}

TypedArray<ImageTexture> CursorSingleton::get_frames(StringName const& name, int32_t res_index) const {
	const cursor_map_t::const_iterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, vformat("Cursor \"%s\" not found", name));

	std::vector<TypedArray<ImageTexture>> const& images = it->second.images;
	ERR_FAIL_INDEX_V_MSG(res_index, images.size(), {}, vformat("Invalid image index for cursor \"%s\": %d", name, res_index));

	return images[res_index];
}

PackedVector2Array CursorSingleton::get_hotspots(StringName const& name, int32_t res_index) const {
	const cursor_map_t::const_iterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, vformat("Cursor \"%s\" not found", name));

	std::vector<PackedVector2Array> const& hotspots = it->second.hotspots;
	ERR_FAIL_INDEX_V_MSG(res_index, hotspots.size(), {}, vformat("Invalid hotspot index for cursor \"%s\": %d", name, res_index));

	return hotspots[res_index];
}

int32_t CursorSingleton::get_animationLength(StringName const& name) const {
	const cursor_map_t::const_iterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, vformat("Cursor \"%s\" not found", name));
	
	return it->second.animationLength;
}

PackedVector2Array CursorSingleton::get_resolutions(StringName const& name) const {
	const cursor_map_t::const_iterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, vformat("Cursor \"%s\" not found", name));

	return it->second.resolutions;
}

PackedFloat32Array CursorSingleton::get_displayRates(StringName const& name) const {
	const cursor_map_t::const_iterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, vformat("Cursor \"%s\" not found", name));

	return it->second.displayRates.value_or(PackedFloat32Array());
}

PackedInt32Array CursorSingleton::get_sequence(StringName const& name) const {
	const cursor_map_t::const_iterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, vformat("Cursor \"%s\" not found", name));

	return it->second.sequence.value_or(PackedInt32Array());
}

void CursorSingleton::generate_resolution(StringName const& name, int32_t base_res_index, Vector2 target_res) {
	cursor_map_t::iterator it = cursors.find(name);
	ERR_FAIL_COND_MSG(it == cursors.end(), vformat("Cursor \"%s\" not found", name));
	cursor_asset_t& cursor = it.value();

	ERR_FAIL_INDEX_MSG(
		base_res_index, cursor.images.size(), vformat("Invalid image index for cursor \"%s\": %d", name, base_res_index)
	);

	TypedArray<ImageTexture> const& images = cursor.images[base_res_index];
	PackedVector2Array const& hotspots = cursor.hotspots[base_res_index];
	const Vector2 resolution = cursor.resolutions[base_res_index];
	TypedArray<ImageTexture> new_frameset;
	PackedVector2Array new_hotspots;

	for (size_t index = 0; index < images.size(); index++) {
		Ref<ImageTexture> const& texture = images[index];
		Ref<Image> image;
		image.instantiate();
		image->copy_from(texture->get_image());
		image->resize(target_res.x, target_res.y, Image::INTERPOLATE_BILINEAR);
		new_frameset.push_back(ImageTexture::create_from_image(image));
		new_hotspots.push_back((hotspots[index] * target_res / resolution).floor());
	}

	cursor.images.push_back(new_frameset);
	cursor.hotspots.push_back(new_hotspots);
	cursor.resolutions.push_back(target_res);
}

static constexpr std::string_view cursor_directory = "gfx/cursors";

static String to_define_file_name(String const& path) {
	static const String backslash = "\\";
	static const String forwardslash = "/";
	static const String cursor_directory_forwardslash = Utilities::std_to_godot_string(cursor_directory) + forwardslash;
	static const String dot = ".";
	return path.replace(backslash, forwardslash).get_slice(cursor_directory_forwardslash, 1).get_slice(dot, 0);
}

Error CursorSingleton::load_cursors() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V_MSG(game_singleton, FAILED, "Error retrieving GameSingleton");

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
	
	if (cursor_files.empty() && animated_cursor_files.empty()){
		Logger::error("failed to load cursors: no files in cursors directory");
		return FAILED;
	}

	Error ret = OK;

	for(fs::path const& file_name : cursor_files) {
		String file = Utilities::std_to_godot_string(file_name.string());
		StringName name = to_define_file_name(file);

		if (!_load_cursor_cur(name,file)){
			Logger::error("failed to load normal cursor at path ", file_name);
			ret = FAILED;
		}
	}

	for(fs::path const& file_name : animated_cursor_files) {
		String file = Utilities::std_to_godot_string(file_name.string());
		StringName name = to_define_file_name(file);

		if (!_load_cursor_ani(name,file)){
			Logger::error("failed to load animated cursor at path ", file_name);
			ret = FAILED;
		}
	}

	return ret;
}

static constexpr int32_t _reverser_lookup[] {
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
	0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
};
static constexpr int32_t _reverse_bits(int32_t byte, int32_t bitsPerPixel=8) {
	int32_t a = _reverser_lookup[(byte & 0b1111)] << 4;
	int32_t b = _reverser_lookup[byte >> 4];
	int32_t c = b | a;
	return c >> (8-bitsPerPixel);
}

static constexpr int32_t _rotate_right(int32_t byte, int32_t size=8) {
	return ((byte & 0b1) << (size-1)) | (byte >> 1);
}

static int32_t _load_int_256(Ref<FileAccess> const& file) {
	int32_t value = file->get_8();
	if (value == 0) value = 256;
	return value;
}

static constexpr int32_t _get_row_start(int32_t x_coord, int32_t y_coord, int32_t bitsPerPixel) {
	x_coord *= bitsPerPixel;
	int32_t rowCount = (x_coord + 31) >> 5;
	return rowCount * y_coord * 4; // 4 bytes per row * rows down
}

static constexpr int32_t _select_bits(uint8_t const* data, int32_t rowStart, int32_t firstBit, int32_t bitCount) {
	int32_t byteIndex = firstBit >> 3;
	int32_t bitInByteIndex = firstBit & 0b111;
	if (bitInByteIndex + bitCount > 8) {
		Logger::error("Attempted to select bits outside of a byte.");
		return 0;
	}
	int32_t byte = _reverse_bits(*(data+rowStart+byteIndex));
	int32_t selected = (byte >> bitInByteIndex) & ((1 << bitCount) - 1);

	//TODO: questionable hack, nothing in the spec suggests we should need to do this
	if (bitCount > 1 && selected != 0){
		return _rotate_right(selected,4);
	}
	return selected;
}

static constexpr bool _read_AND_mask(
	uint8_t const* data, int32_t pixel_x, int32_t pixel_y, int32_t x_dimension, int32_t offset) {
	
	int32_t rowStart = _get_row_start(x_dimension, pixel_y, 1);
	int32_t andBit = _select_bits(data, rowStart + offset,pixel_x, 1);
	return !andBit;
}

static void _pixel_palette_lookup(
		PackedByteArray const& data, PackedByteArray& pixel_data, uint32_t i,
		PackedByteArray const& palette, int32_t coord_x, int32_t coord_y, int32_t x_dimension,
		int32_t offset, bool transparent, int32_t bitsPerPixel) {
	
	int32_t rowStart = _get_row_start(x_dimension, coord_y, bitsPerPixel);
	int32_t pixelBits = _select_bits(data.ptr(), rowStart + offset, coord_x*bitsPerPixel, bitsPerPixel);
	
	if ((pixelBits+1)*4 > palette.size()){
		Logger::error("attempted to select invalid colour palette entry, ", pixelBits);
		return;
	}
	
	//pixel bits serves as an index into the colour palette. We need to multiply the index by the number of bytes per colour (4)
	pixel_data[(i*4) + 0] = palette[pixelBits*4 + 0];
	pixel_data[(i*4) + 1] = palette[pixelBits*4 + 1];
	pixel_data[(i*4) + 2] = palette[pixelBits*4 + 2];
	pixel_data[(i*4) + 3] = 0xFF * transparent; //a
}

/*
24bit pixel support here is questionable:
the spec (per daubnet) says we should pad bytes to end things on 32bit boundaries
but the singular example of a 24bit cursor found on the internet does things like this.
So emit a warning when trying to load one of these
*/
static void _read_24bit_pixel(
	PackedByteArray const& imageData, PackedByteArray& pixel_data,
	int32_t i, int32_t offset, bool opaque) {

	if((i+1)*3 > imageData.size()){
		Logger::error("Pixel ", i, "tried to read from a pixel data array of max size ", pixel_data.size());
		return;
	}

	pixel_data[(i*4) + 0] = imageData[offset + (i*3) + 2]; //r
	pixel_data[(i*4) + 1] = imageData[offset + (i*3) + 1]; //g
	pixel_data[(i*4) + 2] = imageData[offset + (i*3) + 0]; //b
	pixel_data[(i*4) + 3] = 0xFF * opaque; //a
}

static void _read_32bit_pixel(
	PackedByteArray const& imageData, PackedByteArray& pixel_data,
	int32_t i, int32_t offset, bool opaque) {

	if((i+1)*4 > imageData.size()){
		Logger::error("Pixel ", i, "tried to read from a pixel data array of max size ", pixel_data.size());
		return;
	}

	pixel_data[(i*4) + 0] = imageData[offset + (i*4) + 2]; //r
	pixel_data[(i*4) + 1] = imageData[offset + (i*4) + 1]; //g
	pixel_data[(i*4) + 2] = imageData[offset + (i*4) + 0]; //b
	pixel_data[(i*4) + 3] = imageData[offset + (i*4) + 3] * opaque; //a
}

//used to load a .cur file from a file (could be the a whole .cur file, or a .cur within a .ani file)
static CursorSingleton::image_hotspot_pair_asset_t _load_pair(Ref<FileAccess> const& file) {
	CursorSingleton::image_hotspot_pair_asset_t pairs = {};

	//.cur's within .anis won't start of the beginning of the file, so save where they start
	int32_t baseOffset = file->get_position();

	//.cur header
	int32_t reserved = file->get_16();
	int32_t type = file->get_16(); //1=ico, 2=cur
	int32_t imagesCount = file->get_16();
	
	//all the images
	for(int32_t i=0; i<imagesCount; i++){
		Vector2i dimensions = Vector2i(_load_int_256(file),_load_int_256(file)); //TODO
		int32_t palette = file->get_8();
		int32_t imgReserved = file->get_8();
		
		Vector2i hotspot = Vector2i();
		hotspot.x = file->get_16();
		hotspot.y = file->get_16();

		int32_t dataSize = std::min(static_cast<uint64_t>(file->get_32()), file->get_length() - file->get_position());
		int32_t dataOffset = file->get_32();

		//This image header information is sequential in the data, but the images aren't necessarily
		// so save the current position, get the image data and return so we're ready for the next image header
		int32_t endOfImageHeader = file->get_position();

		file->seek(dataOffset+baseOffset);
		PackedByteArray const& imageData = file->get_buffer(dataSize);
		file->seek(endOfImageHeader);

		Ref<Image> image = Ref<Image>();
		image.instantiate();
	
		//PNGs are stored in their entirety, so use Godot's internal loader
		if (imageData.slice(1,4).get_string_from_ascii() == "PNG") {
			image->load_png_from_buffer(imageData);
		}
		else { //BMP based cursor, have to load this manually
			int32_t dibHeaderSize = imageData.decode_u32(0);

			//this is the combined sized of the picture and the transparency bitmask
			// (ex. 32x32 dimension image becomes 32x64 here)
			Vector2i combinedDimensions = Vector2i(imageData.decode_u32(4),imageData.decode_u32(8));
			int32_t colourPlanes = imageData.decode_u16(12);
			int32_t bitsPerPixel = imageData.decode_u16(14);
			if (bitsPerPixel <= 8 || bitsPerPixel == 24){
				Logger::warning("Attempting to import ", bitsPerPixel, "bit cursor, this isn't guaranteed to work");
			}
			else if (bitsPerPixel != 32){
				Logger::error("Invalid or Unsupported bits per pixel while loading cursor image, bpp: ", bitsPerPixel, "loading blank image instead");
			}

			int32_t size = imageData.decode_u32(20);
			Vector2i resolution = Vector2i(imageData.decode_s32(24),imageData.decode_s32(28));
			int32_t paletteSize = imageData.decode_u32(32);

			if (paletteSize == 0 && bitsPerPixel <= 8){
				paletteSize =  1 << bitsPerPixel;
			}
			int32_t importantColours = imageData.decode_u32(36);

			//for BMPs with 8 bits per pixel or less, the pixel data is actually a lookup to this table here
			PackedByteArray const& palette = imageData.slice(40,40+(4*paletteSize));

			// this is where the image data starts
			int32_t offset = 40 + paletteSize*4;

			//where the transparency AND mask starts
			int32_t maskOffset = offset + _get_row_start(dimensions.x,dimensions.y,bitsPerPixel);

			PackedByteArray pixelData = PackedByteArray();
			pixelData.resize(dimensions.x*dimensions.y*4);
			pixelData.fill(255);

			int32_t i=0;
			for(int32_t row=0; row < dimensions.y; row++) {
				for(int32_t col=0; col < dimensions.x; col++) {
					Vector2i coord = Vector2i(col,row);
					bool transparent = _read_AND_mask(
						imageData.ptr(),coord.x,coord.y,dimensions.x,maskOffset
					);
					if (bitsPerPixel <= 8){
						//mostly legacy files, these ones all use a lookup into the colour palette
						_pixel_palette_lookup(
							imageData, pixelData, i, palette, coord.x, coord.y, dimensions.x, offset, transparent, bitsPerPixel
						);
					}/*
					else if (bitsPerPixel == 16) { //TODO
						//Unsupported, error
					}*/
					else if (bitsPerPixel == 24) {
						//Support Questionable, based on 1 example on the internet as opposed to the actual spec
						_read_24bit_pixel(
							imageData, pixelData, i, offset, transparent
						);
					}
					else if (bitsPerPixel == 32) {
						//What vic actually uses
						_read_32bit_pixel(
							imageData, pixelData, i, offset, transparent
						);
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
		
		if (imageTexture.is_null()){
			Logger::error("Image Texture ",Utilities::godot_to_std_string(file->get_path())," was null!");
		}

		pairs.hotspots.push_back(hotspot);
		pairs.images.push_back(imageTexture);
		
	}
	return pairs;

}

bool CursorSingleton::_load_cursor_ani(StringName const& name, String const& path) {
	const Ref<FileAccess> file = FileAccess::open(path, FileAccess::ModeFlags::READ);

	const Error err = FileAccess::get_open_error();
	ERR_FAIL_COND_V_MSG(
		err != OK || file.is_null(), false, vformat("Failed to open ani file: \"%s\"", path)
	);

	//read the RIFF container
	Utilities::read_riff_str(file); //riff_id
	const uint64_t riff_size = std::min(static_cast<uint64_t>(file->get_32()), file->get_length());;
	Utilities::read_riff_str(file); //form_type

	//important variables
	std::vector<TypedArray<ImageTexture>> frames_by_resolution;
	std::vector<PackedVector2Array> hotspots_by_resolution;

	PackedVector2Array resolutions;
	PackedFloat32Array displayRates;
	PackedInt32Array sequence;

	//ani header variables
	int32_t numFrames = 1;
	int32_t numSteps = 1;
	Vector2i dimensions = Vector2i(1,1);
	int32_t bitCount = 1;
	int32_t numPlanes = 1; //???
	int32_t displayRate = 1; //how long each frame should last
	int32_t flags = 0;
	bool iconFlag = false;
	bool sequenceFlag = false;


	while(file->get_position() < riff_size){
		String id = Utilities::read_riff_str(file);
		int32_t size = file->get_32();
		if (id == "LIST"){
			String list_type = Utilities::read_riff_str(file);
		}
		else if (id == "anih"){
			//hack for some files, there's likely a better way
			if (size == 36){ 
				int32_t headerSize = file->get_32();
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
		else if (id == "icon"){

			int32_t file_access_offset = file->get_position();

			image_hotspot_pair_asset_t pair = _load_pair(file);
			//basically pushback an array

			//only store the resolutions from one frame
			if (resolutions.is_empty()){
				for(int32_t i=0;i<pair.images.size();i++){
					
					PackedVector2Array hotspots;
					TypedArray<ImageTexture> images;
					images.push_back(pair.images[i]);
					hotspots.push_back(pair.hotspots[i]);
					frames_by_resolution.push_back(images);
					hotspots_by_resolution.push_back(hotspots);

					resolutions.push_back(Vector2(pair.images[i]->get_width(),pair.images[i]->get_height()));
				}

			}
			else {
				if (pair.images.size() != frames_by_resolution.size()){
					Logger::error(
						"Malformatted .ani cursor file ",
						Utilities::godot_to_std_string(name),
						" had inconsistent number of images per cursor"
					);
				}
				for(int32_t i=0; i<pair.images.size(); i++){
					frames_by_resolution[i].push_back(pair.images[i]);
					hotspots_by_resolution[i].push_back(pair.hotspots[i]);
				}
			}

			//cursor could have been anywhere in the file, come back to a known position
			file->seek(file_access_offset + size);

		}
		else if (id == "seq "){
			for(int32_t i=0; i<numSteps; i++){
				sequence.push_back(file->get_32());
			}
		}
		else if (id == "rate"){
			for(int32_t i=0;i<numSteps;i++){
				displayRates.push_back(file->get_32()/60.0);
			}
		}
		else {
			//Various junk (JUNK, metadata we don't care about, ...)
			file->get_buffer(size);
		}
		//align to even bytes
		if ((file->get_position() & 1) != 0){
			file->get_8();
		}
	}

	//not all ani files have the sequence and rate chunks, if not, fill out these properties
	//manually
	if (sequence.is_empty()){
		for(int32_t i=0; i<numSteps;i++){
			sequence.push_back(i);
		}
	}
	if (displayRates.is_empty()){
		for(int32_t i=0; i<numSteps;i++){
			displayRates.push_back(displayRate/60.0);
		}
	}

	cursors.emplace(
		name,
		cursor_asset_t {
			std::move(hotspots_by_resolution),
			std::move(frames_by_resolution),
			resolutions,
			static_cast<int32_t>(sequence.size()),
			displayRates,
			sequence
		}
	);
	cursor_names.append(name);
	
	return true;
}

bool CursorSingleton::_load_cursor_cur(StringName const& name, String const& path) {
	const Ref<FileAccess> file = FileAccess::open(path, FileAccess::ModeFlags::READ);
	const Error err = FileAccess::get_open_error();
	ERR_FAIL_COND_V_MSG(
		err != OK || file.is_null(), false, vformat("Failed to open cur file: \"%s\"", path)
	);

	image_hotspot_pair_asset_t pair = _load_pair(file);
	
	std::vector<TypedArray<ImageTexture>> frames_by_resolution;
	std::vector<PackedVector2Array> hotspots_by_resolution;

	PackedVector2Array resolutions;

	for(int32_t i=0;i<pair.images.size();i++){
		resolutions.push_back(Vector2(pair.images[i]->get_width(),pair.images[i]->get_height()));
		
		TypedArray<ImageTexture> frames;
		frames.push_back(pair.images[i]);
		frames_by_resolution.push_back(frames);

		PackedVector2Array hotspots;
		hotspots.push_back(pair.hotspots[i]);
		hotspots_by_resolution.push_back(hotspots);
	}

	cursors.emplace(
		name,
		cursor_asset_t {
			std::move(hotspots_by_resolution),
			std::move(frames_by_resolution),
			resolutions,
			1
		}
	);
	cursor_names.append(name);

	return true;
}