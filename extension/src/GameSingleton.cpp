#include "GameSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic/utility/Logger.hpp"

#include "Utilities.hpp"

using namespace godot;
using namespace OpenVic;

TerrainVariant::TerrainVariant(const std::string_view new_identfier,
	colour_t new_colour, Ref<Image> const& new_image)
	: HasIdentifierAndColour { new_identfier, new_colour, true },
	  image { new_image } {}

Ref<Image> TerrainVariant::get_image() const {
	return image;
}

GameSingleton* GameSingleton::singleton = nullptr;

void GameSingleton::_bind_methods() {
	ClassDB::bind_static_method("GameSingleton", D_METHOD("setup_logger"), &GameSingleton::setup_logger);
	ClassDB::bind_method(D_METHOD("load_defines", "file_dict"), &GameSingleton::load_defines);
	ClassDB::bind_method(D_METHOD("load_defines_compatibility_mode", "file_path"), &GameSingleton::load_defines_compatibility_mode);
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
	ClassDB::bind_method(D_METHOD("get_good_icon_texture", "identifier"), &GameSingleton::get_good_icon_texture);

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

	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_identifier_file_key"), &GameSingleton::get_province_identifier_file_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_water_province_file_key"), &GameSingleton::get_water_province_file_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_region_file_key"), &GameSingleton::get_region_file_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_terrain_variant_file_key"), &GameSingleton::get_terrain_variant_file_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_terrain_texture_dir_key"), &GameSingleton::get_terrain_texture_dir_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_image_file_key"), &GameSingleton::get_province_image_file_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_terrain_image_file_key"), &GameSingleton::get_terrain_image_file_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_goods_file_key"), &GameSingleton::get_goods_file_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_good_icons_dir_key"), &GameSingleton::get_good_icons_dir_key);

	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_province_key"), &GameSingleton::get_province_info_province_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_region_key"), &GameSingleton::get_province_info_region_key);
	ClassDB::bind_static_method("GameSingleton", D_METHOD("get_province_info_life_rating_key"), &GameSingleton::get_province_info_life_rating_key);
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
}

void GameSingleton::draw_pie_chart(Ref<Image> image,
	Array const& stopAngles, Array const& colours,	float radius,
	Vector2 shadow_displacement, float shadow_tightness, float shadow_radius, float shadow_thickness,
	Color trim_colour, float trim_size, float gradient_falloff, float gradient_base,
	bool donut, bool donut_inner_trim, float donut_inner_radius) {

	OpenVic::draw_pie_chart(image, stopAngles, colours, radius, shadow_displacement, shadow_tightness, shadow_radius, shadow_thickness,
		trim_colour, trim_size, gradient_falloff, gradient_base,
		donut, donut_inner_trim, donut_inner_radius);
}

GameSingleton* GameSingleton::get_singleton() {
	return singleton;
}

void GameSingleton::_on_state_updated() {
	_update_colour_image();
	emit_signal("state_updated");
}

/* REQUIREMENTS:
 * MAP-21, MAP-23, MAP-25, MAP-32, MAP-33
 */
GameSingleton::GameSingleton() : game_manager { [this]() { _on_state_updated(); } },
								 terrain_variants { "terrain variants" } {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

void GameSingleton::setup_logger() {
	Logger::set_info_func([](std::string&& str) { UtilityFunctions::print(std_to_godot_string(str)); });
	Logger::set_error_func([](std::string&& str) { UtilityFunctions::push_error(std_to_godot_string(str)); });
}

Error GameSingleton::_load_hardcoded_defines() {
	Error err = OK;

	static constexpr colour_t LOW_ALPHA_VALUE = float_to_alpha_value(0.4f);
	static constexpr colour_t HIGH_ALPHA_VALUE = float_to_alpha_value(0.7f);
	using mapmode_t = std::pair<std::string, Mapmode::colour_func_t>;
	const std::vector<mapmode_t> mapmodes {
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
				if (region != nullptr) return HIGH_ALPHA_VALUE | region->get_colour();
				return NULL_COLOUR;
			} },
		{ "mapmode_index",
			[](Map const& map, Province const& province) -> colour_t {
				const colour_t f = fraction_to_colour_byte(province.get_index(), map.get_province_count() + 1);
				return HIGH_ALPHA_VALUE | (f << 16) | (f << 8) | f;
			} },
		{ "mapmode_rgo",
			[](Map const& map, Province const& province) -> colour_t {
				Good const* rgo = province.get_rgo();
				if (rgo != nullptr) return HIGH_ALPHA_VALUE | rgo->get_colour();
				return NULL_COLOUR;
			} },
		{ "mapmode_infrastructure",
			[](Map const& map, Province const& province) -> colour_t {
				Building const* railroad = province.get_building_by_identifier("building_railroad");
				if (railroad != nullptr) {
					colour_t val = fraction_to_colour_byte(railroad->get_level(), railroad->get_type().get_max_level() + 1, 0.5f, 1.0f);
					switch (railroad->get_expansion_state()) {
						case Building::ExpansionState::CannotExpand: val <<= 16; break;
						case Building::ExpansionState::CanExpand: break;
						default: val <<= 8; break;
					}
					return HIGH_ALPHA_VALUE | val;
				}
				return NULL_COLOUR;
			} },
		{ "mapmode_population",
			[](Map const& map, Province const& province) -> colour_t {
				return HIGH_ALPHA_VALUE | (fraction_to_colour_byte(province.get_total_population(), map.get_highest_province_population() + 1, 0.1f, 1.0f) << 8);
			} },
		{ "mapmode_culture",
			[](Map const& map, Province const& province) -> colour_t {
				distribution_t const& cultures = province.get_culture_distribution();
				if (!cultures.empty()) {
					// This breaks if replaced with distribution_t::value_type, something
					// about operator=(volatile const&) being deleted.
					std::pair<HasIdentifierAndColour const*, float> culture = *cultures.begin();
					for (distribution_t::value_type const p : cultures) {
						if (p.second > culture.second) culture = p;
					}
					return HIGH_ALPHA_VALUE | culture.first->get_colour();
				}
				return NULL_COLOUR;
			} }
	};
	for (mapmode_t const& mapmode : mapmodes)
		if (game_manager.map.add_mapmode(mapmode.first, mapmode.second) != SUCCESS)
			err = FAILED;
	game_manager.map.lock_mapmodes();

	using building_type_t = std::tuple<std::string, Building::level_t, Timespan>;
	const std::vector<building_type_t> building_types {
		{ "building_fort", 4, 8 }, { "building_naval_base", 6, 15 }, { "building_railroad", 5, 10 }
	};
	for (building_type_t const& type : building_types)
		if (game_manager.building_manager.add_building_type(std::get<0>(type), std::get<1>(type), std::get<2>(type)) != SUCCESS)
			err = FAILED;
	game_manager.building_manager.lock_building_types();

	return err;
}

GameSingleton::~GameSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

Error GameSingleton::setup_game() {
	return ERR(game_manager.setup());
}

int32_t GameSingleton::get_province_index_from_uv_coords(Vector2 const& coords) const {
	const size_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * get_width();
	const size_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * get_height();
	return game_manager.map.get_province_index_at(x_mod_w, y_mod_h);
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
	Province const* province = game_manager.map.get_province_by_index(index);
	if (province == nullptr) return {};
	Dictionary ret;

	ret[get_province_info_province_key()] = std_to_godot_string(province->get_identifier());

	Region const* region = province->get_region();
	if (region != nullptr) ret[get_province_info_region_key()] = std_to_godot_string(region->get_identifier());

	Good const* rgo = province->get_rgo();
	if (rgo != nullptr) ret[get_province_info_rgo_key()] = std_to_godot_string(rgo->get_identifier());

	ret[get_province_info_life_rating_key()] = province->get_life_rating();
	ret[get_province_info_total_population_key()] = province->get_total_population();
	distribution_t const& pop_types = province->get_pop_type_distribution();
	if (!pop_types.empty()) ret[get_province_info_pop_types_key()] = _distribution_to_dictionary(pop_types);
	//distribution_t const& ideologies = province->get_ideology_distribution();
	//if (!ideologies.empty()) ret[get_province_info_pop_ideologies_key()] = _distribution_to_dictionary(ideologies);
	distribution_t const& cultures = province->get_culture_distribution();
	if (!cultures.empty()) ret[get_province_info_pop_cultures_key()] = _distribution_to_dictionary(cultures);

	std::vector<Building> const& buildings = province->get_buildings();
	if (!buildings.empty()) {
		Array buildings_array;
		buildings_array.resize(buildings.size());
		for (size_t idx = 0; idx < buildings.size(); ++idx) {
			Building const& building = buildings[idx];

			Dictionary building_dict;
			building_dict[get_building_info_building_key()] = std_to_godot_string(building.get_identifier());
			building_dict[get_building_info_level_key()] = static_cast<int32_t>(building.get_level());
			building_dict[get_building_info_expansion_state_key()] = static_cast<int32_t>(building.get_expansion_state());
			building_dict[get_building_info_start_date_key()] = std_to_godot_string(static_cast<std::string>(building.get_start_date()));
			building_dict[get_building_info_end_date_key()] = std_to_godot_string(static_cast<std::string>(building.get_end_date()));
			building_dict[get_building_info_expansion_progress_key()] = building.get_expansion_progress();

			buildings_array[idx] = building_dict;
		}
		ret[get_province_info_buildings_key()] = buildings_array;
	}
	return ret;
}

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

Error GameSingleton::_update_colour_image() {
	static PackedByteArray colour_data_array;
	static constexpr int64_t colour_data_array_size = (static_cast<int64_t>(Province::MAX_INDEX) + 1) * Map::MAPMODE_COLOUR_SIZE;
	colour_data_array.resize(colour_data_array_size);

	Error err = OK;
	if (game_manager.map.generate_mapmode_colours(mapmode_index, colour_data_array.ptrw()) != SUCCESS)
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
	return game_manager.map.get_mapmode_count();
}

String GameSingleton::get_mapmode_identifier(int32_t index) const {
	Mapmode const* mapmode = game_manager.map.get_mapmode_by_index(index);
	if (mapmode != nullptr) return std_to_godot_string(mapmode->get_identifier());
	return String {};
}

Error GameSingleton::set_mapmode(String const& identifier) {
	Mapmode const* mapmode = game_manager.map.get_mapmode_by_identifier(godot_to_std_string(identifier));
	if (mapmode == nullptr) {
		UtilityFunctions::push_error("Failed to set mapmode to: ", identifier);
		return FAILED;
	}
	mapmode_index = mapmode->get_index();
	_update_colour_image();
	return OK;
}

int32_t GameSingleton::get_selected_province_index() const {
	return game_manager.map.get_selected_province_index();
}

void GameSingleton::set_selected_province(int32_t index) {
	game_manager.map.set_selected_province(index);
	_update_colour_image();
	emit_signal("province_selected", index);
}

Error GameSingleton::expand_building(int32_t province_index, String const& building_type_identifier) {
	if (game_manager.expand_building(province_index, godot_to_std_string(building_type_identifier)) != SUCCESS) {
		UtilityFunctions::push_error("Failed to expand ", building_type_identifier, " at province index ", province_index);
		return FAILED;
	}
	return OK;
}

Ref<Texture> GameSingleton::get_good_icon_texture(String const& identifier) const {
	return good_icons.get(identifier, {});
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
	return std_to_godot_string(static_cast<std::string>(game_manager.get_today()));
}

void GameSingleton::try_tick() {
	game_manager.clock.conditionallyAdvanceGame();
}
