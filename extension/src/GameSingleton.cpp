#include "GameSingleton.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic2/Logger.hpp"

using namespace godot;
using namespace OpenVic2;

#define ERR(x) ((x) == SUCCESS ? OK : FAILED)

GameSingleton* GameSingleton::singleton = nullptr;

void GameSingleton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_province_identifier_file", "file_path"), &GameSingleton::load_province_identifier_file);
	ClassDB::bind_method(D_METHOD("load_water_province_file", "file_path"), &GameSingleton::load_water_province_file);
	ClassDB::bind_method(D_METHOD("load_region_file", "file_path"), &GameSingleton::load_region_file);
	ClassDB::bind_method(D_METHOD("load_terrain_file", "file_path"), &GameSingleton::load_terrain_file);
	ClassDB::bind_method(D_METHOD("load_map_images", "province_image_path", "terrain_image_path"), &GameSingleton::load_map_images);
	ClassDB::bind_method(D_METHOD("setup"), &GameSingleton::setup);

	ClassDB::bind_method(D_METHOD("get_province_index_from_uv_coords", "coords"), &GameSingleton::get_province_index_from_uv_coords);
	ClassDB::bind_method(D_METHOD("get_province_info_from_index", "index"), &GameSingleton::get_province_info_from_index);
	ClassDB::bind_method(D_METHOD("get_width"), &GameSingleton::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &GameSingleton::get_height);
	ClassDB::bind_method(D_METHOD("get_aspect_ratio"), &GameSingleton::get_aspect_ratio);
	ClassDB::bind_method(D_METHOD("get_terrain_texture"), &GameSingleton::get_terrain_texture);
	ClassDB::bind_method(D_METHOD("get_province_shape_image_subdivisions"), &GameSingleton::get_province_shape_image_subdivisions);
	ClassDB::bind_method(D_METHOD("get_province_shape_texture"), &GameSingleton::get_province_shape_texture);
	ClassDB::bind_method(D_METHOD("get_province_colour_texture"), &GameSingleton::get_province_colour_texture);

	ClassDB::bind_method(D_METHOD("update_colour_image"), &GameSingleton::update_colour_image);
	ClassDB::bind_method(D_METHOD("get_mapmode_count"), &GameSingleton::get_mapmode_count);
	ClassDB::bind_method(D_METHOD("get_mapmode_identifier", "index"), &GameSingleton::get_mapmode_identifier);
	ClassDB::bind_method(D_METHOD("set_mapmode", "identifier"), &GameSingleton::set_mapmode);

	ClassDB::bind_method(D_METHOD("expand_building", "province_index", "building_type_identifier"), &GameSingleton::expand_building);

	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &GameSingleton::set_paused);
	ClassDB::bind_method(D_METHOD("toggle_paused"), &GameSingleton::toggle_paused);
	ClassDB::bind_method(D_METHOD("is_paused"), &GameSingleton::is_paused);
	ClassDB::bind_method(D_METHOD("increase_speed"), &GameSingleton::increase_speed);
	ClassDB::bind_method(D_METHOD("decrease_speed"), &GameSingleton::decrease_speed);
	ClassDB::bind_method(D_METHOD("can_increase_speed"), &GameSingleton::can_increase_speed);
	ClassDB::bind_method(D_METHOD("can_decrease_speed"), &GameSingleton::can_decrease_speed);
	ClassDB::bind_method(D_METHOD("get_longform_date"), &GameSingleton::get_longform_date);
	ClassDB::bind_method(D_METHOD("try_tick"), &GameSingleton::try_tick);

	ADD_SIGNAL(MethodInfo("state_updated"));
}

GameSingleton* GameSingleton::get_singleton() {
	return singleton;
}

/* REQUIREMENTS:
 * MAP-21, MAP-25
 */
GameSingleton::GameSingleton() : game_manager { [this]() { emit_signal("state_updated"); } },
								 terrain_variants { "terrain variants" } {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	Logger::set_info_func([](std::string&& str) { UtilityFunctions::print(str.c_str()); });
	Logger::set_error_func([](std::string&& str) { UtilityFunctions::push_error(str.c_str()); });

	static constexpr colour_t HIGH_ALPHA_VALUE = to_alpha_value(0.5f);
	static constexpr colour_t LOW_ALPHA_VALUE = to_alpha_value(0.2f);
	using mapmode_t = std::pair<std::string, Mapmode::colour_func_t>;
	const std::vector<mapmode_t> mapmodes = {
		{ "mapmode_terrain",
			[](Map const&, Province const& province) -> colour_t {
				return LOW_ALPHA_VALUE | (province.is_water() ? 0x4287F5 : 0x0D7017);
			} },
		{ "mapmode_province",
			[](Map const&, Province const& province) -> colour_t {
				return HIGH_ALPHA_VALUE | province.get_colour();
			} },
		{ "mapmode_region",
			[](Map const&, Province const& province) -> colour_t {
				Region const* region = province.get_region();
				if (region != nullptr) return (0xCC << 24) | region->get_colour();
				return NULL_COLOUR;
			} },
		{ "mapmode_index",
			[](Map const& map, Province const& province) -> colour_t {
				const uint8_t f = static_cast<float>(province.get_index()) / static_cast<float>(map.get_province_count()) * 255.0f;
				return HIGH_ALPHA_VALUE | (f << 16) | (f << 8) | f;
			} }
	};
	for (mapmode_t const& mapmode : mapmodes)
		game_manager.map.add_mapmode(mapmode.first, mapmode.second);
	game_manager.map.lock_mapmodes();

	using building_type_t = std::tuple<std::string, Building::level_t, Timespan>;
	const std::vector<building_type_t> building_types = {
		{ "building_fort", 4, 8 }, { "building_naval_base", 6, 15 }, { "building_railroad", 5, 10 }
	};
	for (building_type_t const& type : building_types)
		game_manager.building_manager.add_building_type(std::get<0>(type), std::get<1>(type), std::get<2>(type));
	game_manager.building_manager.lock_building_types();
}

GameSingleton::~GameSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

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

using parse_json_entry_func_t = std::function<godot::Error(godot::String const&, godot::Variant const&)>;

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
	if (colour == NULL_COLOUR) {
		UtilityFunctions::push_error("Invalid colour for province identifier \"", identifier, "\": ", entry);
		return FAILED;
	}
	return ERR(game_manager.map.add_province(identifier.utf8().get_data(), colour));
}

Error GameSingleton::load_province_identifier_file(String const& file_path) {
	const Error err = _parse_json_dictionary_file("province identifier", file_path, "prov_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return _parse_province_identifier_entry(identifier, entry);
		});
	game_manager.map.lock_provinces();
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
				province_identifiers.push_back(province_string.utf8().get_data());
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
	return ERR(game_manager.map.add_region(identifier.utf8().get_data(), province_identifiers));
}

Error GameSingleton::load_region_file(String const& file_path) {
	const Error err = _parse_json_dictionary_file("region", file_path, "region_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return _parse_region_entry(identifier, entry);
		});
	game_manager.map.lock_regions();
	return err;
}

TerrainVariant::TerrainVariant(std::string const& new_identfier, colour_t new_colour, Ref<Image> const& new_image)
	: HasIdentifier(new_identfier),
	  HasColour(new_colour),
	  image(new_image) {}

Ref<Image> TerrainVariant::get_image() const { return image; }

Error GameSingleton::_parse_terrain_entry(String const& identifier, Variant const& entry) {
	const colour_t colour = _parse_colour(entry);
	if (colour == NULL_COLOUR) {
		UtilityFunctions::push_error("Invalid colour for terrain texture \"", identifier, "\": ", entry);
		return FAILED;
	}
	static const String terrain_folder = "res://art/terrain/";
	const String terrain_path = terrain_folder + identifier;
	Ref<Image> terrain_image;
	terrain_image.instantiate();
	const Error err = terrain_image->load(terrain_path);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to load terrain image: ", terrain_path);
		return err;
	}
	return ERR(terrain_variants.add_item({ identifier.utf8().get_data(), colour, terrain_image }));
}

Error GameSingleton::load_terrain_file(String const& file_path) {
	Error parse_err = _parse_json_dictionary_file("terrain variants", file_path, "",
		[this](String const& identifier, Variant const& entry) -> Error {
			return _parse_terrain_entry(identifier, entry);
		});
	terrain_variants.lock();
	if (terrain_variants.get_item_count() == 0) parse_err = FAILED;

	Array terrain_images;
	for (TerrainVariant const& var : terrain_variants.get_items()) {
		terrain_variant_map[var.get_colour()] = terrain_images.size();
		terrain_images.append(var.get_image());
	}

	terrain_texture.instantiate();
	const Error texturearray_err = terrain_texture->create_from_images(terrain_images);
	if (texturearray_err != OK) {
		UtilityFunctions::push_error("Failed to create terrain texture array!");
		return texturearray_err;
	}
	return parse_err;
}

Error GameSingleton::load_map_images(String const& province_image_path, String const& terrain_image_path) {
	if (province_shape_texture.is_valid()) {
		UtilityFunctions::push_error("Map images have already been loaded, cannot load: ", province_image_path, " and ", terrain_image_path);
		return FAILED;
	}

	// Load images
	Ref<Image> province_image, terrain_image;
	province_image.instantiate();
	terrain_image.instantiate();
	Error err = province_image->load(province_image_path);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to load province image: ", province_image_path);
		return err;
	}
	err = terrain_image->load(terrain_image_path);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to load terrain image: ", terrain_image_path);
		return err;
	}

	// Validate dimensions and format
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
	const Image::Format province_format = province_image->get_format(), terrain_format = terrain_image->get_format();
	if (province_format != expected_format) {
		UtilityFunctions::push_error("Invalid format (", province_format, ", should be ", expected_format, ") for province image: ", province_image_path);
		err = FAILED;
	}
	if (terrain_format != expected_format) {
		UtilityFunctions::push_error("Invalid format (", terrain_format, ", should be ", expected_format, ") for terrain image: ", terrain_image_path);
		err = FAILED;
	}
	if (err != OK) return err;

	// Generate interleaved province and terrain ID image
	if (game_manager.map.generate_province_shape_image(province_dims.x, province_dims.y, province_image->get_data().ptr(),
			terrain_image->get_data().ptr(), terrain_variant_map) != SUCCESS) return FAILED;

	static constexpr int32_t GPU_DIM_LIMIT = 0x3FFF;
	// For each dimension of the image, this finds the small number of equal subdivisions required get the individual texture dims under GPU_DIM_LIMIT
	for (int i = 0; i < 2; ++i)
		for (image_subdivisions[i] = 1;
			 province_dims[i] / image_subdivisions[i] > GPU_DIM_LIMIT || province_dims[i] % image_subdivisions[i] != 0; ++image_subdivisions[i])
			;

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
		UtilityFunctions::push_error("");
		err = FAILED;
	}

	if (update_colour_image() != OK) err = FAILED;

	return err;
}

godot::Error GameSingleton::setup() {
	return ERR(game_manager.setup());
}

Error GameSingleton::load_water_province_file(String const& file_path) {
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
			if (game_manager.map.set_water_province(identifier.utf8().get_data()) != SUCCESS)
				err = FAILED;
		}
	}
	game_manager.map.lock_water_provinces();
	return err;
}

int32_t GameSingleton::get_province_index_from_uv_coords(Vector2 const& coords) const {
	const size_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * get_width();
	const size_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * get_height();
	return game_manager.map.get_province_index_at(x_mod_w, y_mod_h);
}

#define KEY(x) static const StringName x##_key = #x
Dictionary GameSingleton::get_province_info_from_index(int32_t index) const {
	Province const* province = game_manager.map.get_province_by_index(index);
	if (province == nullptr) return {};
	KEY(province);
	KEY(region);
	KEY(life_rating);
	KEY(buildings);
	Dictionary ret;

	ret[province_key] = province->get_identifier().c_str();

	Region const* region = province->get_region();
	if (region != nullptr) ret[region_key] = region->get_identifier().c_str();

	ret[life_rating_key] = province->get_life_rating();

	std::vector<Building> const& buildings = province->get_buildings();
	if (!buildings.empty()) {
		Array buildings_array;
		buildings_array.resize(buildings.size());
		for (size_t idx = 0; idx < buildings.size(); ++idx) {
			KEY(building);
			KEY(level);
			KEY(expansion_state);
			KEY(start_date);
			KEY(end_date);
			KEY(expansion_progress);

			Dictionary building_dict;
			Building const& building = buildings[idx];
			building_dict[building_key] = building.get_identifier().c_str();
			building_dict[level_key] = static_cast<int32_t>(building.get_level());
			building_dict[expansion_state_key] = static_cast<int32_t>(building.get_expansion_state());
			building_dict[start_date_key] = static_cast<std::string>(building.get_start_date()).c_str();
			building_dict[end_date_key] = static_cast<std::string>(building.get_end_date()).c_str();
			building_dict[expansion_progress_key] = building.get_expansion_progress();

			buildings_array[idx] = building_dict;
		}
		ret[buildings_key] = buildings_array;
	}
	return ret;
}
#undef KEY

int32_t GameSingleton::get_width() const {
	return game_manager.map.get_width();
}

int32_t GameSingleton::get_height() const {
	return game_manager.map.get_height();
}

float GameSingleton::get_aspect_ratio() const {
	return static_cast<float>(get_width()) / static_cast<float>(get_height());
}

Ref<Texture> GameSingleton::get_terrain_texture() const {
	return terrain_texture;
}

Vector2i GameSingleton::get_province_shape_image_subdivisions() const {
	return image_subdivisions;
}

Ref<Texture> GameSingleton::get_province_shape_texture() const {
	return province_shape_texture;
}

Ref<Texture> GameSingleton::get_province_colour_texture() const {
	return province_colour_texture;
}

Error GameSingleton::update_colour_image() {
	static PackedByteArray colour_data_array;
	static constexpr int64_t colour_data_array_size = (MAX_INDEX + 1) * Map::MAPMODE_COLOUR_SIZE;
	colour_data_array.resize(colour_data_array_size);

	Error err = OK;
	if (game_manager.map.generate_mapmode_colours(mapmode_index, colour_data_array.ptrw()) != SUCCESS)
		err = FAILED;

	static constexpr int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(index_t) * 4);
	if (province_colour_image.is_null())
		province_colour_image.instantiate();
	province_colour_image->set_data(PROVINCE_INDEX_SQRT, PROVINCE_INDEX_SQRT,
		false, Image::FORMAT_RGBA8, colour_data_array);
	if (province_colour_image.is_null()) {
		UtilityFunctions::push_error("Failed to update province colour image");
		return FAILED;
	}
	if (province_colour_texture.is_null())
		province_colour_texture = ImageTexture::create_from_image(province_colour_image);
	else
		province_colour_texture->update(province_colour_image);
	return err;
}

int32_t GameSingleton::get_mapmode_count() const {
	return game_manager.map.get_mapmode_count();
}

String GameSingleton::get_mapmode_identifier(int32_t index) const {
	Mapmode const* mapmode = game_manager.map.get_mapmode_by_index(index);
	if (mapmode != nullptr) return mapmode->get_identifier().c_str();
	return String {};
}

Error GameSingleton::set_mapmode(godot::String const& identifier) {
	Mapmode const* mapmode = game_manager.map.get_mapmode_by_identifier(identifier.utf8().get_data());
	if (mapmode == nullptr) {
		UtilityFunctions::push_error("Failed to set mapmode to: ", identifier);
		return FAILED;
	}
	mapmode_index = mapmode->get_index();
	return OK;
}

Error GameSingleton::expand_building(int32_t province_index, String const& building_type_identifier) {
	if (game_manager.expand_building(province_index, building_type_identifier.utf8().get_data()) != SUCCESS) {
		UtilityFunctions::push_error("Failed to expand ", building_type_identifier, " at province index ", province_index);
		return FAILED;
	}
	return OK;
}

void GameSingleton::set_paused(bool paused) {
	game_manager.clock.isPaused = paused;
}

void GameSingleton::toggle_paused() {
	game_manager.clock.isPaused = !game_manager.clock.isPaused;
}

bool GameSingleton::is_paused() const {
	return game_manager.clock.isPaused;
}

void GameSingleton::increase_speed() {
	game_manager.clock.increaseSimulationSpeed();
}

void GameSingleton::decrease_speed() {
	game_manager.clock.decreaseSimulationSpeed();
}

bool GameSingleton::can_increase_speed() const {
	return game_manager.clock.canIncreaseSimulationSpeed();
}

bool GameSingleton::can_decrease_speed() const {
	return game_manager.clock.canDecreaseSimulationSpeed();
}

String GameSingleton::get_longform_date() const {
	return static_cast<std::string>(game_manager.get_today()).c_str();
}

void GameSingleton::try_tick() {
	game_manager.clock.conditionallyAdvanceGame();
}
