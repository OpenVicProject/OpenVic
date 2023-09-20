#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/utility/BMP.hpp>

#include "openvic-extension/Utilities.hpp"

#include "GameSingleton.hpp"

using namespace godot;
using namespace OpenVic;

Error GameSingleton::quazz_map_convert(String const& json_file_path, String const& csv_file_path) {
	Ref<FileAccess> json_file = FileAccess::open(json_file_path, FileAccess::READ);

	Variant data_received;
	if (!json_file.is_valid() && FileAccess::get_open_error() == FAILED) {
		UtilityFunctions::push_error("Error opening JSON file: ", json_file);
		return FAILED;
	}

	String json_content = json_file->get_as_text();
	json_file->close();
	godot::JSON json_data;
	Error error = json_data.parse(json_content);

	if (error == OK) {
		data_received = json_data.get_data(); 
	} else {
		UtilityFunctions::push_error("JSON Parse Error: ", json_data.get_error_message());
		return FAILED;
	}
	Ref<FileAccess> csv_file = FileAccess::open(csv_file_path, FileAccess::WRITE);

	if (!csv_file.is_valid() && FileAccess::get_open_error() == FAILED) {
		UtilityFunctions::push_error("Error creating CSV file: ", csv_file);
		return FAILED;
	}

	Dictionary json_dict = data_received;
	Array keys = json_dict.keys();
	String csv_header = "province_count;red;green;blue;province_name;x\n";
	csv_file->store_string(csv_header);

	for (int i = 0; i < keys.size(); i++) {
		String province_name = keys[i];
		Variant color_data = json_dict[province_name];
		String color_string;

		if (color_data.get_type() == Variant::ARRAY) {
			Array color_array = color_data;
			int red = color_array[0];
			int green = color_array[1];
			int blue = color_array[2];
			color_string = String::num(red) + ";" + String::num(green) + ";" + String::num(blue);
		} else if (color_data.get_type() == Variant::STRING) {
			String hex_color = color_data;
			int red = hex_color.substr(1, 2).hex_to_int();
			int green = hex_color.substr(3, 2).hex_to_int();
			int blue = hex_color.substr(5, 2).hex_to_int();
			color_string = String::num(red) + ";" + String::num(green) + ";" + String::num(blue);
		}

		String csv_line = String::num(i + 1) + ";" + color_string + ";" + province_name + ";x\n";
		csv_file->store_string(csv_line);
	}
	csv_file->close();

	Error err = OK;
	return err;
}

Error GameSingleton::_load_terrain_variants_compatibility_mode(String const& terrain_image_path, String const& terrain_texturesheet_path) {
	// Read BMP's palette to determine terrain variant colours which texture they're associated with
	BMP bmp;
	if (!(bmp.open(godot_to_std_string(terrain_image_path).c_str()) && bmp.read_header() && bmp.read_palette())) {
		UtilityFunctions::push_error("Failed to read BMP palette from compatibility mode terrain image: ", terrain_image_path);
		return FAILED;
	}
	std::vector<colour_t> const& palette = bmp.get_palette();
	static constexpr int32_t SHEET_DIMS = 8, PALETTE_SIZE = SHEET_DIMS * SHEET_DIMS;
	if (palette.size() == 0 || palette.size() < PALETTE_SIZE) {
		UtilityFunctions::push_error("Invalid BMP palette size for terrain image: ", static_cast<uint64_t>(palette.size()), " (expected ", PALETTE_SIZE, ")");
		return FAILED;
	}

	// Load the terrain texture sheet and prepare to slice it up
	Ref<Image> terrain_sheet = load_godot_image(terrain_texturesheet_path);
	if (terrain_sheet.is_null()) {
		UtilityFunctions::push_error("Failed to load terrain texture sheet: ", terrain_texturesheet_path);
		return FAILED;
	}
	terrain_sheet->flip_y();
	const int32_t sheet_width = terrain_sheet->get_width(), sheet_height = terrain_sheet->get_height();
	if (sheet_width < 1 || sheet_width % SHEET_DIMS != 0 || sheet_width != sheet_height) {
		UtilityFunctions::push_error("Invalid terrain texture sheet dims: ", sheet_width, "x", sheet_height, " (must be square with dims positive multiples of ", SHEET_DIMS, ")");
		return FAILED;
	}
	const int32_t slice_size = sheet_width / SHEET_DIMS;

	{
		static constexpr colour_t TERRAIN_WATER_INDEX_COLOUR = 0xFFFFFF;
		Ref<Image> water_image = Image::create(slice_size, slice_size, false, terrain_sheet->get_format());
		ERR_FAIL_NULL_V_EDMSG(water_image, FAILED, "Failed to create water terrain image");
		water_image->fill({ 0.1f, 0.1f, 0.5f });
		terrain_variants.add_item({ "terrain_water", TERRAIN_WATER_INDEX_COLOUR, water_image });
	}
	Error err = OK;
	for (int32_t idx = 0; idx < PALETTE_SIZE; ++idx) {
		const Rect2i slice { (idx % SHEET_DIMS) * slice_size, (7 - (idx / SHEET_DIMS)) * slice_size, slice_size, slice_size };
		const Ref<Image> terrain_image = terrain_sheet->get_region(slice);
		if (terrain_image.is_null() || terrain_image->is_empty()) {
			UtilityFunctions::push_error("Failed to extract terrain texture slice ", slice, " from ", terrain_texturesheet_path);
			err = FAILED;
			continue;
		}
		if (!terrain_variants.add_item({ "terrain_" + std::to_string(idx), palette[idx], terrain_image })) err = FAILED;
	}
	terrain_variants.lock();
	if (_generate_terrain_texture_array() != OK) return FAILED;
	return err;
}

Error GameSingleton::load_defines_compatibility_mode(PackedStringArray const& file_paths) {
	static const fs::path province_image_file = "map/provinces.bmp";
	static const fs::path terrain_image_file = "map/terrain.bmp";
	static const fs::path terrain_texture_file = "map/terrain/texturesheet.tga";

	Dataloader::path_vector_t roots;
	for (String const& path : file_paths) {
		roots.push_back(godot_to_std_string(path));
	}

	Error err = OK;

	if (!dataloader.set_roots(roots)) {
		Logger::error("Failed to set dataloader roots!");
		err = FAILED;
	}

	if (!dataloader.load_defines(game_manager)) {
		UtilityFunctions::push_error("Failed to load defines!");
		err = FAILED;
	}

	game_manager.map.lock_regions();
	if (_load_terrain_variants_compatibility_mode(
			std_to_godot_string(dataloader.lookup_file(terrain_image_file).string()),
			std_to_godot_string(dataloader.lookup_file(terrain_texture_file).string())
		) != OK) {
		UtilityFunctions::push_error("Failed to load terrain variants!");
		err = FAILED;
	}
	if (_load_map_images(
			std_to_godot_string(dataloader.lookup_file(province_image_file).string()),
			std_to_godot_string(dataloader.lookup_file(terrain_image_file).string()),
			true
		) != OK) {
		UtilityFunctions::push_error("Failed to load map images!");
		err = FAILED;
	}
	if (!game_manager.load_hardcoded_defines()) {
		UtilityFunctions::push_error("Failed to hardcoded defines!");
		err = FAILED;
	}
	return err;
}

String GameSingleton::lookup_file(String const& path) const {
	return std_to_godot_string(dataloader.lookup_file(godot_to_std_string(path)).string());
}
