#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/economy/GoodDefinition.hpp>
#include <openvic-simulation/GameManager.hpp>
#include <openvic-simulation/modifier/Modifier.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
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
StringName const& MenuSingleton::_signal_update_tooltip() {
	static const StringName signal_update_tooltip = "update_tooltip";
	return signal_update_tooltip;
}

String MenuSingleton::_get_state_name(State const& state) const {
	StateSet const& state_set = state.get_state_set();

	const String region_identifier = Utilities::std_to_godot_string(state_set.get_region().get_identifier());

	String name = tr(region_identifier);

	const bool named = name != region_identifier;
	const bool owned = state.get_owner() != nullptr;
	const bool split = state_set.get_state_count() > 1;

	if (!named) {
		// Capital province name
		// TODO - confirm capital is never null?
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
		return _get_country_adjective(*state.get_owner()) + " " + name;
	}

	// STATE/CAPITAL
	return name;
}

String MenuSingleton::_get_country_name(CountryInstance const& country) const {
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

String MenuSingleton::_get_country_adjective(CountryInstance const& country) const {
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

static String _make_modifier_effect_value(
	ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
) {
	String result;

	if (plus_for_non_negative && value >= 0) {
		result = "+";
	}

	static constexpr int32_t DECIMAL_PLACES = 2;

	using enum ModifierEffect::format_t;

	switch (format_effect.get_format()) {
	case PROPORTION_DECIMAL:
		value *= 100;
		[[fallthrough]];
	case PERCENTAGE_DECIMAL:
		result += Utilities::std_to_godot_string(value.to_string(DECIMAL_PLACES) + "%");
		break;
	case INT:

		// TODO - remove test code?!?!

		if (!value.is_integer() || value - fixed_point_t::parse(value.to_int64_t()) != fixed_point_t::_0()) {
			Logger::error(
				"Error formatting value for ModifierEffect \"", format_effect.get_identifier(), "\": ", value,
				" is not an integer! Replacing with ", value.to_int64_t()
			);
		}

		result += String::num_int64(value.to_int64_t());
		break;
	case RAW_DECIMAL: [[fallthrough]];
	default: // Use raw decimal as fallback format
		result += Utilities::std_to_godot_string(value.to_string(DECIMAL_PLACES));
		break;
	}

	return result;
}

static String _make_modifier_effect_value_coloured(
	ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
) {
	String result = GUILabel::get_colour_marker();

	if (value == 0) {
		result += "Y";
	} else if (format_effect.is_positive_good() == (value > 0)) {
		result += "G";
	} else {
		result += "R";
	}

	result += _make_modifier_effect_value(format_effect, value, plus_for_non_negative);

	static const String end_text = GUILabel::get_colour_marker() + String { "!" };
	result += end_text;

	return result;
}

String MenuSingleton::_make_modifier_line(
	std::string_view identifier, ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
) const {
	return tr(Utilities::std_to_godot_string(identifier)) + ": " +
		_make_modifier_effect_value_coloured(format_effect, value, plus_for_non_negative);
}

String MenuSingleton::_make_modifier_effects_tooltip(ModifierValue const& modifier) const {
	String result;

	for (auto const& [effect, value] : modifier.get_values()) {
		result += "\n" + _make_modifier_line(effect->get_identifier(), *effect, value, true);
	}

	return result;
}

template<ModifierEntryCallback ENTRY_CHECK_CALLBACK>
String MenuSingleton::_make_modifier_effect_contributions_tooltip(
	ModifierSum const& sum, ModifierEffect const& effect, ENTRY_CHECK_CALLBACK check_entry_callback
) const {
	String result;

	sum.for_each_contributing_modifier(
		effect,
		[this, &effect, &check_entry_callback, &result](
			ModifierSum::modifier_entry_t const& modifier_entry
		) -> void {
			if (check_entry_callback(modifier_entry)) {
				if (!result.is_empty()) {
					result += "\n";
				}

				result += _make_modifier_line(
					modifier_entry.modifier.get_identifier(),
					effect,
					modifier_entry.get_modifier_effect_value(effect),
					true
				);
			}
		}
	);

	return result;
}

template<ModifierEntryCallback ENTRY_CHECK_CALLBACK>
String MenuSingleton::_make_modifier_effect_contributions_tooltip_nullcheck(
	ModifierSum const& sum, ModifierEffect const* effect, ENTRY_CHECK_CALLBACK check_entry_callback
) const {
	return effect != nullptr ? _make_modifier_effect_contributions_tooltip(sum, *effect, check_entry_callback) : String {};
}

String MenuSingleton::_make_rules_tooltip(RuleSet const& rules) const {
	if (rules.empty()) {
		return {};
	}

	static const StringName yes_key = "YES";
	static const StringName no_key = "NO";

	static const String start_text = ": " + GUILabel::get_colour_marker();
	static const String end_text = GUILabel::get_colour_marker() + String { "!" };

	const String enabled_text = start_text + String { "G" } + tr(yes_key) + end_text;
	const String disabled_text = start_text + String { "R" } + tr(no_key) + end_text;

	String result;

	for (auto const& [rule_group, rule_map] : rules.get_rule_groups()) {
		for (auto const& [rule, enabled] : rule_map) {
			result += "\n" + tr(Utilities::std_to_godot_string(rule->get_localisation_key()))
				+ (enabled ? enabled_text : disabled_text);
		}
	}

	return result;
}

String MenuSingleton::_make_mobilisation_impact_tooltip() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	CountryInstance const* country = game_singleton->get_viewed_country();

	if (country == nullptr) {
		return {};
	}

	IssueManager const& issue_manager = game_singleton->get_definition_manager().get_politics_manager().get_issue_manager();

	static const StringName mobilisation_impact_tooltip_localisation_key = "MOBILIZATION_IMPACT_LIMIT_DESC";
	static const String mobilisation_impact_tooltip_replace_impact_key = "$IMPACT$";
	static const String mobilisation_impact_tooltip_replace_policy_key = "$POLICY$";
	static const String mobilisation_impact_tooltip_replace_units_key = "$UNITS$";

	static const StringName mobilisation_impact_tooltip2_localisation_key = "MOBILIZATION_IMPACT_LIMIT_DESC2";
	static const String mobilisation_impact_tooltip2_replace_curr_key = "$CURR$";
	static const String mobilisation_impact_tooltip2_replace_impact_key = "$IMPACT$";

	static const StringName no_issue = "noIssue";

	IssueGroup const* war_policy_issue_group = issue_manager.get_issue_group_by_identifier("war_policy");
	Issue const* war_policy_issue =
		war_policy_issue_group != nullptr ? country->get_ruling_party()->get_policies()[*war_policy_issue_group] : nullptr;

	const String impact_string = Utilities::std_to_godot_string((country->get_mobilisation_impact() * 100).to_string(1) + "%");

	return tr(
		mobilisation_impact_tooltip_localisation_key
	).replace(
		mobilisation_impact_tooltip_replace_impact_key, impact_string
	).replace(
		mobilisation_impact_tooltip_replace_policy_key, tr(
			war_policy_issue != nullptr
				? StringName { Utilities::std_to_godot_string(war_policy_issue->get_identifier()) }
				: no_issue
		)
	).replace(
		mobilisation_impact_tooltip_replace_units_key,
		String::num_uint64(country->get_mobilisation_max_regiment_count())
	) + "\n" + tr(
		mobilisation_impact_tooltip2_localisation_key
	).replace(
		mobilisation_impact_tooltip2_replace_curr_key, String::num_uint64(country->get_regiment_count())
	).replace(
		mobilisation_impact_tooltip2_replace_impact_key, impact_string
	);
}

void MenuSingleton::_bind_methods() {
	OV_BIND_SMETHOD(get_tooltip_separator);
	OV_BIND_METHOD(MenuSingleton::get_country_name_from_identifier, { "country_identifier" });
	OV_BIND_METHOD(MenuSingleton::get_country_adjective_from_identifier, { "country_identifier" });

	/* TOOLTIP */
	OV_BIND_METHOD(MenuSingleton::show_tooltip, { "text", "substitution_dict", "position" });
	OV_BIND_METHOD(MenuSingleton::show_control_tooltip, { "text", "substitution_dict", "control" });
	OV_BIND_METHOD(MenuSingleton::hide_tooltip);

	ADD_SIGNAL(MethodInfo(
		_signal_update_tooltip(), PropertyInfo(Variant::STRING, "text"),
		PropertyInfo(Variant::DICTIONARY, "substitution_dict"), PropertyInfo(Variant::VECTOR2, "position")
	));

	/* PROVINCE OVERVIEW PANEL */
	OV_BIND_METHOD(MenuSingleton::get_province_info_from_index, { "index" });
	OV_BIND_METHOD(MenuSingleton::get_province_building_count);
	OV_BIND_METHOD(MenuSingleton::get_province_building_identifier, { "building_index" });
	OV_BIND_METHOD(MenuSingleton::expand_selected_province_building, { "building_index" });
	OV_BIND_METHOD(MenuSingleton::get_slave_pop_icon_index);
	OV_BIND_METHOD(MenuSingleton::get_administrative_pop_icon_index);
	OV_BIND_METHOD(MenuSingleton::get_rgo_owner_pop_icon_index);

	/* TOPBAR */
	OV_BIND_METHOD(MenuSingleton::get_topbar_info);

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

	BIND_ENUM_CONSTANT(LIST_ENTRY_NONE);
	BIND_ENUM_CONSTANT(LIST_ENTRY_COUNTRY);
	BIND_ENUM_CONSTANT(LIST_ENTRY_STATE);
	BIND_ENUM_CONSTANT(LIST_ENTRY_PROVINCE);

	BIND_ENUM_CONSTANT(SORT_NONE);
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

	/* MILITARY MENU */
	OV_BIND_METHOD(MenuSingleton::get_military_menu_info, { "leader_sort_key" });

	BIND_ENUM_CONSTANT(LEADER_SORT_NONE);
	BIND_ENUM_CONSTANT(LEADER_SORT_PRESTIGE);
	BIND_ENUM_CONSTANT(LEADER_SORT_TYPE);
	BIND_ENUM_CONSTANT(LEADER_SORT_NAME);
	BIND_ENUM_CONSTANT(LEADER_SORT_ASSIGNMENT);
	BIND_ENUM_CONSTANT(MAX_LEADER_SORT_KEY);

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

String MenuSingleton::get_tooltip_separator() {
	static const String tooltip_separator = "\n" + String { "-" }.repeat(14) + "\n";
	return tooltip_separator;
}

String MenuSingleton::get_country_name_from_identifier(String const& country_identifier) const {
	if (country_identifier.is_empty()) {
		return {};
	}

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	CountryInstance const* country = instance_manager->get_country_instance_manager().get_country_instance_by_identifier(
		Utilities::godot_to_std_string(country_identifier)
	);
	ERR_FAIL_NULL_V(country, {});

	return _get_country_name(*country);
}

String MenuSingleton::get_country_adjective_from_identifier(String const& country_identifier) const {
	if (country_identifier.is_empty()) {
		return {};
	}

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	CountryInstance const* country = instance_manager->get_country_instance_manager().get_country_instance_by_identifier(
		Utilities::godot_to_std_string(country_identifier)
	);
	ERR_FAIL_NULL_V(country, {});

	return _get_country_adjective(*country);
}

/* TOOLTIP */

void MenuSingleton::show_tooltip(String const& text, Dictionary const& substitution_dict, Vector2 const& position) {
	emit_signal(_signal_update_tooltip(), text, substitution_dict, position);
}

void MenuSingleton::show_control_tooltip(String const& text, Dictionary const& substitution_dict, Control const* control) {
	ERR_FAIL_NULL(control);

	using namespace OpenVic::Utilities::literals;
	static const Vector2 offset { 0.0_real, 64.0_real };

	show_tooltip(text, substitution_dict, control->get_global_position() + offset);
}

void MenuSingleton::hide_tooltip() {
	show_tooltip({}, {}, {});
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
	static const StringName province_info_rgo_total_employees_key = "rgo_total_employees";
	static const StringName province_info_rgo_employment_percentage_key = "rgo_employment_percentage";
	static const StringName province_info_rgo_employment_tooltip_key = "rgo_employment_tooltip";
	static const StringName province_info_rgo_output_quantity_yesterday_key = "rgo_output_quantity_yesterday";
	static const StringName province_info_rgo_revenue_yesterday_key = "rgo_revenue_yesterday";
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
		ret[province_info_state_key] = _get_state_name(*state);
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

	ResourceGatheringOperation const& rgo = province->get_rgo();
	ret[province_info_rgo_output_quantity_yesterday_key] = rgo.get_output_quantity_yesterday().to_float();
	ret[province_info_rgo_revenue_yesterday_key] = rgo.get_revenue_yesterday().to_float();
	ret[province_info_rgo_total_employees_key] = rgo.get_total_employees_count_cache();
	const pop_size_t max_employee_count = rgo.get_max_employee_count_cache();
	if (max_employee_count == 0) {
		ret[province_info_rgo_employment_percentage_key] = 100.0f;
	} else {
		ret[province_info_rgo_employment_percentage_key] =
			(rgo.get_total_employees_count_cache() * fixed_point_t::_100() / max_employee_count).to_float_rounded();
	}

	if (rgo.is_valid()) {
		String amount_of_employees_by_pop_type;
		for (auto const& [pop_type, employees_of_type] : rgo.get_employee_count_per_type_cache()) {
			if (employees_of_type > 0) {
				amount_of_employees_by_pop_type +=
					"  -" + GUILabel::get_colour_marker() + "Y" +
					tr(Utilities::std_to_godot_string(pop_type.get_identifier())) + GUILabel::get_colour_marker() + "!:" +
					String::num_int64(employees_of_type) + "\n";
			}
		}

		ProductionType const& production_type = *rgo.get_production_type_nullable();

		String contributing_modifier_effects;

		{
			ModifierEffectCache const& modifier_effect_cache =
				game_singleton->get_definition_manager().get_modifier_manager().get_modifier_effect_cache();

			ModifierEffect const* rgo_size_local;
			ModifierEffect const* rgo_size_global;

			if (production_type.is_farm()) {
				rgo_size_local = modifier_effect_cache.get_farm_rgo_size_local();
				rgo_size_global = modifier_effect_cache.get_farm_rgo_size_global();
			} else {
				rgo_size_local = modifier_effect_cache.get_mine_rgo_size_local();
				rgo_size_global = modifier_effect_cache.get_mine_rgo_size_global();
			}

			if (rgo_size_local != nullptr) {
				if (province->get_terrain_type() != nullptr) {
					const fixed_point_t from_terrain = province->get_terrain_type()->get_effect(*rgo_size_local);

					if (from_terrain != fixed_point_t::_0()) {
						contributing_modifier_effects = tr(
							Utilities::std_to_godot_string(province->get_terrain_type()->get_identifier())
						) + ": " + _make_modifier_effect_value_coloured(*rgo_size_local, from_terrain, false) + "\n";
					}
				}

				const fixed_point_t from_province = province->get_modifier_effect_value(*rgo_size_local);

				if (from_province != fixed_point_t::_0()) {
					static const StringName rgo_size_localisation_key = "RGO_SIZE";

					contributing_modifier_effects += tr(rgo_size_localisation_key) + ": " +
						_make_modifier_effect_value_coloured(*rgo_size_local, from_province, false) + "\n";
				}
			}

			const fixed_point_t from_technology = province->get_modifier_effect_value_nullcheck(rgo_size_global) +
				province->get_modifier_effect_value_nullcheck(
					modifier_effect_cache.get_good_effects()[production_type.get_output_good()].get_rgo_size()
				);

			if (from_technology != fixed_point_t::_0()) {
				static const StringName from_technology_localisation_key = "employ_from_tech";

				contributing_modifier_effects += tr(from_technology_localisation_key) +
					_make_modifier_effect_value_coloured(*rgo_size_global, from_technology, false);
			}
		}

		static const StringName employment_localisation_key = "PROVINCEVIEW_EMPLOYMENT";
		static const String value_replace_key = "$VALUE$";
		static const StringName employee_count_localisation_key = "PRODUCTION_FACTORY_EMPLOYEECOUNT_TOOLTIP2";
		static const String employee_replace_key = "$EMPLOYEES$";
		static const String employee_max_replace_key = "$EMPLOYEE_MAX$";
		static const StringName rgo_workforce_localisation_key = "BASE_RGO_SIZE";
		static const StringName province_size_localisation_key = "FROM_PROV_SIZE";

		ret[province_info_rgo_employment_tooltip_key] =
			tr(employment_localisation_key).replace(value_replace_key, {}) + get_tooltip_separator() +
			tr(employee_count_localisation_key).replace(
				employee_replace_key, String::num_int64(rgo.get_total_employees_count_cache())
			).replace(
				employee_max_replace_key, String::num_int64(rgo.get_max_employee_count_cache())
			) + "\n" + amount_of_employees_by_pop_type + tr(rgo_workforce_localisation_key) +
			String::num_int64(production_type.get_base_workforce_size()) + "\n" +
			contributing_modifier_effects + "\n" + tr(province_size_localisation_key) + GUILabel::get_colour_marker() + "G" +
			String::num_int64(static_cast<int32_t>(rgo.get_size_multiplier())); // TODO - remove cast once variable is an int32_t
	}

	GoodDefinition const* const rgo_good = province->get_rgo_good();
	if (rgo_good != nullptr) {
		ret[province_info_rgo_name_key] = Utilities::std_to_godot_string(rgo_good->get_identifier());
		ret[province_info_rgo_icon_key] = static_cast<int32_t>(rgo_good->get_index());
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

/* TOPBAR */

Dictionary MenuSingleton::get_topbar_info() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	CountryInstance const* country = game_singleton->get_viewed_country();
	if (country == nullptr) {
		return {};
	}

	Dictionary ret;

	// Country / Ranking
	static const StringName country_key = "country";
	static const StringName country_status_key = "country_status";
	static const StringName total_rank_key = "total_rank";

	ret[country_key] = Utilities::std_to_godot_string(country->get_identifier());
	ret[country_status_key] = static_cast<int32_t>(country->get_country_status());
	ret[total_rank_key] = static_cast<uint64_t>(country->get_total_rank());

	static const StringName prestige_key = "prestige";
	static const StringName prestige_rank_key = "prestige_rank";
	static const StringName prestige_tooltip_key = "prestige_tooltip";

	ret[prestige_key] = country->get_prestige().to_int32_t();
	ret[prestige_rank_key] = static_cast<uint64_t>(country->get_prestige_rank());
	ret[prestige_tooltip_key] = String {}; // TODO - list prestige sources (e.g. power status)

	static const StringName industrial_power_key = "industrial_power";
	static const StringName industrial_rank_key = "industrial_rank";
	static const StringName industrial_power_tooltip_key = "industrial_power_tooltip";

	ret[industrial_power_key] = country->get_industrial_power().to_int32_t();
	ret[industrial_rank_key] = static_cast<uint64_t>(country->get_industrial_rank());
	{
		String industrial_power_tooltip;

		// Pair: State name / Power
		std::vector<std::pair<String, fixed_point_t>> industrial_power_states;
		for (auto const& [state, power] : country->get_industrial_power_from_states()) {
			industrial_power_states.emplace_back(_get_state_name(*state), power);
		}
		std::sort(
			industrial_power_states.begin(), industrial_power_states.end(),
			[](auto const& a, auto const& b) -> bool {
				// Sort by greatest power, then by state name alphabetically
				return a.second != b.second ? a.second > b.second : a.first < b.first;
			}
		);
		for (auto const& [state_name, power] : industrial_power_states) {
			industrial_power_tooltip += "\n" + state_name + ": " + GUILabel::get_colour_marker() + "Y"
				+ GUINode::float_to_string_dp(power, 3) + GUILabel::get_colour_marker() + "!";
		}

		// Tuple: Country identifier / Country name / Power
		std::vector<std::tuple<String, String, fixed_point_t>> industrial_power_from_investments;
		for (auto const& [country, power] : country->get_industrial_power_from_investments()) {
			industrial_power_from_investments.emplace_back(
				Utilities::std_to_godot_string(country->get_identifier()), _get_country_name(*country), power
			);
		}
		std::sort(
			industrial_power_from_investments.begin(), industrial_power_from_investments.end(),
			[](auto const& a, auto const& b) -> bool {
				// Sort by greatest power, then by country name alphabetically
				return std::get<2>(a) != std::get<2>(b) ? std::get<2>(a) > std::get<2>(b) : std::get<1>(a) < std::get<1>(b);
			}
		);
		for (auto const& [country_identifier, country_name, power] : industrial_power_from_investments) {
			industrial_power_tooltip += "\n" + GUILabel::get_flag_marker() + country_identifier + country_name + ": "
				+ GUILabel::get_colour_marker() + "Y" + GUINode::float_to_string_dp(power, 3) + GUILabel::get_colour_marker()
				+ "!";
		}

		ret[industrial_power_tooltip_key] = std::move(industrial_power_tooltip);
	}

	static const StringName military_power_key = "military_power";
	static const StringName military_rank_key = "military_rank";
	static const StringName military_power_tooltip_key = "military_power_tooltip";

	ret[military_power_key] = country->get_military_power().to_int32_t();
	ret[military_rank_key] = static_cast<uint64_t>(country->get_military_rank());
	{
		String military_power_tooltip;

		static const StringName military_power_from_land_key = "MIL_FROM_TROOPS";
		static const StringName military_power_from_sea_key = "MIL_FROM_CAP_SHIPS";
		static const StringName military_power_from_leaders_key = "MIL_FROM_LEADERS";

		for (auto const& [source, power] : {
			std::pair
			{ military_power_from_land_key, country->get_military_power_from_land() },
			{ military_power_from_sea_key, country->get_military_power_from_sea() },
			{ military_power_from_leaders_key, country->get_military_power_from_leaders() }
		}) {
			if (power != 0) {
				military_power_tooltip += "\n" + tr(source) + ": " + GUILabel::get_colour_marker() + "Y"
					+ GUINode::float_to_string_dp(power, 3) + GUILabel::get_colour_marker() + "!";
			}
		}

		ret[military_power_tooltip_key] = std::move(military_power_tooltip);
	}

	static const StringName colonial_power_available_key = "colonial_power_available";
	static const StringName colonial_power_max_key = "colonial_power_max";
	static const StringName colonial_power_tooltip_key = "colonial_power_tooltip";
	// TODO - colonial power info
	ret[colonial_power_available_key] = 0;
	ret[colonial_power_max_key] = 0;
	ret[colonial_power_tooltip_key] = String {};

	// Production

	// Budget

	// Technology

	// Politics

	// Population

	// Trade

	// Diplomacy

	// Military
	static const StringName regiment_count_key = "regiment_count";
	static const StringName max_supported_regiments_key = "max_supported_regiments";

	ret[regiment_count_key] = static_cast<uint64_t>(country->get_regiment_count());
	ret[max_supported_regiments_key] = static_cast<uint64_t>(country->get_max_supported_regiment_count());

	static const StringName is_mobilised_key = "is_mobilised";
	static const StringName mobilisation_regiments_key = "mobilisation_regiments";
	static const StringName mobilisation_impact_tooltip_key = "mobilisation_impact_tooltip";

	if (country->is_mobilised()) {
		ret[is_mobilised_key] = true;
	} else {
		ret[mobilisation_regiments_key] = static_cast<uint64_t>(country->get_mobilisation_potential_regiment_count());
		ret[mobilisation_impact_tooltip_key] = _make_mobilisation_impact_tooltip();
	}

	return ret;
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

	return Utilities::date_to_formatted_string(instance_manager->get_today(), true);
}

/* MILITARY MENU */

static Ref<Texture2D> _get_leader_picture(LeaderBase const& leader) {
	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, {});

	if (!leader.get_picture().empty()) {
		const Ref<ImageTexture> texture = asset_manager->get_leader_texture_std(leader.get_picture());

		if (texture.is_valid()) {
			return texture;
		}
	}

	return asset_manager->get_missing_leader_texture();
}

Dictionary MenuSingleton::make_leader_dict(LeaderBase const& leader) {
	const decltype(cached_leader_dicts)::const_iterator it = cached_leader_dicts.find(&leader);

	if (it != cached_leader_dicts.end()) {
		return it->second;
	}

	static const StringName military_info_leader_name_key = "leader_name";
	static const StringName military_info_leader_picture_key = "leader_picture";
	static const StringName military_info_leader_prestige_key = "leader_prestige";
	static const StringName military_info_leader_prestige_tooltip_key = "leader_prestige_tooltip";
	static const StringName military_info_leader_background_key = "leader_background";
	static const StringName military_info_leader_personality_key = "leader_personality";
	static const StringName military_info_leader_can_be_used_key = "leader_can_be_used";
	static const StringName military_info_leader_assignment_key = "leader_assignment";
	static const StringName military_info_leader_location_key = "leader_location";
	static const StringName military_info_leader_tooltip_key = "leader_tooltip";

	Dictionary leader_dict;
	String tooltip;
	ModifierValue modifier_value;

	// Picture
	leader_dict[military_info_leader_picture_key] = _get_leader_picture(leader);

	{
		// Branched (can be used, assignment, location, title)
		static const auto branched_section = []<UnitType::branch_t Branch>(
			LeaderBranched<Branch> const& leader, Dictionary& leader_dict
		) -> void {
			leader_dict[military_info_leader_can_be_used_key] = leader.get_can_be_used();

			UnitInstanceGroupBranched<Branch> const* group = leader.get_unit_instance_group();
			if (group != nullptr) {
				leader_dict[military_info_leader_assignment_key] = Utilities::std_to_godot_string(group->get_name());

				ProvinceInstance const* location = group->get_position();
				if (location != nullptr) {
					leader_dict[military_info_leader_location_key] =
						Utilities::std_to_godot_string(location->get_identifier());
				}
			}
		};

		using enum UnitType::branch_t;

		switch (leader.get_branch()) {
		case LAND: {
			static const StringName general_localisation_key = "MILITARY_GENERAL_TOOLTIP";
			tooltip = tr(general_localisation_key) + " ";

			branched_section(static_cast<General const&>(leader), leader_dict);
		} break;

		case NAVAL: {
			static const StringName admiral_localisation_key = "MILITARY_ADMIRAL_TOOLTIP";
			tooltip = tr(admiral_localisation_key) + " ";

			branched_section(static_cast<Admiral const&>(leader), leader_dict);
		} break;

		default:
			UtilityFunctions::push_error(
				"Invalid branch type \"", static_cast<int64_t>(leader.get_branch()), "\" for leader \"",
				Utilities::std_to_godot_string(leader.get_name()), "\""
			);
		}
	}

	{
		// Name
		String leader_name = Utilities::std_to_godot_string(leader.get_name());

		// Make yellow then revert back to default (white)
		static const String leader_name_prefix = GUILabel::get_colour_marker() + String { "Y" };
		static const String leader_name_suffix = GUILabel::get_colour_marker() + String { "!" };

		tooltip += leader_name_prefix + leader_name + leader_name_suffix;

		leader_dict[military_info_leader_name_key] = std::move(leader_name);
	}

	{
		// Prestige
		const fixed_point_t prestige = leader.get_prestige();
		fixed_point_t morale_bonus, organisation_bonus;

		GameSingleton const* game_singleton = GameSingleton::get_singleton();
		if (game_singleton != nullptr) {
			DefinitionManager const& definition_manager = game_singleton->get_definition_manager();
			modifier_value =
				definition_manager.get_military_manager().get_leader_trait_manager().get_leader_prestige_modifier() * prestige;

			ModifierEffectCache const& modifier_effect_cache =
				definition_manager.get_modifier_manager().get_modifier_effect_cache();

			morale_bonus = modifier_value.get_effect_nullcheck(modifier_effect_cache.get_morale_leader());
			organisation_bonus = modifier_value.get_effect_nullcheck(modifier_effect_cache.get_organisation());
		}

		static const StringName prestige_localisation_key = "PRESTIGE_SCORE";
		static const String value_replace_key = "$VAL$";

		String prestige_tooltip = tr(prestige_localisation_key).replace(
			value_replace_key, Utilities::float_to_string_dp(prestige * 100, 2) + "%"
		);

		tooltip += "\n" + prestige_tooltip;

		static const StringName morale_localisation_key = "PRESTIGE_MORALE_BONUS";
		static const StringName organisation_localisation_key = "PRESTIGE_MAX_ORG_BONUS";

		// Morale and organisation bonuses are always green with a + sign, matching the base game's behaviour
		static const String value_prefix = GUILabel::get_colour_marker() + String { "G+" };

		prestige_tooltip += "\n" + tr(morale_localisation_key).replace(
			value_replace_key, value_prefix + Utilities::float_to_string_dp(morale_bonus * 100, 2) + "%"
		) + "\n" + tr(organisation_localisation_key).replace(
			value_replace_key, value_prefix + Utilities::float_to_string_dp(organisation_bonus * 100, 2) + "%"
		);

		leader_dict[military_info_leader_prestige_key] = prestige.to_float();
		leader_dict[military_info_leader_prestige_tooltip_key] = std::move(prestige_tooltip);
	}

	{
		// Background
		String background;

		if (leader.get_background() != nullptr) {
			background = tr(Utilities::std_to_godot_string(leader.get_background()->get_identifier()));
			modifier_value += *leader.get_background();
		} else {
			static const StringName missing_background = "no_background";
			background = tr(missing_background);
		}

		static const StringName background_localisation_key = "MILITARY_BACKGROUND";
		static const String background_replace_key = "$NAME$";

		tooltip += "\n" + tr(background_localisation_key).replace(
			background_replace_key, background
		);

		leader_dict[military_info_leader_background_key] = std::move(background);
	}

	{
		// Personality
		String personality;

		if (leader.get_personality() != nullptr) {
			personality = tr(Utilities::std_to_godot_string(leader.get_personality()->get_identifier()));
			modifier_value += *leader.get_personality();
		} else {
			static const StringName missing_personality = "no_personality";
			personality = tr(missing_personality);
		}

		static const StringName personality_localisation_key = "MILITARY_PERSONALITY";
		static const String personality_replace_key = "$NAME$";

		tooltip += "\n" + tr(personality_localisation_key).replace(
			personality_replace_key, personality
		);

		leader_dict[military_info_leader_personality_key] = std::move(personality);
	}

	tooltip += _make_modifier_effects_tooltip(modifier_value);

	leader_dict[military_info_leader_tooltip_key] = std::move(tooltip);

	cached_leader_dicts.emplace(&leader, leader_dict);

	return leader_dict;
}

Dictionary MenuSingleton::make_army_dict(ArmyInstance const& army) {
	static const StringName military_info_army_leader_picture_key = "army_leader_picture";
	static const StringName military_info_army_leader_tooltip_key = "army_leader_tooltip";
	static const StringName military_info_army_name_key = "army_name";
	static const StringName military_info_army_location_key = "army_location";
	static const StringName military_info_army_regiment_count_key = "army_regiment_count";
	static const StringName military_info_army_men_count_key = "army_men_count";
	static const StringName military_info_army_max_men_count_key = "army_max_men_count";
	static const StringName military_info_army_morale_key = "army_morale";
	static const StringName military_info_army_moving_tooltip_key = "army_moving_tooltip";
	static const StringName military_info_army_digin_tooltip_key = "army_digin_tooltip";
	static const StringName military_info_army_combat_key = "army_combat";

	Dictionary army_dict;

	if (army.get_leader() != nullptr) {
		static const StringName military_info_leader_picture_key = "leader_picture";
		static const StringName military_info_leader_tooltip_key = "leader_tooltip";

		const Dictionary leader_dict = make_leader_dict(*army.get_leader());

		army_dict[military_info_army_leader_picture_key] = leader_dict.get(
			military_info_leader_picture_key, Ref<Texture2D> {}
		);
		army_dict[military_info_army_leader_tooltip_key] = leader_dict.get(military_info_leader_tooltip_key, String {});
	}

	army_dict[military_info_army_name_key] = Utilities::std_to_godot_string(army.get_name());
	if (army.get_position() != nullptr) {
		army_dict[military_info_army_location_key] = Utilities::std_to_godot_string(army.get_position()->get_identifier());
	}
	army_dict[military_info_army_regiment_count_key] = static_cast<uint64_t>(army.get_unit_count());

	// TODO - calculate (max) men and morale properly in ArmyInstance and RegimentInstances and set here
	army_dict[military_info_army_men_count_key] = static_cast<uint64_t>(army.get_unit_count() * 3000);
	army_dict[military_info_army_max_men_count_key] = static_cast<uint64_t>(army.get_unit_count() * 3000);
	army_dict[military_info_army_morale_key] = 1.0f;

	if (true /* army.is_moving() */) {
		static const StringName moving_localisation_key = "MILITARY_MOVING_TOOLTIP";
		static const String moving_location_replace_key = "$LOCATION$";
		static const String moving_date_replace_key = "$DATE$";

		ProvinceInstance const* destination = nullptr;
		Date arrival_date {};

		army_dict[military_info_army_moving_tooltip_key] = tr(moving_localisation_key).replace(
			moving_location_replace_key,
			tr(GUINode::format_province_name(
				destination != nullptr ? Utilities::std_to_godot_string(destination->get_identifier()) : String {}, false
			))
		).replace(moving_date_replace_key, Utilities::std_to_godot_string(arrival_date.to_string()));
	}

	if (true /* army.is_digging_in() */) {
		static const StringName digin_localisation_key = "MILITARY_DIGIN_TOOLTIP";
		static const String moving_days_replace_key = "$DAYS$";

		// TODO - get days spent digging in
		int64_t days_spent_digging_in = 4;

		army_dict[military_info_army_digin_tooltip_key] = tr(digin_localisation_key).replace(
			moving_days_replace_key, String::num_int64(days_spent_digging_in)
		);
	}

	if (true /* army.is_in_combat() */) {
		army_dict[military_info_army_combat_key] = true;
	}

	return army_dict;
}

Dictionary MenuSingleton::make_navy_dict(NavyInstance const& navy) {
	static const StringName military_info_navy_leader_picture_key = "navy_leader_picture";
	static const StringName military_info_navy_leader_tooltip_key = "navy_leader_tooltip";
	static const StringName military_info_navy_name_key = "navy_name";
	static const StringName military_info_navy_location_key = "navy_location";
	static const StringName military_info_navy_ship_count_key = "navy_ship_count";
	static const StringName military_info_navy_morale_key = "navy_morale";
	static const StringName military_info_navy_strength_key = "navy_strength";
	static const StringName military_info_navy_moving_tooltip_key = "navy_moving_tooltip";
	static const StringName military_info_navy_combat_key = "navy_combat";

	Dictionary navy_dict;

	if (navy.get_leader() != nullptr) {
		static const StringName military_info_leader_picture_key = "leader_picture";
		static const StringName military_info_leader_tooltip_key = "leader_tooltip";

		const Dictionary leader_dict = make_leader_dict(*navy.get_leader());

		navy_dict[military_info_navy_leader_picture_key] = leader_dict.get(
			military_info_leader_picture_key, Ref<Texture2D> {}
		);
		navy_dict[military_info_navy_leader_tooltip_key] = leader_dict.get(military_info_leader_tooltip_key, String {});
	}

	navy_dict[military_info_navy_name_key] = Utilities::std_to_godot_string(navy.get_name());
	if (navy.get_position() != nullptr) {
		navy_dict[military_info_navy_location_key] = Utilities::std_to_godot_string(navy.get_position()->get_identifier());
	}
	navy_dict[military_info_navy_ship_count_key] = static_cast<uint64_t>(navy.get_unit_count());

	// TODO - calculate strength and morale properly in NavyInstance from ShipInstances and set here
	navy_dict[military_info_navy_morale_key] = 1.0f;
	navy_dict[military_info_navy_strength_key] = 1.0f;

	if (true /* navy.is_moving() */) {
		static const StringName moving_localisation_key = "MILITARY_MOVING_TOOLTIP";
		static const String moving_location_replace_key = "$LOCATION$";
		static const String moving_date_replace_key = "$DATE$";

		ProvinceInstance const* destination = nullptr;
		Date arrival_date {};

		navy_dict[military_info_navy_moving_tooltip_key] = tr(moving_localisation_key).replace(
			moving_location_replace_key,
			tr(GUINode::format_province_name(
				destination != nullptr ? Utilities::std_to_godot_string(destination->get_identifier()) : String {}, false
			))
		).replace(moving_date_replace_key, Utilities::std_to_godot_string(arrival_date.to_string()));
	}

	if (true /* navy.is_in_combat() */) {
		navy_dict[military_info_navy_combat_key] = true;
	}

	return navy_dict;
}

Dictionary MenuSingleton::get_military_menu_info(LeaderSortKey leader_sort_key) {
	cached_leader_dicts.clear();

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	DefinitionManager const& definition_manager = game_singleton->get_definition_manager();
	ModifierEffectCache const& modifier_effect_cache = definition_manager.get_modifier_manager().get_modifier_effect_cache();
	IssueManager const& issue_manager = definition_manager.get_politics_manager().get_issue_manager();

	CountryInstance const* country = game_singleton->get_viewed_country();
	if (country == nullptr) {
		return {};
	}

	Dictionary ret;

	// Military stats
	static const StringName military_info_war_exhaustion_key = "war_exhaustion";
	static const StringName military_info_war_exhaustion_max_key = "war_exhaustion_max";
	static const StringName military_info_supply_consumption_key = "supply_consumption";
	static const StringName military_info_organisation_regain_key = "organisation_regain";
	static const StringName military_info_land_organisation_key = "land_organisation";
	static const StringName military_info_naval_organisation_key = "naval_organisation";
	static const StringName military_info_land_unit_start_experience_key = "land_unit_start_experience";
	static const StringName military_info_naval_unit_start_experience_key = "naval_unit_start_experience";
	static const StringName military_info_recruit_time_key = "recruit_time";
	static const StringName military_info_combat_width_key = "combat_width";
	static const StringName military_info_digin_cap_key = "digin_cap";
	static const StringName military_info_military_tactics_key = "military_tactics";

	ret[military_info_war_exhaustion_key] = country->get_war_exhaustion().to_float();
	ret[military_info_war_exhaustion_max_key] = country->get_war_exhaustion_max().to_float();
	ret[military_info_supply_consumption_key] = country->get_supply_consumption().to_float();
	ret[military_info_organisation_regain_key] = country->get_organisation_regain().to_float();
	ret[military_info_land_organisation_key] = country->get_land_organisation().to_float();
	ret[military_info_naval_organisation_key] = country->get_naval_organisation().to_float();
	ret[military_info_land_unit_start_experience_key] = country->get_land_unit_start_experience().to_float();
	ret[military_info_naval_unit_start_experience_key] = country->get_naval_unit_start_experience().to_float();
	ret[military_info_recruit_time_key] = country->get_recruit_time().to_float();
	ret[military_info_combat_width_key] = country->get_combat_width();
	ret[military_info_digin_cap_key] = country->get_digin_cap();
	ret[military_info_military_tactics_key] = country->get_military_tactics().to_float();

	// Mobilisation
	static const StringName military_info_is_mobilised_key = "is_mobilised";
	static const StringName military_info_mobilisation_progress_key = "mobilisation_progress";
	static const StringName military_info_mobilisation_size_key = "mobilisation_size";
	static const StringName military_info_mobilisation_size_tooltip_key = "mobilisation_size_tooltip";
	static const StringName military_info_mobilisation_impact_tooltip_key = "mobilisation_impact_tooltip";
	static const StringName military_info_mobilisation_economy_impact_key = "mobilisation_economy_impact";
	static const StringName military_info_mobilisation_economy_impact_tooltip_key = "mobilisation_economy_impact_tooltip";

	ret[military_info_is_mobilised_key] = country->is_mobilised();
	// ret[military_info_mobilisation_progress_key] = country->get_mobilisation_progress().to_float();
	ret[military_info_mobilisation_size_key] = static_cast<uint64_t>(country->get_mobilisation_potential_regiment_count());

	static const StringName mobilisation_size_tooltip_localisation_key = "MOB_SIZE_IRO";
	static const String mobilisation_size_tooltip_replace_value_key = "$VALUE$";

	ret[military_info_mobilisation_size_tooltip_key] = tr(mobilisation_size_tooltip_localisation_key).replace(
		mobilisation_size_tooltip_replace_value_key, Utilities::std_to_godot_string(
			(country->get_modifier_effect_value_nullcheck(modifier_effect_cache.get_mobilisation_size()) * 100).to_string(2)
		)
	) + "\n" + _make_modifier_effect_contributions_tooltip_nullcheck(
		country->get_modifier_sum(), modifier_effect_cache.get_mobilisation_size()
	);

	ret[military_info_mobilisation_impact_tooltip_key] = _make_mobilisation_impact_tooltip();

	ret[military_info_mobilisation_economy_impact_key] = country->get_mobilisation_economy_impact().to_float();

	{
		ModifierEffect const* mobilisation_economy_impact = modifier_effect_cache.get_mobilisation_economy_impact();
		fixed_point_t research_contribution;

		String mobilisation_economy_impact_tooltip = _make_modifier_effect_contributions_tooltip_nullcheck(
			country->get_modifier_sum(),
			mobilisation_economy_impact,
			[mobilisation_economy_impact, &research_contribution](
				ModifierSum::modifier_entry_t const& modifier_entry
			) -> bool {
				using enum Modifier::modifier_type_t;

				switch (modifier_entry.modifier.get_type()) {
				case TECHNOLOGY:
					[[fallthrough]];
				case INVENTION:
					research_contribution += modifier_entry.get_modifier_effect_value(*mobilisation_economy_impact);
					return false;

				default:
					return true;
				}
			}
		);

		if (research_contribution != fixed_point_t::_0()) {
			if (!mobilisation_economy_impact_tooltip.is_empty()) {
				mobilisation_economy_impact_tooltip = "\n" + mobilisation_economy_impact_tooltip;
			}

			static const StringName research_contribution_negative_key = "MOB_ECO_IMPACT";
			static const StringName research_contribution_positive_key = "MOB_ECO_PENALTY";
			static const String replace_value_key = "$VALUE$";

			mobilisation_economy_impact_tooltip = tr(
				research_contribution < fixed_point_t::_0()
					? research_contribution_negative_key
					: research_contribution_positive_key
			).replace(
				replace_value_key,
				_make_modifier_effect_value(*mobilisation_economy_impact, research_contribution.abs(), false)
			) + mobilisation_economy_impact_tooltip;
		}

		ret[military_info_mobilisation_economy_impact_tooltip_key] = mobilisation_economy_impact_tooltip;
	}

	// Leaders
	static const StringName military_info_general_count_key = "general_count";
	static const StringName military_info_admiral_count_key = "admiral_count";
	static const StringName military_info_create_leader_count_key = "create_leader_count";
	static const StringName military_info_auto_create_leaders_key = "auto_create_leaders";
	static const StringName military_info_auto_assign_leaders_key = "auto_assign_leaders";
	static const StringName military_info_leaders_list_key = "leaders_list";

	ret[military_info_general_count_key] = static_cast<uint64_t>(country->get_general_count());
	ret[military_info_admiral_count_key] = static_cast<uint64_t>(country->get_admiral_count());
	ret[military_info_create_leader_count_key] = static_cast<uint64_t>(country->get_create_leader_count());
	ret[military_info_auto_create_leaders_key] = country->get_auto_create_leaders();
	ret[military_info_auto_assign_leaders_key] = country->get_auto_assign_leaders();

	if (country->has_leaders()) {
		std::vector<LeaderBase const*> sorted_leaders;
		sorted_leaders.reserve(country->get_leader_count());
		for (General const& general : country->get_generals()) {
			sorted_leaders.push_back(&general);
		}
		for (Admiral const& admiral : country->get_admirals()) {
			sorted_leaders.push_back(&admiral);
		}

		switch (leader_sort_key) {
		case LEADER_SORT_PRESTIGE:
			std::sort(
				sorted_leaders.begin(), sorted_leaders.end(),
				[](LeaderBase const* a, LeaderBase const* b) -> bool {
					return a->get_prestige() > b->get_prestige();
				}
			);
			break;
		case LEADER_SORT_TYPE:
			std::sort(
				sorted_leaders.begin(), sorted_leaders.end(),
				[](LeaderBase const* a, LeaderBase const* b) -> bool {
					return a->get_branch() > b->get_branch();
				}
			);
			break;
		case LEADER_SORT_NAME:
			std::sort(
				sorted_leaders.begin(), sorted_leaders.end(),
				[](LeaderBase const* a, LeaderBase const* b) -> bool {
					return a->get_name() > b->get_name();
				}
			);
			break;
		case LEADER_SORT_ASSIGNMENT:
			//std::sort(
			//	sorted_leaders.begin(), sorted_leaders.end(),
			//	[](LeaderBase const* a, LeaderBase const* b) -> bool {
			//		// Do we have to convert to provinces or armies?
			//		return a->get_prestige() > b->get_prestige();
			//	}
			//);
			break;
		default: break;
		}

		TypedArray<Dictionary> leaders;
		if (leaders.resize(sorted_leaders.size()) == OK) {

			for (size_t index = 0; index < sorted_leaders.size(); ++index) {
				leaders[index] = make_leader_dict(*sorted_leaders[index]);
			}

			ret[military_info_leaders_list_key] = std::move(leaders);
		} else {
			UtilityFunctions::push_error(
				"Failed to resize military menu leaders array to the correct size (",
				static_cast<int64_t>(sorted_leaders.size()), ") for country \"",
				Utilities::std_to_godot_string(country->get_identifier()), "\""
			);
		}
	}

	// Armies and Navies
	static const StringName military_info_is_disarmed_key = "is_disarmed";
	static const StringName military_info_armies_key = "armies";
	static const StringName military_info_in_progress_brigades_key = "in_progress_brigades";
	static const StringName military_info_navies_key = "navies";
	static const StringName military_info_in_progress_ships_key = "in_progress_ships";

	ret[military_info_is_disarmed_key] = country->is_disarmed();

	if (country->has_armies()) {
		std::vector<ArmyInstance const*> sorted_armies;
		sorted_armies.reserve(country->get_army_count());
		for (ArmyInstance const* army : country->get_armies()) {
			sorted_armies.push_back(army);
		}

		// TODO - sort armies...

		TypedArray<Dictionary> armies;
		if (armies.resize(sorted_armies.size()) == OK) {

			for (size_t index = 0; index < sorted_armies.size(); ++index) {
				armies[index] = make_army_dict(*sorted_armies[index]);
			}

			ret[military_info_armies_key] = std::move(armies);
		} else {
			UtilityFunctions::push_error(
				"Failed to resize military menu armies array to the correct size (",
				static_cast<int64_t>(sorted_armies.size()), ") for country \"",
				Utilities::std_to_godot_string(country->get_identifier()), "\""
			);
		}
	}

	// ret[military_info_in_progress_brigades_key] = TypedArray<Dictionary> {};

	if (country->has_navies()) {
		std::vector<NavyInstance const*> sorted_navies;
		sorted_navies.reserve(country->get_navy_count());
		for (NavyInstance const* navy : country->get_navies()) {
			sorted_navies.push_back(navy);
		}

		// TODO - sort navies...

		TypedArray<Dictionary> navies;
		if (navies.resize(sorted_navies.size()) == OK) {

			for (size_t index = 0; index < sorted_navies.size(); ++index) {
				navies[index] = make_navy_dict(*sorted_navies[index]);
			}

			ret[military_info_navies_key] = std::move(navies);
		} else {
			UtilityFunctions::push_error(
				"Failed to resize military menu navies array to the correct size (",
				static_cast<int64_t>(sorted_navies.size()), ") for country \"",
				Utilities::std_to_godot_string(country->get_identifier()), "\""
			);
		}
	}

	// ret[military_info_in_progress_ships_key] = TypedArray<Dictionary> {};

	/*
	Each army/navy needs: leader icon, name, location, regiments, men (only for land), morale, strength, moving, digin, combat
	Each regiment/ship needs: build progress, unit_eta, name & icon of unit type, location
	*/

	return ret;
}

/* Find/Search Panel */

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
			String display_name = _get_state_name(state);
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
			String display_name = _get_country_name(country);
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
