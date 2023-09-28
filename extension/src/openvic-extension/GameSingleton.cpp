#include "GameSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/utility/Logger.hpp>

#include "openvic-extension/LoadLocalisation.hpp"
#include "openvic-extension/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

GameSingleton* GameSingleton::singleton = nullptr;

void GameSingleton::_bind_methods() {
	ClassDB::bind_static_method("GameSingleton", D_METHOD("setup_logger"), &GameSingleton::setup_logger);
	ClassDB::bind_method(D_METHOD("load_defines_compatibility_mode", "file_paths"), &GameSingleton::load_defines_compatibility_mode);
	ClassDB::bind_method(D_METHOD("lookup_file", "path"), &GameSingleton::lookup_file);
	ClassDB::bind_method(D_METHOD("setup_game"), &GameSingleton::setup_game);

	ClassDB::bind_method(D_METHOD("get_province_index_from_uv_coords", "coords"), &GameSingleton::get_province_index_from_uv_coords);
	ClassDB::bind_method(D_METHOD("get_province_info_from_index", "index"), &GameSingleton::get_province_info_from_index);
	ClassDB::bind_method(D_METHOD("get_width"), &GameSingleton::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &GameSingleton::get_height);
	ClassDB::bind_method(D_METHOD("get_aspect_ratio"), &GameSingleton::get_aspect_ratio);
	ClassDB::bind_method(D_METHOD("get_terrain_texture"), &GameSingleton::get_terrain_texture);
	ClassDB::bind_method(D_METHOD("get_province_shape_image_subdivisions"), &GameSingleton::get_province_shape_image_subdivisions);
	ClassDB::bind_method(D_METHOD("get_province_shape_texture"), &GameSingleton::get_province_shape_texture);
	ClassDB::bind_method(D_METHOD("get_province_colour_texture"), &GameSingleton::get_province_colour_texture);

	ClassDB::bind_method(D_METHOD("get_mapmode_count"), &GameSingleton::get_mapmode_count);
	ClassDB::bind_method(D_METHOD("get_mapmode_identifier", "index"), &GameSingleton::get_mapmode_identifier);
	ClassDB::bind_method(D_METHOD("set_mapmode", "identifier"), &GameSingleton::set_mapmode);
	ClassDB::bind_method(D_METHOD("get_selected_province_index"), &GameSingleton::get_selected_province_index);
	ClassDB::bind_method(D_METHOD("set_selected_province", "index"), &GameSingleton::set_selected_province);

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
	ADD_SIGNAL(MethodInfo("province_selected", PropertyInfo(Variant::INT, "index")));

	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_province_key"), &GameSingleton::get_province_info_province_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_region_key"), &GameSingleton::get_province_info_region_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_life_rating_key"), &GameSingleton::get_province_info_life_rating_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_terrain_type_key"), &GameSingleton::get_province_info_terrain_type_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_total_population_key"), &GameSingleton::get_province_info_total_population_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_pop_types_key"), &GameSingleton::get_province_info_pop_types_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_pop_ideologies_key"), &GameSingleton::get_province_info_pop_ideologies_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_pop_cultures_key"), &GameSingleton::get_province_info_pop_cultures_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_rgo_key"), &GameSingleton::get_province_info_rgo_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_buildings_key"), &GameSingleton::get_province_info_buildings_key);

	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_building_info_building_key"), &GameSingleton::get_building_info_building_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_building_info_level_key"), &GameSingleton::get_building_info_level_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_building_info_expansion_state_key"), &GameSingleton::get_building_info_expansion_state_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_building_info_start_date_key"), &GameSingleton::get_building_info_start_date_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_building_info_end_date_key"), &GameSingleton::get_building_info_end_date_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_building_info_expansion_progress_key"), &GameSingleton::get_building_info_expansion_progress_key);

	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_piechart_info_size_key"), &GameSingleton::get_piechart_info_size_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_piechart_info_colour_key"), &GameSingleton::get_piechart_info_colour_key);

	ClassDB::bind_static_method("GameSingleton", D_METHOD("draw_pie_chart", "image", "stopAngles", "colours", "radius",
		"shadow_displacement", "shadow_tightness", "shadow_radius", "shadow_thickness",
		"trim_colour", "trim_size", "gradient_falloff", "gradient_base",
		"donut", "donut_inner_trim", "donut_inner_radius"), &GameSingleton::draw_pie_chart);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("load_image", "path"), &GameSingleton::load_image);
}

void GameSingleton::draw_pie_chart(Ref<Image> image,
	Array const& stopAngles, Array const& colours, float radius,
	Vector2 shadow_displacement, float shadow_tightness, float shadow_radius, float shadow_thickness,
	Color trim_colour, float trim_size, float gradient_falloff, float gradient_base,
	bool donut, bool donut_inner_trim, float donut_inner_radius) {

	OpenVic::draw_pie_chart(image, stopAngles, colours, radius, shadow_displacement, shadow_tightness, shadow_radius, shadow_thickness,
		trim_colour, trim_size, gradient_falloff, gradient_base,
		donut, donut_inner_trim, donut_inner_radius);
}

Ref<Image> GameSingleton::load_image(String const& path) {
	return load_godot_image(path);
}

GameSingleton* GameSingleton::get_singleton() {
	return singleton;
}

void GameSingleton::_on_state_updated() {
	_update_colour_image();
	emit_signal("state_updated");
}

/* REQUIREMENTS:
 * MAP-21, MAP-23, MAP-25, MAP-32, MAP-33, MAP-34
 */
GameSingleton::GameSingleton() : game_manager { [this]() { _on_state_updated(); } } {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

void GameSingleton::setup_logger() {
	Logger::set_info_func([](std::string&& str) { UtilityFunctions::print(std_to_godot_string(str)); });
	Logger::set_warning_func([](std::string&& str) { UtilityFunctions::push_warning(std_to_godot_string(str)); });
	Logger::set_error_func([](std::string&& str) { UtilityFunctions::push_error(std_to_godot_string(str)); });
}

GameSingleton::~GameSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

Error GameSingleton::setup_game() {
	bool ret = game_manager.setup();
	ret &= dataloader.load_pop_history(game_manager, "history/pops/" + game_manager.get_today().to_string());
	return ERR(ret);
}

int32_t GameSingleton::get_province_index_from_uv_coords(Vector2 const& coords) const {
	const size_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * get_width();
	const size_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * get_height();
	return game_manager.get_map().get_province_index_at(x_mod_w, y_mod_h);
}

StringName const& GameSingleton::get_province_info_province_key() {
	static const StringName key = "province";
	return key;
}
StringName const& GameSingleton::get_province_info_region_key() {
	static const StringName key = "region";
	return key;
}
StringName const& GameSingleton::get_province_info_life_rating_key() {
	static const StringName key = "life_rating";
	return key;
}
StringName const& GameSingleton::get_province_info_terrain_type_key() {
	static const StringName key = "terrain_type";
	return key;
}
StringName const& GameSingleton::get_province_info_total_population_key() {
	static const StringName key = "total_population";
	return key;
}
StringName const& GameSingleton::get_province_info_pop_types_key() {
	static const StringName key = "pop_types";
	return key;
}
StringName const& GameSingleton::get_province_info_pop_ideologies_key() {
	static const StringName key = "pop_ideologies";
	return key;
}
StringName const& GameSingleton::get_province_info_pop_cultures_key() {
	static const StringName key = "pop_cultures";
	return key;
}
StringName const& GameSingleton::get_province_info_rgo_key() {
	static const StringName key = "rgo";
	return key;
}
StringName const& GameSingleton::get_province_info_buildings_key() {
	static const StringName key = "buildings";
	return key;
}

StringName const& GameSingleton::get_building_info_building_key() {
	static const StringName key = "building";
	return key;
}
StringName const& GameSingleton::get_building_info_level_key() {
	static const StringName key = "level";
	return key;
}
StringName const& GameSingleton::get_building_info_expansion_state_key() {
	static const StringName key = "expansion_state";
	return key;
}
StringName const& GameSingleton::get_building_info_start_date_key() {
	static const StringName key = "start_date";
	return key;
}
StringName const& GameSingleton::get_building_info_end_date_key() {
	static const StringName key = "end_date";
	return key;
}
StringName const& GameSingleton::get_building_info_expansion_progress_key() {
	static const StringName key = "expansion_progress";
	return key;
}

StringName const& GameSingleton::get_piechart_info_size_key() {
	static const StringName key = "size";
	return key;
}
StringName const& GameSingleton::get_piechart_info_colour_key() {
	static const StringName key = "colour";
	return key;
}

Dictionary GameSingleton::_distribution_to_dictionary(distribution_t const& dist) const {
	Dictionary dict;
	for (distribution_t::value_type const& p : dist) {
		Dictionary sub_dict;
		sub_dict[get_piechart_info_size_key()] = p.second;
		sub_dict[get_piechart_info_colour_key()] = to_godot_color(p.first->get_colour());
		dict[std_to_godot_string(p.first->get_identifier())] = sub_dict;
	}
	return dict;
}

Dictionary GameSingleton::get_province_info_from_index(int32_t index) const {
	Province const* province = game_manager.get_map().get_province_by_index(index);
	if (province == nullptr) return {};
	Dictionary ret;

	ret[get_province_info_province_key()] = std_to_godot_string(province->get_identifier());

	Region const* region = province->get_region();
	if (region != nullptr) ret[get_province_info_region_key()] = std_to_godot_string(region->get_identifier());

	Good const* rgo = province->get_rgo();
	if (rgo != nullptr) ret[get_province_info_rgo_key()] = std_to_godot_string(rgo->get_identifier());

	ret[get_province_info_life_rating_key()] = province->get_life_rating();

	TerrainType const* terrain_type = province->get_terrain_type();
	if (terrain_type != nullptr) ret[get_province_info_terrain_type_key()] = std_to_godot_string(terrain_type->get_identifier());

	ret[get_province_info_total_population_key()] = province->get_total_population();
	distribution_t const& pop_types = province->get_pop_type_distribution();
	if (!pop_types.empty()) ret[get_province_info_pop_types_key()] = _distribution_to_dictionary(pop_types);
	//distribution_t const& ideologies = province->get_ideology_distribution();
	//if (!ideologies.empty()) ret[get_province_info_pop_ideologies_key()] = _distribution_to_dictionary(ideologies);
	distribution_t const& cultures = province->get_culture_distribution();
	if (!cultures.empty()) ret[get_province_info_pop_cultures_key()] = _distribution_to_dictionary(cultures);

	std::vector<BuildingInstance> const& buildings = province->get_buildings();
	if (!buildings.empty()) {
		Array buildings_array;
		buildings_array.resize(buildings.size());
		for (size_t idx = 0; idx < buildings.size(); ++idx) {
			BuildingInstance const& building = buildings[idx];

			Dictionary building_dict;
			building_dict[get_building_info_building_key()] = std_to_godot_string(building.get_identifier());
			building_dict[get_building_info_level_key()] = static_cast<int32_t>(building.get_current_level());
			building_dict[get_building_info_expansion_state_key()] = static_cast<int32_t>(building.get_expansion_state());
			building_dict[get_building_info_start_date_key()] = std_to_godot_string(building.get_start_date().to_string());
			building_dict[get_building_info_end_date_key()] = std_to_godot_string(building.get_end_date().to_string());
			building_dict[get_building_info_expansion_progress_key()] = building.get_expansion_progress();

			buildings_array[idx] = building_dict;
		}
		ret[get_province_info_buildings_key()] = buildings_array;
	}
	return ret;
}

int32_t GameSingleton::get_width() const {
	return game_manager.get_map().get_width();
}

int32_t GameSingleton::get_height() const {
	return game_manager.get_map().get_height();
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

Error GameSingleton::_update_colour_image() {
	static PackedByteArray colour_data_array;
	static constexpr int64_t colour_data_array_size = (static_cast<int64_t>(Province::MAX_INDEX) + 1) * Map::MAPMODE_COLOUR_SIZE;
	colour_data_array.resize(colour_data_array_size);

	Error err = OK;
	if (!game_manager.get_map().generate_mapmode_colours(mapmode_index, colour_data_array.ptrw()))
		err = FAILED;

	static constexpr int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(Province::index_t) * 4);
	if (province_colour_image.is_null()) {
		province_colour_image.instantiate();
		ERR_FAIL_NULL_V_EDMSG(province_colour_image, FAILED,
			"Failed to create province colour image");
	}
	province_colour_image->set_data(PROVINCE_INDEX_SQRT, PROVINCE_INDEX_SQRT,
		false, Image::FORMAT_RGBA8, colour_data_array);
	if (province_colour_texture.is_null()) {
		province_colour_texture = ImageTexture::create_from_image(province_colour_image);
		ERR_FAIL_NULL_V_EDMSG(province_colour_texture, FAILED,
			"Failed to create province colour texture");
	} else province_colour_texture->update(province_colour_image);
	return err;
}

int32_t GameSingleton::get_mapmode_count() const {
	return game_manager.get_map().get_mapmode_count();
}

String GameSingleton::get_mapmode_identifier(int32_t index) const {
	Mapmode const* mapmode = game_manager.get_map().get_mapmode_by_index(index);
	if (mapmode != nullptr) return std_to_godot_string(mapmode->get_identifier());
	return String {};
}

Error GameSingleton::set_mapmode(String const& identifier) {
	Mapmode const* mapmode = game_manager.get_map().get_mapmode_by_identifier(godot_to_std_string(identifier));
	if (mapmode == nullptr) {
		UtilityFunctions::push_error("Failed to set mapmode to: ", identifier);
		return FAILED;
	}
	mapmode_index = mapmode->get_index();
	_update_colour_image();
	return OK;
}

int32_t GameSingleton::get_selected_province_index() const {
	return game_manager.get_map().get_selected_province_index();
}

void GameSingleton::set_selected_province(int32_t index) {
	game_manager.get_map().set_selected_province(index);
	_update_colour_image();
	emit_signal("province_selected", index);
}

Error GameSingleton::expand_building(int32_t province_index, String const& building_type_identifier) {
	if (!game_manager.expand_building(province_index, godot_to_std_string(building_type_identifier))) {
		UtilityFunctions::push_error("Failed to expand ", building_type_identifier, " at province index ", province_index);
		return FAILED;
	}
	return OK;
}

void GameSingleton::set_paused(bool paused) {
	game_manager.get_clock().isPaused = paused;
}

void GameSingleton::toggle_paused() {
	game_manager.get_clock().isPaused = !game_manager.get_clock().isPaused;
}

bool GameSingleton::is_paused() const {
	return game_manager.get_clock().isPaused;
}

void GameSingleton::increase_speed() {
	game_manager.get_clock().increaseSimulationSpeed();
}

void GameSingleton::decrease_speed() {
	game_manager.get_clock().decreaseSimulationSpeed();
}

bool GameSingleton::can_increase_speed() const {
	return game_manager.get_clock().canIncreaseSimulationSpeed();
}

bool GameSingleton::can_decrease_speed() const {
	return game_manager.get_clock().canDecreaseSimulationSpeed();
}

String GameSingleton::get_longform_date() const {
	return std_to_godot_string(game_manager.get_today().to_string());
}

void GameSingleton::try_tick() {
	game_manager.get_clock().conditionallyAdvanceGame();
}

Error GameSingleton::_load_map_images(bool flip_vertical) {
	if (province_shape_texture.is_valid()) {
		UtilityFunctions::push_error("Map images have already been loaded!");
		return FAILED;
	}

	Error err = OK;

	const Vector2i province_dims {
		static_cast<int32_t>(game_manager.get_map().get_width()),
		static_cast<int32_t>(game_manager.get_map().get_height()) };

	static constexpr int32_t GPU_DIM_LIMIT = 0x3FFF;
	// For each dimension of the image, this finds the small number of equal subdivisions required get the individual texture dims under GPU_DIM_LIMIT
	for (int i = 0; i < 2; ++i)
		for (image_subdivisions[i] = 1; province_dims[i] / image_subdivisions[i] > GPU_DIM_LIMIT ||
			province_dims[i] % image_subdivisions[i] != 0; ++image_subdivisions[i]);

	Map::shape_pixel_t const* province_shape_data = game_manager.get_map().get_province_shape_image().data();
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

Error GameSingleton::_load_terrain_variants_compatibility_mode(String const& terrain_texturesheet_path) {
	static constexpr int32_t SHEET_DIMS = 8, SHEET_SIZE = SHEET_DIMS * SHEET_DIMS;

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

	Array terrain_images;
	{
		static constexpr colour_t TERRAIN_WATER_INDEX_COLOUR = 0xFFFFFF;
		Ref<Image> water_image = Image::create(slice_size, slice_size, false, terrain_sheet->get_format());
		ERR_FAIL_NULL_V_EDMSG(water_image, FAILED, "Failed to create water terrain image");
		water_image->fill({ 0.1f, 0.1f, 0.5f });
		terrain_images.append(water_image);
	}
	Error err = OK;
	for (int32_t idx = 0; idx < SHEET_SIZE; ++idx) {
		const Rect2i slice { (idx % SHEET_DIMS) * slice_size, (7 - (idx / SHEET_DIMS)) * slice_size, slice_size, slice_size };
		const Ref<Image> terrain_image = terrain_sheet->get_region(slice);
		if (terrain_image.is_null() || terrain_image->is_empty()) {
			UtilityFunctions::push_error("Failed to extract terrain texture slice ", slice, " from ", terrain_texturesheet_path);
			err = FAILED;
		}
		terrain_images.append(terrain_image);
	}

	terrain_texture.instantiate();
	if (terrain_texture->create_from_images(terrain_images) != OK) {
		UtilityFunctions::push_error("Failed to create terrain texture array!");
		return FAILED;
	}
	return err;
}

Error GameSingleton::load_defines_compatibility_mode(PackedStringArray const& file_paths) {
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
	if (_load_terrain_variants_compatibility_mode(
		std_to_godot_string(dataloader.lookup_file(terrain_texture_file).string())
		) != OK) {
		UtilityFunctions::push_error("Failed to load terrain variants!");
		err = FAILED;
	}
	if (_load_map_images(true) != OK) {
		UtilityFunctions::push_error("Failed to load map images!");
		err = FAILED;
	}
	if (!game_manager.load_hardcoded_defines()) {
		UtilityFunctions::push_error("Failed to hardcoded defines!");
		err = FAILED;
	}
	if (!dataloader.load_localisation_files(LoadLocalisation::add_message)) {
		UtilityFunctions::push_error("Failed to load localisation!");
		err = FAILED;
	}

	return err;
}

String GameSingleton::lookup_file(String const& path) const {
	return std_to_godot_string(dataloader.lookup_file(godot_to_std_string(path)).string());
}
