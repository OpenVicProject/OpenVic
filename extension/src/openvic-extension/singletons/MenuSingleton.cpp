#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/GameManager.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

StringName const& MenuSingleton::_signal_population_menu_province_list_changed() {
	static const StringName signal_population_menu_province_list_changed = "population_menu_province_list_changed";
	return signal_population_menu_province_list_changed;
}
StringName const& MenuSingleton::_signal_population_menu_province_list_selected_changed() {
	static const StringName signal_population_menu_province_list_selected_changed =
		"population_menu_province_list_selected_changed";
	return signal_population_menu_province_list_selected_changed;
}
StringName const& MenuSingleton::_signal_population_menu_pops_changed() {
	static const StringName signal_population_menu_pops_changed = "population_menu_pops_changed";
	return signal_population_menu_pops_changed;
}
StringName const& MenuSingleton::_signal_search_cache_changed() {
	static const StringName signal_search_cache_changed = "search_cache_changed";
	return signal_search_cache_changed;
}

String MenuSingleton::get_state_name(State const& state) const {
	StateSet const& state_set = state.get_state_set();

	const String region_identifier = Utilities::std_to_godot_string(state_set.get_region().get_identifier());

	String name = tr(region_identifier);

	const bool named = name != region_identifier;
	const bool owned = state.get_owner() != nullptr;
	const bool split = state_set.get_state_count() > 1;

	if (!named) {
		// Capital province name
		name = tr(GUINode::format_province_name(Utilities::std_to_godot_string(state.get_capital()->get_identifier())));

		if (!owned) {
			static const StringName region_key = "REGION_NAME";
			static const String name_key = "$NAME$";

			String region = tr(region_key);

			if (region != region_key) {
				// CAPITAL Region
				return region.replace(name_key, name);
			}
		}
	}

	if (owned && split) {
		// COUNTRY STATE/CAPITAL
		return get_country_adjective(*state.get_owner()) + " " + name;
	}

	// STATE/CAPITAL
	return name;
}

String MenuSingleton::get_country_name(CountryInstance const& country) const {
	if (country.get_government_type() != nullptr && !country.get_government_type()->get_identifier().empty()) {
		const String government_name_key = Utilities::std_to_godot_string(StringUtils::append_string_views(
			country.get_identifier(), "_", country.get_government_type()->get_identifier()
		));

		String government_name = tr(government_name_key);

		if (government_name != government_name_key) {
			return government_name;
		}
	}

	return tr(Utilities::std_to_godot_string(country.get_identifier()));
}

String MenuSingleton::get_country_adjective(CountryInstance const& country) const {
	static constexpr std::string_view adjective = "_ADJ";

	if (country.get_government_type() != nullptr && !country.get_government_type()->get_identifier().empty()) {
		const String government_adjective_key = Utilities::std_to_godot_string(StringUtils::append_string_views(
			country.get_identifier(), "_", country.get_government_type()->get_identifier(), adjective
		));

		String government_adjective = tr(government_adjective_key);

		if (government_adjective != government_adjective_key) {
			return government_adjective;
		}
	}

	return tr(Utilities::std_to_godot_string(StringUtils::append_string_views(country.get_identifier(), adjective)));
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

	OV_BIND_METHOD(MenuSingleton::population_menu_update_locale_sort_cache);
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

	/* Find/Search Panel */
	OV_BIND_METHOD(MenuSingleton::generate_search_cache);
	OV_BIND_METHOD(MenuSingleton::update_search_results, { "text" });
	OV_BIND_METHOD(MenuSingleton::get_search_result_rows, { "start", "count" });
	OV_BIND_METHOD(MenuSingleton::get_search_result_row_count);
	OV_BIND_METHOD(MenuSingleton::get_search_result_position, { "result_index" });

	ADD_SIGNAL(MethodInfo(_signal_search_cache_changed()));
}

MenuSingleton* MenuSingleton::get_singleton() {
	return singleton;
}

MenuSingleton::MenuSingleton() : population_menu {
	.pop_type_sort_cache { nullptr }, .culture_sort_cache { nullptr }, .religion_sort_cache { nullptr },
	.province_sort_cache { nullptr }, .rebel_type_sort_cache { nullptr }
} {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

MenuSingleton::~MenuSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

/* PROVINCE OVERVIEW PANEL */

static TypedArray<Dictionary> _make_buildings_dict_array(
	ProvinceInstance const* province
) {
	std::vector<BuildingInstance> const& buildings = province->get_buildings();

	if (buildings.empty()) {
		return {};
	}

	static const StringName building_info_level_key = "level";
	static const StringName building_info_expansion_state_key = "expansion_state";
	static const StringName building_info_start_date_key = "start_date";
	static const StringName building_info_end_date_key = "end_date";
	static const StringName building_info_expansion_progress_key = "expansion_progress";

	/* This system relies on the province buildings all being present in the right order. It will have to
	 * be changed if we want to support variable combinations and permutations of province buildings. */
	TypedArray<Dictionary> buildings_array;

	if (buildings_array.resize(buildings.size()) == OK) {
		for (size_t idx = 0; idx < buildings.size(); ++idx) {
			BuildingInstance const& building = buildings[idx];

			Dictionary building_dict;
			building_dict[building_info_level_key] = static_cast<int32_t>(building.get_level());
			building_dict[building_info_expansion_state_key] = static_cast<int32_t>(building.get_expansion_state());
			building_dict[building_info_start_date_key] = Utilities::std_to_godot_string(building.get_start_date().to_string());
			building_dict[building_info_end_date_key] = Utilities::std_to_godot_string(building.get_end_date().to_string());
			building_dict[building_info_expansion_progress_key] = building.get_expansion_progress();

			buildings_array[idx] = std::move(building_dict);
		}
	} else {
		UtilityFunctions::push_error(
			"Failed to resize buildings array to the correct size (", static_cast<int64_t>(buildings.size()),
			") for province ", Utilities::std_to_godot_string(province->get_identifier())
		);
	}

	return buildings_array;
}

Dictionary MenuSingleton::get_province_info_from_index(int32_t index) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	static const StringName province_info_province_key = "province";
	static const StringName province_info_state_key = "state";
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

	ProvinceInstance const* province = instance_manager->get_map_instance().get_province_instance_by_index(index);
	if (province == nullptr) {
		return {};
	}
	Dictionary ret;

	ret[province_info_province_key] = Utilities::std_to_godot_string(province->get_identifier());

	State const* state = province->get_state();
	if (state != nullptr) {
		ret[province_info_state_key] = get_state_name(*state);
	}

	ret[province_info_slave_status_key] = province->get_slave();

	ret[province_info_colony_status_key] = static_cast<int32_t>(province->get_colony_status());

	TerrainType const* terrain_type = province->get_terrain_type();
	if (terrain_type != nullptr) {
		ret[province_info_terrain_type_key] = Utilities::std_to_godot_string(terrain_type->get_identifier());
	}

	ret[province_info_life_rating_key] = province->get_life_rating();

	CountryInstance const* controller = province->get_controller();
	if (controller != nullptr) {
		ret[province_info_controller_key] = Utilities::std_to_godot_string(controller->get_identifier());
	}

	GoodDefinition const* rgo = province->get_rgo();
	if (rgo != nullptr) {
		ret[province_info_rgo_name_key] = Utilities::std_to_godot_string(rgo->get_identifier());
		ret[province_info_rgo_icon_key] = static_cast<int32_t>(rgo->get_index());
	}

	Crime const* crime = province->get_crime();
	if (crime != nullptr) {
		ret[province_info_crime_name_key] = Utilities::std_to_godot_string(crime->get_identifier());
		ret[province_info_crime_icon_key] = static_cast<int32_t>(crime->get_icon());
	}

	ret[province_info_total_population_key] = province->get_total_population();

	GFXPieChartTexture::godot_pie_chart_data_t pop_types =
		GFXPieChartTexture::distribution_to_slices_array(province->get_pop_type_distribution());
	if (!pop_types.is_empty()) {
		ret[province_info_pop_types_key] = std::move(pop_types);
	}

	GFXPieChartTexture::godot_pie_chart_data_t ideologies =
		GFXPieChartTexture::distribution_to_slices_array(province->get_ideology_distribution());
	if (!ideologies.is_empty()) {
		ret[province_info_pop_ideologies_key] = std::move(ideologies);
	}

	GFXPieChartTexture::godot_pie_chart_data_t cultures =
		GFXPieChartTexture::distribution_to_slices_array(province->get_culture_distribution());
	if (!cultures.is_empty()) {
		ret[province_info_pop_cultures_key] = std::move(cultures);
	}

	ordered_set<CountryInstance*> const& cores = province->get_cores();
	if (!cores.empty()) {
		PackedStringArray cores_array;
		if (cores_array.resize(cores.size()) == OK) {
			for (size_t idx = 0; idx < cores.size(); ++idx) {
				cores_array[idx] = Utilities::std_to_godot_string(cores.data()[idx]->get_identifier());
			}
			ret[province_info_cores_key] = std::move(cores_array);
		} else {
			UtilityFunctions::push_error(
				"Failed to resize cores array to the correct size (", static_cast<int64_t>(cores.size()), ") for province ",
				Utilities::std_to_godot_string(province->get_identifier())
			);
		}
	}

	TypedArray<Dictionary> building_dict_array = _make_buildings_dict_array(province);
	if (!building_dict_array.is_empty()) {
		ret[province_info_buildings_key] = std::move(building_dict_array);
	}

	return ret;
}

int32_t MenuSingleton::get_province_building_count() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);

	return game_singleton->get_definition_manager().get_economy_manager().get_building_type_manager()
		.get_province_building_types().size();
}

String MenuSingleton::get_province_building_identifier(int32_t building_index) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	std::vector<BuildingType const*> const& province_building_types = game_singleton->get_definition_manager()
		.get_economy_manager().get_building_type_manager().get_province_building_types();
	ERR_FAIL_COND_V_MSG(
		building_index < 0 || building_index >= province_building_types.size(), {},
		vformat("Invalid province building index: %d", building_index)
	);
	return Utilities::std_to_godot_string(province_building_types[building_index]->get_identifier());
}

Error MenuSingleton::expand_selected_province_building(int32_t building_index) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	InstanceManager* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, FAILED);

	ERR_FAIL_COND_V_MSG(
		!instance_manager->expand_selected_province_building(building_index), FAILED,
		vformat("Failed to expand the currently selected province's building index %d", building_index)
	);
	return OK;
}

int32_t MenuSingleton::get_slave_pop_icon_index() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);

	const PopType::sprite_t sprite = game_singleton->get_definition_manager().get_pop_manager().get_slave_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "Slave sprite unset!");
	return sprite;
}

int32_t MenuSingleton::get_administrative_pop_icon_index() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);

	const PopType::sprite_t sprite = game_singleton->get_definition_manager().get_pop_manager().get_administrative_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "Administrative sprite unset!");
	return sprite;
}

int32_t MenuSingleton::get_rgo_owner_pop_icon_index() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);

	const PopType::sprite_t sprite = game_singleton->get_definition_manager().get_economy_manager().get_production_type_manager().get_rgo_owner_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "RGO owner sprite unset!");
	return sprite;
}

/* TIME/SPEED CONTROL PANEL */

void MenuSingleton::set_paused(bool paused) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL(game_singleton);
	InstanceManager* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->get_simulation_clock().set_paused(paused);
}

void MenuSingleton::toggle_paused() {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL(game_singleton);
	InstanceManager* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->get_simulation_clock().toggle_paused();
}

bool MenuSingleton::is_paused() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, true);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, true);

	return instance_manager->get_simulation_clock().is_paused();
}

void MenuSingleton::increase_speed() {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL(game_singleton);
	InstanceManager* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->get_simulation_clock().increase_simulation_speed();
}

void MenuSingleton::decrease_speed() {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL(game_singleton);
	InstanceManager* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->get_simulation_clock().decrease_simulation_speed();
}

int32_t MenuSingleton::get_speed() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, 0);

	return instance_manager->get_simulation_clock().get_simulation_speed();
}

bool MenuSingleton::can_increase_speed() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, false);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, false);

	return instance_manager->get_simulation_clock().can_increase_simulation_speed();
}

bool MenuSingleton::can_decrease_speed() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, false);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, false);

	return instance_manager->get_simulation_clock().can_decrease_simulation_speed();
}

String MenuSingleton::get_longform_date() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	return Utilities::date_to_formatted_string(instance_manager->get_today());
}

Error MenuSingleton::generate_search_cache() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, FAILED);

	search_panel.entry_cache.clear();

	std::vector<ProvinceInstance> const& provinces = instance_manager->get_map_instance().get_province_instances();
	std::vector<StateSet> const& state_sets = instance_manager->get_map_instance().get_state_manager().get_state_sets();
	std::vector<CountryInstance> const& countries = instance_manager->get_country_instance_manager().get_country_instances();

	// TODO - reserve actual state count rather than state set count (maybe use a vector of pointers to all states?)
	search_panel.entry_cache.reserve(provinces.size() + state_sets.size() + countries.size());

	for (ProvinceInstance const& province : provinces) {
		String identifier = Utilities::std_to_godot_string(province.get_identifier());
		String display_name = tr(GUINode::format_province_name(identifier));
		String search_name = display_name.to_lower();

		search_panel.entry_cache.push_back({
			&province, std::move(display_name), std::move(search_name), identifier.to_lower()
		});
	}

	for (StateSet const& state_set : state_sets) {
		for (State const& state : state_set.get_states()) {
			String display_name = get_state_name(state);
			String search_name = display_name.to_lower();

			search_panel.entry_cache.push_back({
				// TODO - include state identifier? (region and/or split?)
				&state, std::move(display_name), std::move(search_name), {}
			});
		}
	}

	for (CountryInstance const& country : countries) {
		// TODO - replace with a proper "exists" check
		if (country.get_capital() != nullptr) {
			String display_name = get_country_name(country);
			String search_name = display_name.to_lower();

			search_panel.entry_cache.push_back({
				&country, std::move(display_name), std::move(search_name),
				Utilities::std_to_godot_string(country.get_identifier()).to_lower()
			});
		}
	}

	std::sort(search_panel.entry_cache.begin(), search_panel.entry_cache.end(), [](auto const& a, auto const& b) -> bool {
		return a.search_name < b.search_name;
	});

	emit_signal(_signal_search_cache_changed());

	return OK;
}

void MenuSingleton::update_search_results(String const& text) {
	// Sanatise input
	const String search_text = text.strip_edges().to_lower();

	search_panel.result_indices.clear();

	if (!search_text.is_empty()) {
		// Search through cache
		for (size_t idx = 0; idx < search_panel.entry_cache.size(); ++idx) {
			search_panel_t::entry_t const& entry = search_panel.entry_cache[idx];

			if (entry.search_name.begins_with(search_text) || entry.identifier == search_text) {
				search_panel.result_indices.push_back(idx);
			}
		}
	}
}

PackedStringArray MenuSingleton::get_search_result_rows(int32_t start, int32_t count) const {
	if (search_panel.result_indices.empty()) {
		return {};
	}

	ERR_FAIL_INDEX_V_MSG(
		start, search_panel.result_indices.size(), {},
		vformat("Invalid start for search panel result rows: %d", start)
	);
	ERR_FAIL_COND_V_MSG(count <= 0, {}, vformat("Invalid count for search panel result rows: %d", count));

	if (start + count > search_panel.result_indices.size()) {
		UtilityFunctions::push_warning(
			"Requested search result rows beyond the end of the result indices (", start, " + ", count, " > ",
			static_cast<int64_t>(search_panel.result_indices.size()), "), limiting to ",
			static_cast<int64_t>(search_panel.result_indices.size() - start), " rows."
		);
		count = search_panel.result_indices.size() - start;
	}

	PackedStringArray results;
	results.resize(count);

	for (size_t idx = 0; idx < count; ++idx) {
		results[idx] = search_panel.entry_cache[search_panel.result_indices[start + idx]].display_name;
	}

	return results;
}

int32_t MenuSingleton::get_search_result_row_count() const {
	return search_panel.result_indices.size();
}

Vector2 MenuSingleton::get_search_result_position(int32_t result_index) const {
	ERR_FAIL_INDEX_V(result_index, search_panel.result_indices.size(), {});

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	struct entry_visitor_t {
		fvec2_t operator()(ProvinceInstance const* province) {
			return province->get_province_definition().get_centre();
		}

		fvec2_t operator()(State const* state) {
			return (*this)(state->get_capital());
		}

		fvec2_t operator()(CountryInstance const* country) {
			return (*this)(country->get_capital());
		}
	} entry_visitor;

	return game_singleton->normalise_map_position(
		std::visit(entry_visitor, search_panel.entry_cache[search_panel.result_indices[result_index]].target)
	);
}
