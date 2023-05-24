#include "GameSingleton.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "Utilities.hpp"

using namespace godot;
using namespace OpenVic;

static Error _load_json_file(String const& file_description, String const& file_path, Variant& result) {
	result.clear();
	UtilityFunctions::print("Loading ", file_description, " file: ", file_path);
	const Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load ", file_description, " file: ", file_path);
		return err == OK ? FAILED : err;
	}
	const String json_string = file->get_as_text();
	JSON json;
	err = json.parse(json_string);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to parse ", file_description, " file as JSON: ", file_path,
			"\nError at line ", json.get_error_line(), ": ", json.get_error_message());
		return err;
	}
	result = json.get_data();
	return err;
}

using parse_json_entry_func_t = std::function<Error(String const&, Variant const&)>;

static Error _parse_json_dictionary_file(String const& file_description, String const& file_path,
	String const& identifier_prefix, parse_json_entry_func_t parse_entry) {
	Variant json_var;
	Error err = _load_json_file(file_description, file_path, json_var);
	if (err != OK) return err;
	const Variant::Type type = json_var.get_type();
	if (type != Variant::DICTIONARY) {
		UtilityFunctions::push_error("Invalid ", file_description, " JSON: root has type ",
			Variant::get_type_name(type), " (expected Dictionary)");
		return FAILED;
	}
	Dictionary const& dict = json_var;
	const Array identifiers = dict.keys();
	for (int64_t idx = 0; idx < identifiers.size(); ++idx) {
		String const& identifier = identifiers[idx];
		Variant const& entry = dict[identifier];
		if (identifier.is_empty()) {
			UtilityFunctions::push_error("Empty identifier in ", file_description, " file with entry: ", entry);
			err = FAILED;
			continue;
		}
		if (!identifier.begins_with(identifier_prefix))
			UtilityFunctions::push_warning("Identifier in ", file_description, " file missing \"", identifier_prefix, "\" prefix: ", identifier);
		if (parse_entry(identifier, entry) != OK) err = FAILED;
	}
	return err;
}

static colour_t _parse_colour(Variant const& var) {
	const Variant::Type type = var.get_type();
	if (type == Variant::ARRAY) {
		Array const& colour_array = var;
		if (colour_array.size() == 3) {
			colour_t colour = NULL_COLOUR;
			for (int jdx = 0; jdx < 3; ++jdx) {
				Variant const& var = colour_array[jdx];
				if (var.get_type() != Variant::FLOAT) return NULL_COLOUR;
				const double colour_double = var;
				if (std::trunc(colour_double) != colour_double) return NULL_COLOUR;
				const int64_t colour_int = static_cast<int64_t>(colour_double);
				if (colour_int < 0 || colour_int > 255) return NULL_COLOUR;
				colour = (colour << 8) | colour_int;
			}
			return colour;
		}
	} else if (type == Variant::STRING) {
		String const& colour_string = var;
		if (colour_string.is_valid_hex_number()) {
			const int64_t colour_int = colour_string.hex_to_int();
			if (colour_int != NULL_COLOUR && colour_int <= MAX_COLOUR_RGB)
				return colour_int;
		}
	}
	return NULL_COLOUR;
}

Error GameSingleton::_parse_province_identifier_entry(String const& identifier, Variant const& entry) {
	const colour_t colour = _parse_colour(entry);
	if (colour == NULL_COLOUR || colour > MAX_COLOUR_RGB) {
		UtilityFunctions::push_error("Invalid colour for province identifier \"", identifier, "\": ", entry);
		return FAILED;
	}
	return ERR(game_manager.map.add_province(godot_to_std_string(identifier), colour));
}

Error GameSingleton::_load_province_identifier_file(String const& file_path) {
	const Error err = _parse_json_dictionary_file("province identifier", file_path, "prov_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return _parse_province_identifier_entry(identifier, entry);
		});
	game_manager.map.lock_provinces();
	return err;
}

Error GameSingleton::_load_water_province_file(String const& file_path) {
	Variant json_var;
	Error err = _load_json_file("water province", file_path, json_var);
	if (err != OK) return err;
	Variant::Type type = json_var.get_type();
	if (type != Variant::ARRAY) {
		UtilityFunctions::push_error("Invalid water province JSON: root has type ",
			Variant::get_type_name(type), " (expected Array)");
		err = FAILED;
	} else {
		Array const& array = json_var;
		for (int64_t idx = 0; idx < array.size(); ++idx) {
			Variant const& entry = array[idx];
			type = entry.get_type();
			if (type != Variant::STRING) {
				UtilityFunctions::push_error("Invalid water province identifier: ", entry);
				err = FAILED;
				continue;
			}
			String const& identifier = entry;
			if (game_manager.map.set_water_province(godot_to_std_string(identifier)) != SUCCESS)
				err = FAILED;
		}
	}
	game_manager.map.lock_water_provinces();
	return err;
}

Error GameSingleton::_parse_region_entry(String const& identifier, Variant const& entry) {
	Error err = OK;
	Variant::Type type = entry.get_type();
	std::vector<std::string> province_identifiers;
	if (type == Variant::ARRAY) {
		Array const& province_array = entry;
		for (int64_t idx = 0; idx < province_array.size(); ++idx) {
			Variant const& province_var = province_array[idx];
			type = province_var.get_type();
			if (type == Variant::STRING) {
				String const& province_string = province_var;
				province_identifiers.push_back(godot_to_std_string(province_string));
			} else {
				UtilityFunctions::push_error("Invalid province identifier for region \"", identifier, "\": ", entry);
				err = FAILED;
			}
		}
	}
	if (province_identifiers.empty()) {
		UtilityFunctions::push_error("Invalid province list for region \"", identifier, "\": ", entry);
		return FAILED;
	}
	return ERR(game_manager.map.add_region(godot_to_std_string(identifier), province_identifiers));
}

Error GameSingleton::_load_region_file(String const& file_path) {
	const Error err = _parse_json_dictionary_file("region", file_path, "region_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return _parse_region_entry(identifier, entry);
		});
	game_manager.map.lock_regions();
	return err;
}

Error GameSingleton::_parse_terrain_entry(String const& identifier, Variant const& entry, String const& terrain_texture_dir_path) {
	const colour_t colour = _parse_colour(entry);
	if (colour == NULL_COLOUR || colour > MAX_COLOUR_RGB) {
		UtilityFunctions::push_error("Invalid colour for terrain texture \"", identifier, "\": ", entry);
		return FAILED;
	}
	const String terrain_path = terrain_texture_dir_path + identifier;
	const Ref<Image> terrain_image = load_godot_image(terrain_path);
	if (terrain_image.is_null()) {
		UtilityFunctions::push_error("Failed to load terrain image: ", terrain_path);
		return FAILED;
	}
	return ERR(terrain_variants.add_item({ godot_to_std_string(identifier), colour, terrain_image }));
}

Error GameSingleton::_load_terrain_variants(String const& terrain_identifiers_path, String const& terrain_texture_dir_path) {
	Error err = _parse_json_dictionary_file("terrain variants", terrain_identifiers_path, "",
		[this, terrain_texture_dir_path](String const& identifier, Variant const& entry) -> Error {
			return _parse_terrain_entry(identifier, entry, terrain_texture_dir_path + String { "/" });
		});
	terrain_variants.lock();
	if (_generate_terrain_texture_array() != OK) return FAILED;
	return err;
}

Error GameSingleton::_generate_terrain_texture_array() {
	Error err = OK;
	if (terrain_variants.get_item_count() == 0) {
		UtilityFunctions::push_error("Failed to load terrain textures!");
		return FAILED;
	}
	// TerrainVariant count is limited by the data type representing it in the map image
	if (terrain_variants.get_item_count() > TerrainVariant::MAX_INDEX) {
		UtilityFunctions::push_error("Too many terrain textures - all after the first ", MAX_INDEX, " will be ignored");
		err = FAILED;
	}

	Array terrain_images;
	for (size_t i = 0; i < terrain_variants.get_item_count() && i < TerrainVariant::MAX_INDEX; ++i) {
		TerrainVariant const& var = *terrain_variants.get_item_by_index(i);
		terrain_variant_map[var.get_colour()] = i;
		terrain_images.append(var.get_image());
	}

	terrain_texture.instantiate();
	if (terrain_texture->create_from_images(terrain_images) != OK) {
		UtilityFunctions::push_error("Failed to create terrain texture array!");
		return FAILED;
	}
	return err;
}

Error GameSingleton::_load_map_images(String const& province_image_path, String const& terrain_image_path, bool flip_vertical) {
	if (province_shape_texture.is_valid()) {
		UtilityFunctions::push_error("Map images have already been loaded, cannot load: ", province_image_path, " and ", terrain_image_path);
		return FAILED;
	}

	// Load images
	Ref<Image> province_image = load_godot_image(province_image_path);
	if (province_image.is_null()) {
		UtilityFunctions::push_error("Failed to load province image: ", province_image_path);
		return FAILED;
	}
	Ref<Image> terrain_image = load_godot_image(terrain_image_path);
	if (terrain_image.is_null()) {
		UtilityFunctions::push_error("Failed to load terrain image: ", terrain_image_path);
		return FAILED;
	}

	if (flip_vertical) {
		province_image->flip_y();
		terrain_image->flip_y();
	}

	// Validate dimensions and format
	Error err = OK;
	const Vector2i province_dims = province_image->get_size(), terrain_dims = terrain_image->get_size();
	if (province_dims.x < 1 || province_dims.y < 1) {
		UtilityFunctions::push_error("Invalid dimensions (", province_dims.x, "x", province_dims.y, ") for province image: ", province_image_path);
		err = FAILED;
	}
	if (province_dims != terrain_dims) {
		UtilityFunctions::push_error("Invalid dimensions (", terrain_dims.x, "x", terrain_dims.y, ") for terrain image: ",
			terrain_image_path, " (must match province image: (", province_dims.x, "x", province_dims.x, "))");
		err = FAILED;
	}
	static constexpr Image::Format expected_format = Image::FORMAT_RGB8;
	if (province_image->get_format() == Image::FORMAT_RGBA8) province_image->convert(expected_format);
	if (terrain_image->get_format() == Image::FORMAT_RGBA8) terrain_image->convert(expected_format);
	if (province_image->get_format() != expected_format) {
		UtilityFunctions::push_error("Invalid format (", province_image->get_format(), ", should be ", expected_format, ") for province image: ", province_image_path);
		err = FAILED;
	}
	if (terrain_image->get_format() != expected_format) {
		UtilityFunctions::push_error("Invalid format (", terrain_image->get_format(), ", should be ", expected_format, ") for terrain image: ", terrain_image_path);
		err = FAILED;
	}
	if (err != OK) return err;

	// Generate interleaved province and terrain ID image
	if (game_manager.map.generate_province_shape_image(province_dims.x, province_dims.y, province_image->get_data().ptr(),
		terrain_image->get_data().ptr(), terrain_variant_map) != SUCCESS) err = FAILED;

	static constexpr int32_t GPU_DIM_LIMIT = 0x3FFF;
	// For each dimension of the image, this finds the small number of equal subdivisions required get the individual texture dims under GPU_DIM_LIMIT
	for (int i = 0; i < 2; ++i)
		for (image_subdivisions[i] = 1; province_dims[i] / image_subdivisions[i] > GPU_DIM_LIMIT ||
			province_dims[i] % image_subdivisions[i] != 0; ++image_subdivisions[i]);

	Map::shape_pixel_t const* province_shape_data = game_manager.map.get_province_shape_image().data();
	const Vector2i divided_dims = province_dims / image_subdivisions;
	Array province_shape_images;
	province_shape_images.resize(image_subdivisions.x * image_subdivisions.y);
	for (int32_t v = 0; v < image_subdivisions.y; ++v) {
		for (int32_t u = 0; u < image_subdivisions.x; ++u) {
			PackedByteArray index_data_array;
			index_data_array.resize(divided_dims.x * divided_dims.y * sizeof(Map::shape_pixel_t));

			for (int32_t y = 0; y < divided_dims.y; ++y)
				memcpy(index_data_array.ptrw() + y * divided_dims.x * sizeof(Map::shape_pixel_t),
					province_shape_data + (v * divided_dims.y + y) * province_dims.x + u * divided_dims.x,
					divided_dims.x * sizeof(Map::shape_pixel_t));

			const Ref<Image> province_shape_subimage = Image::create_from_data(divided_dims.x, divided_dims.y, false, Image::FORMAT_RGB8, index_data_array);
			if (province_shape_subimage.is_null()) {
				UtilityFunctions::push_error("Failed to create province shape image (", u, ", ", v, ")");
				err = FAILED;
			}
			province_shape_images[u + v * image_subdivisions.x] = province_shape_subimage;
		}
	}

	province_shape_texture.instantiate();
	if (province_shape_texture->create_from_images(province_shape_images) != OK) {
		UtilityFunctions::push_error("Failed to create terrain texture array!");
		err = FAILED;
	}

	if (_update_colour_image() != OK) err = FAILED;

	return err;
}

Error GameSingleton::_parse_good_entry(String const& identifier, Variant const& entry) {
	if (entry.get_type() != Variant::DICTIONARY) {
		UtilityFunctions::push_error("Invalid good entry for ", identifier, ": ", entry);
		return FAILED;
	}
	Dictionary const& dict = entry;

	static const String key_category = "category";
	Variant const& var_category = dict.get(key_category, "");
	String category;
	if (var_category.get_type() == Variant::STRING) category = var_category;
	else UtilityFunctions::push_error("Invalid good category for ", identifier, ": ", var_category);

	static const String key_base_price = "base_price";
	Variant const& var_base_price = dict.get(key_base_price, NULL_PRICE);
	price_t base_price = NULL_PRICE;
	if (var_base_price.get_type() == Variant::FLOAT) base_price = var_base_price;
	else UtilityFunctions::push_error("Invalid good base price for ", identifier, ": ", var_base_price);

	static const String key_colour = "colour";
	Variant const& var_colour = dict.get(key_colour, "");
	const colour_t colour = _parse_colour(var_colour);
	if (colour > MAX_COLOUR_RGB) {
		UtilityFunctions::push_error("Invalid good colour for ", identifier, ": ", var_colour);
		return FAILED;
	}

	static const String key_default_available = "default_available";
	Variant const& var_default_available = dict.get(key_default_available, true);
	bool default_available = false;
	if (var_default_available.get_type() == Variant::BOOL) default_available = var_default_available;
	else UtilityFunctions::push_error("Invalid good available default bool value for ", identifier, ": ", var_default_available);

	static const String key_tradeable = "tradeable";
	Variant const& var_tradeable = dict.get(key_tradeable, true);
	bool tradeable = false;
	if (var_tradeable.get_type() == Variant::BOOL) tradeable = var_tradeable;
	else UtilityFunctions::push_error("Invalid good tradeable bool value for ", identifier, ": ", var_tradeable);

	static const String key_currency = "currency";
	Variant const& var_currency = dict.get(key_currency, true);
	bool currency = false;
	if (var_currency.get_type() == Variant::BOOL) currency = var_currency;
	else UtilityFunctions::push_error("Invalid good currency bool value for ", identifier, ": ", var_currency);

	static const String key_overseas_maintenance = "overseas_maintenance";
	Variant const& var_overseas_maintenance = dict.get(key_overseas_maintenance, true);
	bool overseas_maintenance = false;
	if (var_overseas_maintenance.get_type() == Variant::BOOL) overseas_maintenance = var_overseas_maintenance;
	else UtilityFunctions::push_error("Invalid good overseas maintenance bool value for ", identifier, ": ", var_overseas_maintenance);

	return ERR(game_manager.good_manager.add_good(godot_to_std_string(identifier), godot_to_std_string(category),
		colour, base_price, default_available, tradeable, currency, overseas_maintenance));
}

Error GameSingleton::_load_goods(String const& defines_path, String const& icons_dir_path) {
	Error err = _parse_json_dictionary_file("good", defines_path, "good_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return _parse_good_entry(identifier, entry);
		});
	game_manager.good_manager.lock_goods();
	for (Good const& good : game_manager.good_manager.get_goods()) {
		const String path = icons_dir_path + String { "/" } + std_to_godot_string(good.get_identifier()) + ".png";
		const Ref<Image> image = load_godot_image(path);
		if (image.is_null()) {
			UtilityFunctions::push_error("Failed to load good icon image: ", path);
			err = FAILED;
			continue;
		}
		const Ref<Texture> tex = ImageTexture::create_from_image(image);
		if (tex.is_null()) {
			UtilityFunctions::push_error("Failed to generate good icon texture: ", path);
			err = FAILED;
			continue;
		}
		good_icons[std_to_godot_string(good.get_identifier())] = tex;
	}
	return err;
}

StringName const& GameSingleton::get_province_identifier_file_key() {
	static const StringName key = "province_identifiers";
	return key;
}
StringName const& GameSingleton::get_water_province_file_key() {
	static const StringName key = "water_provinces";
	return key;
}
StringName const& GameSingleton::get_region_file_key() {
	static const StringName key = "regions";
	return key;
}
StringName const& GameSingleton::get_terrain_variant_file_key() {
	static const StringName key = "terrain_variants";
	return key;
}
StringName const& GameSingleton::get_terrain_texture_dir_key() {
	static const StringName key = "terrain_textures";
	return key;
}
StringName const& GameSingleton::get_province_image_file_key() {
	static const StringName key = "province_image";
	return key;
}
StringName const& GameSingleton::get_terrain_image_file_key() {
	static const StringName key = "terrain_image";
	return key;
}
StringName const& GameSingleton::get_goods_file_key() {
	static const StringName key = "goods";
	return key;
}
StringName const& GameSingleton::get_good_icons_dir_key() {
	static const StringName key = "good_icons";
	return key;
}

Error GameSingleton::load_defines(Dictionary const& file_dict) {
	Error err = OK;
	if (_load_province_identifier_file(file_dict.get(get_province_identifier_file_key(), "")) != OK) {
		UtilityFunctions::push_error("Failed to load province identifiers!");
		err = FAILED;
	}
	if (_load_water_province_file(file_dict.get(get_water_province_file_key(), "")) != OK) {
		UtilityFunctions::push_error("Failed to load water provinces!");
		err = FAILED;
	}
	if (_load_region_file(file_dict.get(get_region_file_key(), "")) != OK) {
		UtilityFunctions::push_error("Failed to load regions!");
		err = FAILED;
	}
	if (_load_terrain_variants(file_dict.get(get_terrain_variant_file_key(), ""),
		file_dict.get(get_terrain_texture_dir_key(), "")) != OK) {
		UtilityFunctions::push_error("Failed to load terrain variants!");
		err = FAILED;
	}
	if (_load_map_images(file_dict.get(get_province_image_file_key(), ""), file_dict.get(get_terrain_image_file_key(), "")) != OK) {
		UtilityFunctions::push_error("Failed to load map images!");
		err = FAILED;
	}
	if (_load_goods(file_dict.get(get_goods_file_key(), ""), file_dict.get(get_good_icons_dir_key(), "")) != OK) {
		UtilityFunctions::push_error("Failed to load goods!");
		err = FAILED;
	}
	if (_load_hardcoded_defines() != OK) {
		UtilityFunctions::push_error("Failed to hardcoded defines!");
		err = FAILED;
	}
	return err;
}
