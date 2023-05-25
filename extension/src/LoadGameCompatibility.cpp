#include "GameSingleton.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic/utility/BMP.hpp"

#include "Utilities.hpp"

using namespace godot;
using namespace OpenVic;

Error GameSingleton::_load_province_identifier_file_compatibility_mode(String const& file_path) {
	UtilityFunctions::print("Loading compatibility mode province identifier file: ", file_path);

	const Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load compatibility mode province identifier file: ", file_path);
		return err == OK ? FAILED : err;
	}

	int line_number = 0;
	while (!file->eof_reached()) {
		const PackedStringArray line = file->get_csv_line(";");
		line_number++;

		if (line.is_empty() || (line.size() == 1 && line[0].is_empty()))
			continue;

		if (line_number < 2) continue; // skip header line
		index_t id = NULL_INDEX;
		colour_t colour = NULL_COLOUR;
		if (line.size() > 0) {
			if (line[0].is_empty()) {
				id = game_manager.map.get_province_count() + 1;
			} else if (line[0].is_valid_int()) {
				const int64_t val = line[0].to_int();
				if (val > NULL_INDEX && val <= MAX_INDEX) id = val;
			}
			for (int i = 1; i < 4; ++i) {
				if (line.size() > i) {
					if (line[i].is_valid_int()) {
						const int64_t int_val = line[i].to_int();
						if (int_val >= NULL_COLOUR && int_val <= FULL_COLOUR) {
							colour = (colour << 8) | int_val;
							continue;
						}
					} else if (line[i].is_valid_float()) {
						const double double_val = line[i].to_float();
						if (std::trunc(double_val) == double_val) {
							const int64_t int_val = double_val;
							if (int_val >= NULL_COLOUR && int_val <= FULL_COLOUR) {
								colour = (colour << 8) | int_val;
								continue;
							}
						}
					}
				}
				colour = NULL_COLOUR;
				break;
			}
		}
		if (id == NULL_INDEX || colour == NULL_COLOUR) {
			UtilityFunctions::push_error("Invalid province ID-colour entry \"", line, "\" on line ", line_number, " in file: ", file_path);
			err = FAILED;
			continue;
		}
		static const std::string province_prefix = "PROV";
		if (game_manager.map.add_province(province_prefix + std::to_string(id), colour) != SUCCESS) err = FAILED;
	}
	game_manager.map.lock_provinces();
	return err;
}

Error GameSingleton::_load_terrain_variants_compatibility_mode(String const& terrain_image_path, String const& terrain_texturesheet_path) {
	// Read BMP's palette to determine terrain variant colours which texture they're associated with
	BMP bmp;
	if (bmp.open(godot_to_std_string(terrain_image_path).c_str()) != SUCCESS || bmp.read_header() != SUCCESS || bmp.read_palette() != SUCCESS) {
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
		Ref<Image> water_image = Image::create(slice_size, slice_size, false, terrain_sheet->get_format());
		ERR_FAIL_NULL_V_EDMSG(water_image, FAILED, "Failed to create water terrain image");
		water_image->fill({ 0.0f, 0.0f, 1.0f });
		terrain_variants.add_item({ "terrain_water", 0xFFFFFF, water_image });
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
		if (terrain_variants.add_item({ "terrain_" + std::to_string(idx), palette[idx], terrain_image }) != SUCCESS) err = FAILED;
	}
	terrain_variants.lock();
	if (_generate_terrain_texture_array() != OK) return FAILED;
	return err;
}

Error GameSingleton::load_defines_compatibility_mode(String const& file_path) {
	static const String province_identifier_file = "/map/definition.csv";
	static const String province_image_file = "/map/provinces.bmp";
	static const String terrain_image_file = "/map/terrain.bmp";
	static const String terrain_texture_dir = "/map/terrain/texturesheet.tga";

	Error err = OK;
	if (_load_province_identifier_file_compatibility_mode(file_path + province_identifier_file) != OK) {
		UtilityFunctions::push_error("Failed to load province identifiers!");
		err = FAILED;
	}
	game_manager.map.lock_water_provinces();
	game_manager.map.lock_regions();
	if (_load_terrain_variants_compatibility_mode(file_path + terrain_image_file, file_path + terrain_texture_dir) != OK) {
		UtilityFunctions::push_error("Failed to load terrain variants!");
		err = FAILED;
	}
	if (_load_map_images(file_path + province_image_file, file_path + terrain_image_file, true) != OK) {
		UtilityFunctions::push_error("Failed to load map images!");
		err = FAILED;
	}
	game_manager.good_manager.lock_goods();
	if (_load_hardcoded_defines() != OK) {
		UtilityFunctions::push_error("Failed to hardcoded defines!");
		err = FAILED;
	}
	return err;
}
