#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/GameManager.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::std_to_godot_string;
using OpenVic::Utilities::std_view_to_godot_string;

StringName const& MenuSingleton::_signal_population_menu_province_list_changed() {
	static const StringName signal_population_menu_province_list_changed = "population_menu_province_list_changed";
	return signal_population_menu_province_list_changed;
}
StringName const& MenuSingleton::_signal_population_menu_province_list_selected_changed() {
	static const StringName signal_population_menu_province_list_selected_changed = "population_menu_province_list_selected_changed";
	return signal_population_menu_province_list_selected_changed;
}
StringName const& MenuSingleton::_signal_population_menu_pops_changed() {
	static const StringName signal_population_menu_pops_changed = "population_menu_pops_changed";
	return signal_population_menu_pops_changed;
}

void MenuSingleton::_bind_methods() {
	/* PROVINCE OVERVIEW PANEL */
	OV_BIND_METHOD(MenuSingleton::get_province_info_from_index, { "index" });
	OV_BIND_METHOD(MenuSingleton::get_province_building_count);
	OV_BIND_METHOD(MenuSingleton::get_province_building_identifier, { "building_index" });
	OV_BIND_METHOD(MenuSingleton::expand_selected_province_building, { "building_index" });
	OV_BIND_METHOD(MenuSingleton::get_slave_pop_icon_index);
	OV_BIND_METHOD(MenuSingleton::get_administrative_pop_icon_index);
	OV_BIND_METHOD(MenuSingleton::get_rgo_owner_pop_icon_index);

	/* TIME/SPEED CONTROL PANEL */
	OV_BIND_METHOD(MenuSingleton::set_paused, { "paused" });
	OV_BIND_METHOD(MenuSingleton::toggle_paused);
	OV_BIND_METHOD(MenuSingleton::is_paused);
	OV_BIND_METHOD(MenuSingleton::increase_speed);
	OV_BIND_METHOD(MenuSingleton::decrease_speed);
	OV_BIND_METHOD(MenuSingleton::get_speed);
	OV_BIND_METHOD(MenuSingleton::can_increase_speed);
	OV_BIND_METHOD(MenuSingleton::can_decrease_speed);
	OV_BIND_METHOD(MenuSingleton::get_longform_date);

	/* POPULATION MENU */
	OV_BIND_METHOD(MenuSingleton::get_population_menu_province_list_row_count);
	OV_BIND_METHOD(MenuSingleton::get_population_menu_province_list_rows, { "start", "count" });
	OV_BIND_METHOD(
		MenuSingleton::population_menu_select_province_list_entry, { "select_index", "set_scroll_index" }, DEFVAL(false)
	);
	OV_BIND_METHOD(MenuSingleton::population_menu_select_province, { "province_index" });
	OV_BIND_METHOD(MenuSingleton::population_menu_toggle_expanded, { "toggle_index", "emit_selected_changed" }, DEFVAL(true));

	OV_BIND_METHOD(MenuSingleton::population_menu_select_sort_key, { "sort_key" });
	OV_BIND_METHOD(MenuSingleton::get_population_menu_pop_rows, { "start", "count" });
	OV_BIND_METHOD(MenuSingleton::get_population_menu_pop_row_count);

	OV_BIND_METHOD(MenuSingleton::get_population_menu_pop_filter_setup_info);
	OV_BIND_METHOD(MenuSingleton::get_population_menu_pop_filter_info);
	OV_BIND_METHOD(MenuSingleton::population_menu_toggle_pop_filter, { "filter_index" });
	OV_BIND_METHOD(MenuSingleton::population_menu_select_all_pop_filters);
	OV_BIND_METHOD(MenuSingleton::population_menu_deselect_all_pop_filters);

	OV_BIND_METHOD(MenuSingleton::get_population_menu_distribution_setup_info);
	OV_BIND_METHOD(MenuSingleton::get_population_menu_distribution_info);

	ADD_SIGNAL(MethodInfo(_signal_population_menu_province_list_changed()));
	ADD_SIGNAL(
		MethodInfo(_signal_population_menu_province_list_selected_changed(), PropertyInfo(Variant::INT, "scroll_index"))
	);
	ADD_SIGNAL(MethodInfo(_signal_population_menu_pops_changed()));

	using enum population_menu_t::ProvinceListEntry;
	BIND_ENUM_CONSTANT(LIST_ENTRY_NONE);
	BIND_ENUM_CONSTANT(LIST_ENTRY_COUNTRY);
	BIND_ENUM_CONSTANT(LIST_ENTRY_STATE);
	BIND_ENUM_CONSTANT(LIST_ENTRY_PROVINCE);

	using enum population_menu_t::PopSortKey;
	BIND_ENUM_CONSTANT(NONE);
	BIND_ENUM_CONSTANT(SORT_SIZE);
	BIND_ENUM_CONSTANT(SORT_TYPE);
	BIND_ENUM_CONSTANT(SORT_CULTURE);
	BIND_ENUM_CONSTANT(SORT_RELIGION);
	BIND_ENUM_CONSTANT(SORT_LOCATION);
	BIND_ENUM_CONSTANT(SORT_MILITANCY);
	BIND_ENUM_CONSTANT(SORT_CONSCIOUSNESS);
	BIND_ENUM_CONSTANT(SORT_IDEOLOGY);
	BIND_ENUM_CONSTANT(SORT_ISSUES);
	BIND_ENUM_CONSTANT(SORT_UNEMPLOYMENT);
	BIND_ENUM_CONSTANT(SORT_CASH);
	BIND_ENUM_CONSTANT(SORT_LIFE_NEEDS);
	BIND_ENUM_CONSTANT(SORT_EVERYDAY_NEEDS);
	BIND_ENUM_CONSTANT(SORT_LUXURY_NEEDS);
	BIND_ENUM_CONSTANT(SORT_REBEL_FACTION);
	BIND_ENUM_CONSTANT(SORT_SIZE_CHANGE);
	BIND_ENUM_CONSTANT(SORT_LITERACY);
}

MenuSingleton* MenuSingleton::get_singleton() {
	return singleton;
}

MenuSingleton::MenuSingleton() : game_manager {
	[]() -> GameManager* {
		GameSingleton* game_singleton = GameSingleton::get_singleton();
		ERR_FAIL_NULL_V_MSG(
			game_singleton, nullptr, "Cannot initialise MenuSingleton's GameManager pointer - GameSingleton not initialised!"
		);
		return &game_singleton->get_game_manager();
	}()
} {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

MenuSingleton::~MenuSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

/* PROVINCE OVERVIEW PANEL */

Dictionary MenuSingleton::get_province_info_from_index(int32_t index) const {
	ERR_FAIL_NULL_V(game_manager, {});

	static const StringName province_info_province_key = "province";
	static const StringName province_info_region_key = "region";
	static const StringName province_info_slave_status_key = "slave_status";
	static const StringName province_info_colony_status_key = "colony_status";
	static const StringName province_info_terrain_type_key = "terrain_type";
	static const StringName province_info_life_rating_key = "life_rating";
	static const StringName province_info_controller_key = "controller";
	static const StringName province_info_rgo_name_key = "rgo_name";
	static const StringName province_info_rgo_icon_key = "rgo_icon";
	static const StringName province_info_crime_name_key = "crime_name";
	static const StringName province_info_crime_icon_key = "crime_icon";
	static const StringName province_info_total_population_key = "total_population";
	static const StringName province_info_pop_types_key = "pop_types";
	static const StringName province_info_pop_ideologies_key = "pop_ideologies";
	static const StringName province_info_pop_cultures_key = "pop_cultures";
	static const StringName province_info_cores_key = "cores";
	static const StringName province_info_buildings_key = "buildings";

	Province const* province = game_manager->get_map().get_province_by_index(index);
	if (province == nullptr) {
		return {};
	}
	Dictionary ret;

	ret[province_info_province_key] = std_view_to_godot_string(province->get_identifier());

	Region const* region = province->get_region();
	if (region != nullptr) {
		ret[province_info_region_key] = std_view_to_godot_string(region->get_identifier());
	}

	ret[province_info_slave_status_key] = province->get_slave();

	ret[province_info_colony_status_key] = static_cast<int32_t>(province->get_colony_status());

	TerrainType const* terrain_type = province->get_terrain_type();
	if (terrain_type != nullptr) {
		ret[province_info_terrain_type_key] = std_view_to_godot_string(terrain_type->get_identifier());
	}

	ret[province_info_life_rating_key] = province->get_life_rating();

	Country const* controller = province->get_controller();
	if (controller != nullptr) {
		ret[province_info_controller_key] = std_view_to_godot_string(controller->get_identifier());
	}

	Good const* rgo = province->get_rgo();
	if (rgo != nullptr) {
		ret[province_info_rgo_name_key] = std_view_to_godot_string(rgo->get_identifier());
		ret[province_info_rgo_icon_key] = static_cast<int32_t>(rgo->get_index());
	}

	Crime const* crime = province->get_crime();
	if (crime != nullptr) {
		ret[province_info_crime_name_key] = std_view_to_godot_string(crime->get_identifier());
		ret[province_info_crime_icon_key] = static_cast<int32_t>(crime->get_icon());
	}

	ret[province_info_total_population_key] = province->get_total_population();

	fixed_point_map_t<PopType const*> const& pop_types = province->get_pop_type_distribution();
	if (!pop_types.empty()) {
		ret[province_info_pop_types_key] = GFXPieChartTexture::distribution_to_slices_array(pop_types);
	}

	fixed_point_map_t<Ideology const*> const& ideologies = province->get_ideology_distribution();
	if (!ideologies.empty()) {
		ret[province_info_pop_ideologies_key] = GFXPieChartTexture::distribution_to_slices_array(ideologies);
	}

	fixed_point_map_t<Culture const*> const& cultures = province->get_culture_distribution();
	if (!cultures.empty()) {
		ret[province_info_pop_cultures_key] = GFXPieChartTexture::distribution_to_slices_array(cultures);
	}

	std::vector<Country const*> const& cores = province->get_cores();
	if (!cores.empty()) {
		PackedStringArray cores_array;
		if (cores_array.resize(cores.size()) == OK) {
			for (size_t idx = 0; idx < cores.size(); ++idx) {
				cores_array[idx] = std_view_to_godot_string(cores[idx]->get_identifier());
			}
			ret[province_info_cores_key] = std::move(cores_array);
		} else {
			UtilityFunctions::push_error(
				"Failed to resize cores array to the correct size (", static_cast<int64_t>(cores.size()), ") for province ",
				std_view_to_godot_string(province->get_identifier())
			);
		}
	}

	static const StringName building_info_level_key = "level";
	static const StringName building_info_expansion_state_key = "expansion_state";
	static const StringName building_info_start_date_key = "start_date";
	static const StringName building_info_end_date_key = "end_date";
	static const StringName building_info_expansion_progress_key = "expansion_progress";

	std::vector<BuildingInstance> const& buildings = province->get_buildings();
	if (!buildings.empty()) {
		/* This system relies on the province buildings all being present in the right order. It will have to
		 * be changed if we want to support variable combinations and permutations of province buildings. */
		TypedArray<Dictionary> buildings_array;
		if (buildings_array.resize(buildings.size()) == OK) {
			for (size_t idx = 0; idx < buildings.size(); ++idx) {
				BuildingInstance const& building = buildings[idx];

				Dictionary building_dict;
				building_dict[building_info_level_key] = static_cast<int32_t>(building.get_level());
				building_dict[building_info_expansion_state_key] = static_cast<int32_t>(building.get_expansion_state());
				building_dict[building_info_start_date_key] = std_to_godot_string(building.get_start_date().to_string());
				building_dict[building_info_end_date_key] = std_to_godot_string(building.get_end_date().to_string());
				building_dict[building_info_expansion_progress_key] = building.get_expansion_progress();

				buildings_array[idx] = std::move(building_dict);
			}
			ret[province_info_buildings_key] = std::move(buildings_array);
		} else {
			UtilityFunctions::push_error(
				"Failed to resize buildings array to the correct size (", static_cast<int64_t>(buildings.size()),
				") for province ", std_view_to_godot_string(province->get_identifier())
			);
		}
	}
	return ret;
}

int32_t MenuSingleton::get_province_building_count() const {
	ERR_FAIL_NULL_V(game_manager, 0);

	return game_manager->get_economy_manager().get_building_type_manager().get_province_building_types().size();
}

String MenuSingleton::get_province_building_identifier(int32_t building_index) const {
	ERR_FAIL_NULL_V(game_manager, {});

	std::vector<BuildingType const*> const& province_building_types =
		game_manager->get_economy_manager().get_building_type_manager().get_province_building_types();
	ERR_FAIL_COND_V_MSG(
		building_index < 0 || building_index >= province_building_types.size(), {},
		vformat("Invalid province building index: %d", building_index)
	);
	return std_view_to_godot_string(province_building_types[building_index]->get_identifier());
}

Error MenuSingleton::expand_selected_province_building(int32_t building_index) {
	ERR_FAIL_NULL_V(game_manager, FAILED);

	ERR_FAIL_COND_V_MSG(
		!game_manager->expand_selected_province_building(building_index), FAILED,
		vformat("Failed to expand the currently selected province's building index %d", building_index)
	);
	return OK;
}

int32_t MenuSingleton::get_slave_pop_icon_index() const {
	ERR_FAIL_NULL_V(game_manager, 0);

	const PopType::sprite_t sprite = game_manager->get_pop_manager().get_slave_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "Slave sprite unset!");
	return sprite;
}

int32_t MenuSingleton::get_administrative_pop_icon_index() const {
	ERR_FAIL_NULL_V(game_manager, 0);

	const PopType::sprite_t sprite = game_manager->get_pop_manager().get_administrative_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "Administrative sprite unset!");
	return sprite;
}

int32_t MenuSingleton::get_rgo_owner_pop_icon_index() const {
	ERR_FAIL_NULL_V(game_manager, 0);

	const PopType::sprite_t sprite = game_manager->get_economy_manager().get_production_type_manager().get_rgo_owner_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "RGO owner sprite unset!");
	return sprite;
}

/* TIME/SPEED CONTROL PANEL */

void MenuSingleton::set_paused(bool paused) {
	ERR_FAIL_NULL(game_manager);

	game_manager->get_simulation_clock().set_paused(paused);
}

void MenuSingleton::toggle_paused() {
	ERR_FAIL_NULL(game_manager);

	game_manager->get_simulation_clock().toggle_paused();
}

bool MenuSingleton::is_paused() const {
	ERR_FAIL_NULL_V(game_manager, true);

	return game_manager->get_simulation_clock().is_paused();
}

void MenuSingleton::increase_speed() {
	ERR_FAIL_NULL(game_manager);

	game_manager->get_simulation_clock().increase_simulation_speed();
}

void MenuSingleton::decrease_speed() {
	ERR_FAIL_NULL(game_manager);

	game_manager->get_simulation_clock().decrease_simulation_speed();
}

int32_t MenuSingleton::get_speed() const {
	ERR_FAIL_NULL_V(game_manager, 0);

	return game_manager->get_simulation_clock().get_simulation_speed();
}

bool MenuSingleton::can_increase_speed() const {
	ERR_FAIL_NULL_V(game_manager, false);

	return game_manager->get_simulation_clock().can_increase_simulation_speed();
}

bool MenuSingleton::can_decrease_speed() const {
	ERR_FAIL_NULL_V(game_manager, false);

	return game_manager->get_simulation_clock().can_decrease_simulation_speed();
}

String MenuSingleton::get_longform_date() const {
	ERR_FAIL_NULL_V(game_manager, {});

	return Utilities::date_to_formatted_string(game_manager->get_today());
}
