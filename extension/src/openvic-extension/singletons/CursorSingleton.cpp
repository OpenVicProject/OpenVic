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
#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/Convert.hpp"
#include <openvic-extension/utility/Utilities.hpp>

#include <fmt/std.h>

using namespace godot;
using namespace OpenVic;

void CursorSingleton::_bind_methods() {
	OV_BIND_METHOD(CursorSingleton::load_cursors);
	OV_BIND_METHOD(CursorSingleton::get_frames, {"cursor_name","resolution_index"}, "normal", 0);
	OV_BIND_METHOD(CursorSingleton::get_hotspots, {"cursor_name","resolution_index"}, "normal", 0);
	OV_BIND_METHOD(CursorSingleton::get_animation_length, {"cursor_name"}, "normal");
	OV_BIND_METHOD(CursorSingleton::get_display_rates, {"cursor_name"}, "normal");
	OV_BIND_METHOD(CursorSingleton::get_sequence, {"cursor_name"}, "normal");
	OV_BIND_METHOD(CursorSingleton::get_resolutions, {"cursor_name"}, "normal");
	OV_BIND_METHOD(CursorSingleton::generate_resolution, {"cursor_name", "base_resolution_index", "target_resolution"}, "normal", 0, Vector2(64,64));
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
	const cursor_map_t::ConstIterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, Utilities::format("Cursor \"%s\" not found", name));

	std::vector<TypedArray<ImageTexture>> const& images = it->value.images;
	ERR_FAIL_INDEX_V_MSG(res_index, images.size(), {}, Utilities::format("Invalid image index for cursor \"%s\": %d", name, res_index));

	return images[res_index];
}

PackedVector2Array CursorSingleton::get_hotspots(StringName const& name, int32_t res_index) const {
	const cursor_map_t::ConstIterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, Utilities::format("Cursor \"%s\" not found", name));

	std::vector<PackedVector2Array> const& hotspots = it->value.hotspots;
	ERR_FAIL_INDEX_V_MSG(res_index, hotspots.size(), {}, Utilities::format("Invalid hotspot index for cursor \"%s\": %d", name, res_index));

	return hotspots[res_index];
}

int32_t CursorSingleton::get_animation_length(StringName const& name) const {
	const cursor_map_t::ConstIterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, Utilities::format("Cursor \"%s\" not found", name));
	
	return it->value.animation_length;
}

PackedVector2Array CursorSingleton::get_resolutions(StringName const& name) const {
	const cursor_map_t::ConstIterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, Utilities::format("Cursor \"%s\" not found", name));

	return it->value.resolutions;
}

PackedFloat32Array CursorSingleton::get_display_rates(StringName const& name) const {
	const cursor_map_t::ConstIterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, Utilities::format("Cursor \"%s\" not found", name));

	return it->value.display_rates;
}

PackedInt32Array CursorSingleton::get_sequence(StringName const& name) const {
	const cursor_map_t::ConstIterator it = cursors.find(name);
	ERR_FAIL_COND_V_MSG(it == cursors.end(), {}, Utilities::format("Cursor \"%s\" not found", name));

	return it->value.sequence;
}

void CursorSingleton::generate_resolution(StringName const& name, int32_t base_res_index, Vector2 target_res) {
	cursor_map_t::Iterator it = cursors.find(name);
	ERR_FAIL_COND_MSG(it == cursors.end(), Utilities::format("Cursor \"%s\" not found", name));
	cursor_asset_t& cursor = it->value;

	ERR_FAIL_INDEX_MSG(
		base_res_index, cursor.images.size(), Utilities::format("Invalid image index for cursor \"%s\": %d", name, base_res_index)
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

static String _to_define_file_name(String const& path) {
	static const String backslash = "\\";
	static const String forwardslash = "/";
	static const String cursor_directory_forwardslash = convert_to<String>(cursor_directory) + forwardslash;
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

	ERR_FAIL_COND_V_MSG(cursor_files.empty() && animated_cursor_files.empty(), FAILED, "No files in cursors directory");

	Error ret = OK;

	for(fs::path const& file_name : cursor_files) {
		String file = convert_to<String>(file_name.string());
		StringName name = _to_define_file_name(file);

		if (!_load_cursor_cur(name,file)){
			spdlog::error_s("Failed to load normal cursor at path {}", file_name);
			ret = FAILED;
		}
	}

	for(fs::path const& file_name : animated_cursor_files) {
		String file = convert_to<String>(file_name.string());
		StringName name = _to_define_file_name(file);

		if (!_load_cursor_ani(name,file)){
			spdlog::error_s("Failed to load animated cursor at path {}", file_name);
			ret = FAILED;
		}
	}

	return ret;
}

static constexpr int32_t _reverser_lookup[] {
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
	0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
};
static constexpr int32_t _reverse_bits(int32_t byte, int32_t bits_per_pixel=8) {
	int32_t a = _reverser_lookup[(byte & 0b1111)] << 4;
	int32_t b = _reverser_lookup[byte >> 4];
	int32_t c = b | a;
	return c >> (8-bits_per_pixel);
}

static constexpr int32_t _rotate_right(int32_t byte, int32_t size=8) {
	return ((byte & 0b1) << (size-1)) | (byte >> 1);
}

static int32_t _load_int_256(Ref<FileAccess> const& file) {
	int32_t value = file->get_8();
	if (value == 0) {
		value = 256;
	}
	return value;
}

static constexpr int32_t _get_row_start(int32_t x_coord, int32_t y_coord, int32_t bits_per_pixel) {
	x_coord *= bits_per_pixel;
	int32_t row_count = (x_coord + 31) >> 5;
	return row_count * y_coord * 4; // 4 bytes per row * rows down
}

static constexpr int32_t _select_bits(uint8_t const* data, int32_t row_start, int32_t first_bit, int32_t bit_count) {
	int32_t byte_index = first_bit >> 3;
	int32_t bit_in_byte_index = first_bit & 0b111;
	ERR_FAIL_COND_V_MSG(bit_in_byte_index + bit_count > 8, 0, "Attempted to select bits outside of a byte.");
	int32_t byte = _reverse_bits(*(data+row_start+byte_index));
	int32_t selected = (byte >> bit_in_byte_index) & ((1 << bit_count) - 1);

	//TODO: questionable hack, nothing in the spec suggests we should need to do this
	if (bit_count > 1 && selected != 0){
		return _rotate_right(selected,4);
	}
	return selected;
}

static constexpr bool _read_AND_mask(
	uint8_t const* data, int32_t pixel_x, int32_t pixel_y, int32_t x_dimension, int32_t offset) {
	
	int32_t row_start = _get_row_start(x_dimension, pixel_y, 1);
	int32_t and_bit = _select_bits(data, row_start + offset,pixel_x, 1);
	return !and_bit;
}

static void _pixel_palette_lookup(
		PackedByteArray const& data, PackedByteArray& pixel_data, uint32_t i,
		PackedByteArray const& palette, int32_t coord_x, int32_t coord_y, int32_t x_dimension,
		int32_t offset, bool transparent, int32_t bits_per_pixel) {
	
	int32_t row_start = _get_row_start(x_dimension, coord_y, bits_per_pixel);
	int32_t pixel_bits = _select_bits(data.ptr(), row_start + offset, coord_x*bits_per_pixel, bits_per_pixel);

	ERR_FAIL_COND_MSG(
		(pixel_bits + 1) * 4 > palette.size(), vformat("Attempted to select invalid colour palette entry %s", pixel_bits)
	);

	//pixel bits serves as an index into the colour palette. We need to multiply the index by the number of bytes per colour (4)
	pixel_data[(i*4) + 0] = palette[pixel_bits*4 + 0];
	pixel_data[(i*4) + 1] = palette[pixel_bits*4 + 1];
	pixel_data[(i*4) + 2] = palette[pixel_bits*4 + 2];
	pixel_data[(i*4) + 3] = 0xFF * transparent; //a
}

/*
24bit pixel support here is questionable:
the spec (per daubnet) says we should pad bytes to end things on 32bit boundaries
but the singular example of a 24bit cursor found on the internet does things like this.
So emit a warning when trying to load one of these
*/
static void _read_24bit_pixel(
	PackedByteArray const& image_data, PackedByteArray& pixel_data,
	int32_t i, int32_t offset, bool opaque) {
	ERR_FAIL_COND_MSG(
		(i + 1) * 3 > image_data.size(),
		vformat("Pixel %d tried to read from a pixel data array of max size %d", i, pixel_data.size())
	);

	pixel_data[(i*4) + 0] = image_data[offset + (i*3) + 2]; //r
	pixel_data[(i*4) + 1] = image_data[offset + (i*3) + 1]; //g
	pixel_data[(i*4) + 2] = image_data[offset + (i*3) + 0]; //b
	pixel_data[(i*4) + 3] = 0xFF * opaque; //a
}

static void _read_32bit_pixel(
	PackedByteArray const& image_data, PackedByteArray& pixel_data,
	int32_t i, int32_t offset, bool opaque) {
	ERR_FAIL_COND_MSG(
		(i + 1) * 4 > image_data.size(),
		vformat("Pixel %d tried to read from a pixel data array of max size %d", i, pixel_data.size())
	);

	pixel_data[(i*4) + 0] = image_data[offset + (i*4) + 2]; //r
	pixel_data[(i*4) + 1] = image_data[offset + (i*4) + 1]; //g
	pixel_data[(i*4) + 2] = image_data[offset + (i*4) + 0]; //b
	pixel_data[(i*4) + 3] = image_data[offset + (i*4) + 3] * opaque; //a
}

//used to load a .cur file from a file (could be the a whole .cur file, or a .cur within a .ani file)
static CursorSingleton::image_hotspot_pair_asset_t _load_pair(Ref<FileAccess> const& file) {
	CursorSingleton::image_hotspot_pair_asset_t pairs = {};

	//.cur's within .anis won't start of the beginning of the file, so save where they start
	int32_t base_offset = file->get_position();

	//.cur header
	int32_t reserved = file->get_16();
	int32_t type = file->get_16(); //1=ico, 2=cur
	int32_t images_count = file->get_16();
	
	//all the images
	for(int32_t i=0; i<images_count; i++){
		Vector2i dimensions = Vector2i(_load_int_256(file),_load_int_256(file)); //TODO
		int32_t palette = file->get_8();
		file->get_8(); //int32_t img_reserved
		
		Vector2i hotspot = Vector2i();
		hotspot.x = file->get_16();
		hotspot.y = file->get_16();

		int32_t data_size = std::min(static_cast<uint64_t>(file->get_32()), file->get_length() - file->get_position());
		int32_t data_offset = file->get_32();

		//This image header information is sequential in the data, but the images aren't necessarily
		// so save the current position, get the image data and return so we're ready for the next image header
		int32_t end_of_image_header = file->get_position();

		file->seek(data_offset+base_offset);
		PackedByteArray const& image_data = file->get_buffer(data_size);
		file->seek(end_of_image_header);

		Ref<Image> image = Ref<Image>();
		image.instantiate();
	
		// PNGs are stored in their entirety, so use Godot's internal loader
		if (image_data.slice(1,4).get_string_from_ascii() == "PNG") {
			image->load_png_from_buffer(image_data);
		}
		else { //BMP based cursor, have to load this manually
			//int32_t dib_header_size = image_data.decode_u32(0);

			//this is the combined sized of the picture and the transparency bitmask
			// (ex. 32x32 dimension image becomes 32x64 here)
			//Vector2i combined_dimensions = Vector2i(image_data.decode_u32(4),image_data.decode_u32(8));
			//int32_t colour_planes = image_data.decode_u16(12);
			int32_t bits_per_pixel = image_data.decode_u16(14);
			if (bits_per_pixel <= 8 || bits_per_pixel == 24){
				spdlog::warn_s("Attempting to import {} bit cursor, this isn't guaranteed to work", bits_per_pixel);
			}
			else if (bits_per_pixel != 32){
				spdlog::error_s(
					"Unsupported bits per pixel while loading cursor image with {} bits per pixel, loading blank image",
					bits_per_pixel
				);
			}

			int32_t size = image_data.decode_u32(20);
			Vector2i resolution = Vector2i(image_data.decode_s32(24),image_data.decode_s32(28));
			int32_t palette_size = image_data.decode_u32(32);

			if (palette_size == 0 && bits_per_pixel <= 8){
				palette_size =  1 << bits_per_pixel;
			}
			//int32_t important_colours = image_data.decode_u32(36);

			//for BMPs with 8 bits per pixel or less, the pixel data is actually a lookup to this table here
			PackedByteArray const& palette = image_data.slice(40,40+(4*palette_size));

			// this is where the image data starts
			int32_t offset = 40 + palette_size*4;

			//where the transparency AND mask starts
			int32_t mask_offset = offset + _get_row_start(dimensions.x,dimensions.y,bits_per_pixel);

			PackedByteArray pixel_data = PackedByteArray();
			pixel_data.resize(dimensions.x*dimensions.y*4);
			pixel_data.fill(255);

			int32_t i=0;
			for(int32_t row=0; row < dimensions.y; row++) {
				for(int32_t col=0; col < dimensions.x; col++) {
					Vector2i coord = Vector2i(col,row);
					bool transparent = _read_AND_mask(
						image_data.ptr(),coord.x,coord.y,dimensions.x,mask_offset
					);
					if (bits_per_pixel <= 8){
						//mostly legacy files, these ones all use a lookup into the colour palette
						_pixel_palette_lookup(
							image_data, pixel_data, i, palette, coord.x, coord.y, dimensions.x, offset, transparent, bits_per_pixel
						);
					}/*
					else if (bits_per_pixel == 16) { //TODO
						//Unsupported, error
					}*/
					else if (bits_per_pixel == 24) {
						//Support Questionable, based on 1 example on the internet as opposed to the actual spec
						_read_24bit_pixel(
							image_data, pixel_data, i, offset, transparent
						);
					}
					else if (bits_per_pixel == 32) {
						//What vic actually uses
						_read_32bit_pixel(
							image_data, pixel_data, i, offset, transparent
						);
					}
					i++;
				}
			}
			
			image = image->create_from_data(dimensions.x,dimensions.y,false, Image::FORMAT_RGBA8,pixel_data);
			//bmp images are stored bottom to top
			image->flip_y();
		}
		Ref<ImageTexture> image_texture = Ref<ImageTexture>();
		image_texture.instantiate();

		image_texture = image_texture->create_from_image(image);
		
		if (image_texture.is_null()){
			spdlog::error_s("Image Texture {} was null!", file->get_path());
		}

		pairs.hotspots.push_back(hotspot);
		pairs.images.push_back(image_texture);
		
	}
	return pairs;

}

bool CursorSingleton::_load_cursor_ani(StringName const& name, String const& path) {
	const Ref<FileAccess> file = FileAccess::open(path, FileAccess::ModeFlags::READ);

	const Error err = FileAccess::get_open_error();
	ERR_FAIL_COND_V_MSG(
		err != OK || file.is_null(), false, Utilities::format("Failed to open ani file: \"%s\"", path)
	);

	//read the RIFF container
	Utilities::read_riff_str(file); //riff_id
	const uint64_t riff_size = std::min(static_cast<uint64_t>(file->get_32()), file->get_length());;
	Utilities::read_riff_str(file); //form_type

	//important variables
	std::vector<TypedArray<ImageTexture>> frames_by_resolution;
	std::vector<PackedVector2Array> hotspots_by_resolution;

	PackedVector2Array resolutions;
	PackedFloat32Array display_rates;
	PackedInt32Array sequence;

	//ani header variables
	int32_t num_frames = 1;
	int32_t num_steps = 1;
	Vector2i dimensions = Vector2i(1,1);
	int32_t bit_count = 1;
	int32_t num_planes = 1; //???
	int32_t display_rate = 1; //how long each frame should last
	int32_t flags = 0;
	bool icon_flag = false;
	bool sequence_flag = false;


	while(file->get_position() < riff_size){
		String id = Utilities::read_riff_str(file);
		int32_t size = file->get_32();
		if (id == "LIST"){
			String list_type = Utilities::read_riff_str(file);
		}
		else if (id == "anih"){
			//hack for some files, there's likely a better way
			if (size == 36){ 
				file->get_32(); // header_size
			}
			num_frames = file->get_32();
			num_steps = file->get_32();
			dimensions = Vector2i(file->get_32(),file->get_32());
			bit_count = file->get_32();
			num_planes = file->get_32();
			display_rate = file->get_32();
			flags = file->get_32();
			icon_flag = flags & 0x1;
			sequence_flag = flags & 0x2;
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
					spdlog::error_s(
						"Malformatted .ani cursor file {} had inconsistent number of images per cursor",
						name
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
			for(int32_t i=0; i<num_steps; i++){
				sequence.push_back(file->get_32());
			}
		}
		else if (id == "rate"){
			for(int32_t i=0;i<num_steps;i++){
				display_rates.push_back(file->get_32()/60.0);
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
		for(int32_t i=0; i<num_steps;i++){
			sequence.push_back(i);
		}
	}
	if (display_rates.is_empty()){
		for(int32_t i=0; i<num_steps;i++){
			display_rates.push_back(display_rate/60.0);
		}
	}

	cursors.insert(
		name,
		cursor_asset_t {
			std::move(hotspots_by_resolution),
			std::move(frames_by_resolution),
			resolutions,
			static_cast<int32_t>(sequence.size()),
			display_rates,
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
		err != OK || file.is_null(), false, Utilities::format("Failed to open cur file: \"%s\"", path)
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

	cursors.insert(
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