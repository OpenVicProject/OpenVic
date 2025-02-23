#include "MenuSingleton.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include <openvic-simulation/modifier/Modifier.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>
#include <openvic-simulation/economy/GoodDefinition.hpp>
#include <openvic-simulation/research/Technology.hpp>
#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/modifier/ModifierEffect.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/utility/Utilities.hpp"
#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"

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

static constexpr int32_t DECIMAL_PLACES = 2;

String MenuSingleton::_make_modifier_effect_value(
	ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
) {
	String result;

	if (plus_for_non_negative && value >= 0) {
		result = "+";
	}

	using enum ModifierEffect::format_t;

	switch (format_effect.get_format()) {
	case PROPORTION_DECIMAL:
		value *= 100;
		[[fallthrough]];
	case PERCENTAGE_DECIMAL:
		result += Utilities::fixed_point_to_string_dp(value, DECIMAL_PLACES) + "%";
		break;
	case INT:
		// This won't produce a decimal point for actual whole numbers, but if the value has a fractional part it will be
		// displayed to 2 decimal places. This mirrors the base game, where effects which are meant to have integer values
		// will still display a fractional part if they are given one in the game defines.
		result += value.is_integer()
			? String::num_int64(value.to_int64_t())
			: Utilities::fixed_point_to_string_dp(value, DECIMAL_PLACES);
		break;
	case RAW_DECIMAL: [[fallthrough]];
	default: // Use raw decimal as fallback format
		result += Utilities::fixed_point_to_string_dp(value, DECIMAL_PLACES);
		break;
	}

	return result;
}

String MenuSingleton::_make_modifier_effect_value_coloured(
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

String MenuSingleton::_make_modifier_effect_tooltip(ModifierEffect const& effect, const fixed_point_t value) const {
	return tr(Utilities::std_to_godot_string(effect.get_localisation_key())) + ": " +
		_make_modifier_effect_value_coloured(effect, value, true);
}

String MenuSingleton::_make_modifier_effects_tooltip(ModifierValue const& modifier) const {
	String result;

	for (auto const& [effect, value] : modifier.get_values()) {
		if (value != fixed_point_t::_0()) {
			result += "\n" + _make_modifier_effect_tooltip(*effect, value);
		}
	}

	return result;
}

template<typename T>
requires std::same_as<T, CountryInstance> || std::same_as<T, ProvinceInstance>
String MenuSingleton::_make_modifier_effect_contributions_tooltip(
	T const& modifier_sum, ModifierEffect const& effect, fixed_point_t* tech_contributions,
	fixed_point_t* other_contributions, String const& prefix, String const& suffix
) const {
	String result;

	modifier_sum.for_each_contributing_modifier(
		effect,
		[this, &effect, tech_contributions, other_contributions, &prefix, &suffix, &result](
			modifier_entry_t const& modifier_entry, fixed_point_t value
		) -> void {
			using enum Modifier::modifier_type_t;

			// TODO - make sure we only include invention contributions from inside their "effect = { ... }" blocks,
			// as contributions from outside the blocks are treated as if they're from normal country modifiers and
			// displayed as "[invention name]: X.Y%" (both types of contributions can come from the same invention)
			if (tech_contributions != nullptr && (
				modifier_entry.modifier.get_type() == TECHNOLOGY || modifier_entry.modifier.get_type() == INVENTION
			)) {
				*tech_contributions += value;
				return;
			}

			if (other_contributions != nullptr) {
				*other_contributions += value;
			}

			result += prefix;

			if (effect.is_global()) {
				ProvinceInstance const* province = modifier_entry.get_source_province();
				if (province != nullptr) {
					result += tr(GUINode::format_province_name(Utilities::std_to_godot_string(province->get_identifier())));
					result += ": ";
				}
			}

			result += tr(Utilities::std_to_godot_string(modifier_entry.modifier.get_identifier()));
			result += ": ";
			result += _make_modifier_effect_value_coloured(effect, value, true);
			result += suffix;
		}
	);

	return result;
}

template String OpenVic::MenuSingleton::_make_modifier_effect_contributions_tooltip<CountryInstance>(
	CountryInstance const&, ModifierEffect const&, fixed_point_t*, fixed_point_t*, String const&, String const&
) const;
template String OpenVic::MenuSingleton::_make_modifier_effect_contributions_tooltip<ProvinceInstance>(
	ProvinceInstance const&, ModifierEffect const&, fixed_point_t*, fixed_point_t*, String const&, String const&
) const;

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
	CountryInstance const* country = PlayerSingleton::get_singleton()->get_player_country();

	if (country == nullptr) {
		return {};
	}

	IssueManager const& issue_manager =
		GameSingleton::get_singleton()->get_definition_manager().get_politics_manager().get_issue_manager();

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

	const String impact_string = Utilities::fixed_point_to_string_dp(country->get_mobilisation_impact() * 100, 1) + "%";

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
	OV_BIND_SMETHOD(get_tooltip_condition_met);
	OV_BIND_SMETHOD(get_tooltip_condition_unmet);

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
	OV_BIND_METHOD(MenuSingleton::get_slave_pop_icon_index);
	OV_BIND_METHOD(MenuSingleton::get_administrative_pop_icon_index);
	OV_BIND_METHOD(MenuSingleton::get_rgo_owner_pop_icon_index);

	/* TOPBAR */
	OV_BIND_METHOD(MenuSingleton::get_topbar_info);

	/* TIME/SPEED CONTROL PANEL */
	OV_BIND_METHOD(MenuSingleton::is_paused);
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

	/* TRADE MENU */
	OV_BIND_METHOD(MenuSingleton::get_trade_menu_good_categories_info);
	OV_BIND_METHOD(MenuSingleton::get_trade_menu_trade_details_info, { "trade_detail_good_index", "stockpile_cutoff_slider" });
	OV_BIND_METHOD(MenuSingleton::get_trade_menu_tables_info);
	OV_BIND_SMETHOD(calculate_trade_menu_stockpile_cutoff_amount, { "slider" });

	BIND_ENUM_CONSTANT(TRADE_SETTING_NONE);
	BIND_ENUM_CONSTANT(TRADE_SETTING_AUTOMATED);
	BIND_ENUM_CONSTANT(TRADE_SETTING_BUYING);
	BIND_ENUM_CONSTANT(TRADE_SETTING_SELLING);

	/* MILITARY MENU */
	OV_BIND_METHOD(MenuSingleton::get_military_menu_info, {
		"leader_sort_key", "sort_leaders_descending",
		"army_sort_key", "sort_armies_descending",
		"navy_sort_key", "sort_navies_descending"
	});

	BIND_ENUM_CONSTANT(LEADER_SORT_NONE);
	BIND_ENUM_CONSTANT(LEADER_SORT_PRESTIGE);
	BIND_ENUM_CONSTANT(LEADER_SORT_TYPE);
	BIND_ENUM_CONSTANT(LEADER_SORT_NAME);
	BIND_ENUM_CONSTANT(LEADER_SORT_ASSIGNMENT);
	BIND_ENUM_CONSTANT(MAX_LEADER_SORT_KEY);

	BIND_ENUM_CONSTANT(UNIT_GROUP_SORT_NONE);
	BIND_ENUM_CONSTANT(UNIT_GROUP_SORT_NAME);
	BIND_ENUM_CONSTANT(UNIT_GROUP_SORT_STRENGTH);
	BIND_ENUM_CONSTANT(MAX_UNIT_GROUP_SORT_KEY);

	/* Find/Search Panel */
	OV_BIND_METHOD(MenuSingleton::generate_search_cache);
	OV_BIND_METHOD(MenuSingleton::update_search_results, { "text" });
	OV_BIND_METHOD(MenuSingleton::get_search_result_rows, { "start", "count" });
	OV_BIND_METHOD(MenuSingleton::get_search_result_row_count);
	OV_BIND_METHOD(MenuSingleton::get_search_result_position, { "result_index" });

	ADD_SIGNAL(MethodInfo(_signal_search_cache_changed()));

	/* TECHNOLOGY MENU */
	OV_BIND_METHOD(MenuSingleton::get_technology_menu_defines);
	OV_BIND_METHOD(MenuSingleton::get_technology_menu_info);
	OV_BIND_METHOD(MenuSingleton::get_specific_technology_info, { "technology_identifier" });
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

String MenuSingleton::get_tooltip_condition_met() {
	static const String condition_met = String { "(" } + GUILabel::get_colour_marker() + String { "G*" } + GUILabel::get_colour_marker() + "W)";
	return condition_met;
}

String MenuSingleton::get_tooltip_condition_unmet() {
	static const String condition_unmet = String { "(" } + GUILabel::get_colour_marker() + String { "RX" } + GUILabel::get_colour_marker() + "W)";
	return condition_unmet;
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
			building_dict[building_info_start_date_key] = Utilities::date_to_string(building.get_start_date());
			building_dict[building_info_end_date_key] = Utilities::date_to_string(building.get_end_date());
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
	static const StringName province_info_terrain_type_tooltip_key = "terrain_type_tooltip";
	static const StringName province_info_life_rating_key = "life_rating";
	static const StringName province_info_controller_key = "controller";
	static const StringName province_info_controller_tooltip_key = "controller_tooltip";
	static const StringName province_info_rgo_icon_key = "rgo_icon";
	static const StringName province_info_rgo_production_tooltip_key = "rgo_production_tooltip";
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
		String terrain_type_string = Utilities::std_to_godot_string(terrain_type->get_identifier());

		static const StringName terrain_type_localisation_key = "PROVINCEVIEW_TERRAIN";
		static const String terrain_type_replace_key = "$TERRAIN$";
		static const StringName movement_cost_localisation_key = "TERRAIN_MOVEMENT_COST";
		static const String terrain_type_template_string = "%s" + get_tooltip_separator() + "%s" +
			GUILabel::get_colour_marker() + "Y%s" + GUILabel::get_colour_marker() + "!%s";

		ret[province_info_terrain_type_tooltip_key] = vformat(
			terrain_type_template_string,
			tr(terrain_type_localisation_key).replace(terrain_type_replace_key, tr(terrain_type_string)),
			tr(movement_cost_localisation_key),
			Utilities::fixed_point_to_string_dp(terrain_type->get_movement_cost(), 2),
			_make_modifier_effects_tooltip(*terrain_type)
		);

		ret[province_info_terrain_type_key] = std::move(terrain_type_string);
	}

	ret[province_info_life_rating_key] = province->get_life_rating();

	CountryInstance const* controller = province->get_controller();
	if (controller != nullptr) {
		ret[province_info_controller_key] = Utilities::std_to_godot_string(controller->get_identifier());

		static const StringName controller_localisation_key = "PV_CONTROLLER";
		static const String controller_template_string = "%s %s";
		ret[province_info_controller_tooltip_key] = vformat(
			controller_template_string, tr(controller_localisation_key), _get_country_name(*controller)
		);
	}

	ResourceGatheringOperation const& rgo = province->get_rgo();

	if (rgo.is_valid()) {
		ProductionType const& production_type = *rgo.get_production_type_nullable();
		GoodDefinition const& rgo_good = *province->get_rgo_good();

		ret[province_info_rgo_icon_key] = static_cast<int32_t>(rgo_good.get_index());

		ret[province_info_rgo_output_quantity_yesterday_key] = rgo.get_output_quantity_yesterday().to_float();
		ret[province_info_rgo_revenue_yesterday_key] = rgo.get_revenue_yesterday().to_float();
		ret[province_info_rgo_total_employees_key] = rgo.get_total_employees_count_cache();
		const pop_size_t max_employee_count = rgo.get_max_employee_count_cache();
		if (max_employee_count == 0) {
			ret[province_info_rgo_employment_percentage_key] = 100.0f;
		} else {
			ret[province_info_rgo_employment_percentage_key] =
				(rgo.get_total_employees_count_cache() * fixed_point_t::_100() / max_employee_count).to_float();
		}

		ModifierEffectCache const& modifier_effect_cache =
			game_singleton->get_definition_manager().get_modifier_manager().get_modifier_effect_cache();

		fixed_point_t output_from_workers = fixed_point_t::_1(), throughput_from_workers = fixed_point_t::_0(),
			output_multiplier = fixed_point_t::_1(), throughput_multiplier = fixed_point_t::_1();
		String size_string, output_string, throughput_string;

		static const String employee_effect_template_string = "\n  -%s: " + GUILabel::get_colour_marker() + "Y%s" +
			GUILabel::get_colour_marker() + "!: %s";

		using enum Job::effect_t;

		std::optional<Job> const& owner_job = production_type.get_owner();
		if (owner_job.has_value()) {
			PopType const& owner_pop_type = *owner_job->get_pop_type();
			State const* state = province->get_state();

			if (unlikely(state == nullptr)) {
				Logger::error(
					"Province \"", province->get_identifier(), "\" has no state, preventing calculation of state-wide "
					"population proportion of RGO owner pop type \"", owner_pop_type.get_identifier(), "\""
				);
			} else {
				const fixed_point_t effect_value = owner_job->get_effect_multiplier() *
					state->get_pop_type_distribution()[owner_pop_type] / state->get_total_population();

				static const StringName owners_localisation_key = "PRODUCTION_FACTOR_OWNER";

				switch (owner_job->get_effect_type()) {
				case OUTPUT:
					output_multiplier += effect_value;
					output_string += vformat(
						employee_effect_template_string,
						tr(owners_localisation_key),
						tr(Utilities::std_to_godot_string(owner_pop_type.get_identifier())),
						_make_modifier_effect_value_coloured(
							*modifier_effect_cache.get_rgo_output(), effect_value, true
						)
					);
					break;
				case THROUGHPUT:
					throughput_multiplier += effect_value;
					throughput_string += vformat(
						employee_effect_template_string,
						tr(owners_localisation_key),
						tr(Utilities::std_to_godot_string(owner_pop_type.get_identifier())),
						_make_modifier_effect_value_coloured(
							*modifier_effect_cache.get_rgo_throughput(), effect_value, true
						)
					);
					break;
				default:
					break;
				}
			}
		}

		String amount_of_employees_by_pop_type;

		for (auto const& [pop_type, employees_of_type] : rgo.get_employee_count_per_type_cache()) {
			if (employees_of_type <= 0) {
				continue;
			}

			static const String amount_of_employees_by_pop_type_template_string = "\n  -" + GUILabel::get_colour_marker() +
				"Y%s" + GUILabel::get_colour_marker() + "!:%d";

			amount_of_employees_by_pop_type += vformat(
				amount_of_employees_by_pop_type_template_string,
				tr(Utilities::std_to_godot_string(pop_type.get_identifier())),
				employees_of_type
			);

			for (Job const& job : production_type.get_jobs()) {
				if (job.get_pop_type() != &pop_type) {
					continue;
				}

				const fixed_point_t effect_multiplier = job.get_effect_multiplier();
				fixed_point_t relative_to_workforce = fixed_point_t::parse(employees_of_type) / max_employee_count;
				const fixed_point_t effect_value = effect_multiplier == fixed_point_t::_1()
					? relative_to_workforce
					: effect_multiplier * std::min(relative_to_workforce, job.get_amount());

				static const StringName workers_localisation_key = "PRODUCTION_FACTOR_WORKER";

				switch (job.get_effect_type()) {
					case OUTPUT:
						output_from_workers += effect_value;
						output_string += vformat(
							employee_effect_template_string,
							tr(workers_localisation_key),
							tr(Utilities::std_to_godot_string(pop_type.get_identifier())),
							_make_modifier_effect_value_coloured(
								*modifier_effect_cache.get_rgo_output(), effect_value, true
							)
						);
						break;
					case THROUGHPUT:
						throughput_from_workers += effect_value;
						throughput_string += vformat(
							employee_effect_template_string,
							tr(workers_localisation_key),
							tr(Utilities::std_to_godot_string(pop_type.get_identifier())),
							_make_modifier_effect_value_coloured(
								*modifier_effect_cache.get_rgo_throughput(), effect_value, true
							)
						);
						break;
					default:
						break;
				}
			}
		}

		{
			fixed_point_t output_from_tech, throughput_from_tech;
			String output_modifiers, throughput_modifiers;

			static const String modifier_effect_contributions_prefix = "\n  -";

			output_modifiers += _make_modifier_effect_contributions_tooltip(
				*province, *modifier_effect_cache.get_rgo_output(), &output_from_tech, &output_multiplier,
				modifier_effect_contributions_prefix
			);
			output_modifiers += _make_modifier_effect_contributions_tooltip(
				*province, *modifier_effect_cache.get_local_rgo_output(), &output_from_tech, &output_multiplier,
				modifier_effect_contributions_prefix
			);
			throughput_modifiers += _make_modifier_effect_contributions_tooltip(
				*province, *modifier_effect_cache.get_rgo_throughput(), &throughput_from_tech, &throughput_multiplier,
				modifier_effect_contributions_prefix
			);
			throughput_modifiers += _make_modifier_effect_contributions_tooltip(
				*province, *modifier_effect_cache.get_local_rgo_throughput(), &throughput_from_tech, &throughput_multiplier,
				modifier_effect_contributions_prefix
			);

			fixed_point_t size_from_terrain, size_from_province;

			if (production_type.get_is_farm_for_non_tech()) {
				output_modifiers += _make_modifier_effect_contributions_tooltip(
					*province, *modifier_effect_cache.get_farm_rgo_output_global(), &output_from_tech, &output_multiplier,
					modifier_effect_contributions_prefix
				);
				output_modifiers += _make_modifier_effect_contributions_tooltip(
					*province, *modifier_effect_cache.get_farm_rgo_output_local(), &output_from_tech, &output_multiplier,
					modifier_effect_contributions_prefix
				);

				if (terrain_type != nullptr) {
					size_from_terrain = terrain_type->get_effect(*modifier_effect_cache.get_farm_rgo_size_local());
				}
				size_from_province = province->get_modifier_effect_value(*modifier_effect_cache.get_farm_rgo_size_local());
			}

			if (production_type.get_is_mine_for_non_tech()) {
				output_modifiers += _make_modifier_effect_contributions_tooltip(
					*province, *modifier_effect_cache.get_mine_rgo_output_global(), &output_from_tech, &output_multiplier,
					modifier_effect_contributions_prefix
				);
				output_modifiers += _make_modifier_effect_contributions_tooltip(
					*province, *modifier_effect_cache.get_mine_rgo_output_local(), &output_from_tech, &output_multiplier,
					modifier_effect_contributions_prefix
				);

				if (terrain_type != nullptr) {
					size_from_terrain += terrain_type->get_effect(*modifier_effect_cache.get_mine_rgo_size_local());
				}
				size_from_province += province->get_modifier_effect_value(*modifier_effect_cache.get_mine_rgo_size_local());
			}

			static const String size_modifier_template_string = "%s: %s\n";

			if (size_from_terrain != fixed_point_t::_0()) {
				size_string = vformat(
					size_modifier_template_string,
					tr(Utilities::std_to_godot_string(province->get_terrain_type()->get_identifier())),
					_make_modifier_effect_value_coloured(
						*modifier_effect_cache.get_farm_rgo_size_local(), size_from_terrain, false
					)
				);
			}

			if (size_from_province != fixed_point_t::_0()) {
				static const StringName rgo_size_localisation_key = "RGO_SIZE";

				size_string += vformat(
					size_modifier_template_string,
					tr(rgo_size_localisation_key),
					_make_modifier_effect_value_coloured(
						*modifier_effect_cache.get_farm_rgo_size_local(), size_from_province, false
					)
				);
			}

			ModifierEffectCache::good_effects_t const& good_effects =
				modifier_effect_cache.get_good_effects()[production_type.get_output_good()];

			output_from_tech += province->get_modifier_effect_value(*good_effects.get_rgo_goods_output());
			throughput_from_tech += province->get_modifier_effect_value(*good_effects.get_rgo_goods_throughput());

			fixed_point_t size_from_tech;

			size_from_tech = province->get_modifier_effect_value(*good_effects.get_rgo_size());
			if (production_type.get_is_farm_for_tech()) {
				const fixed_point_t value =
					province->get_modifier_effect_value(*modifier_effect_cache.get_farm_rgo_throughput_and_output());
				output_from_tech += value;
				throughput_from_tech += value;

				size_from_tech += province->get_modifier_effect_value(*modifier_effect_cache.get_farm_rgo_size_global());
			}

			if (production_type.get_is_mine_for_tech()) {
				const fixed_point_t value =
					province->get_modifier_effect_value(*modifier_effect_cache.get_mine_rgo_throughput_and_output());
				output_from_tech += value;
				throughput_from_tech += value;

				size_from_tech += province->get_modifier_effect_value(*modifier_effect_cache.get_mine_rgo_size_global());
			}

			if (size_from_tech != fixed_point_t::_0()) {
				static const StringName from_technology_localisation_key = "employ_from_tech";

				size_string += tr(from_technology_localisation_key) + _make_modifier_effect_value_coloured(
					*modifier_effect_cache.get_farm_rgo_size_global(), size_from_tech, false
				);
			}

			static const String tech_modifier_template_string = modifier_effect_contributions_prefix + String { "%s: %s" };

			if (output_from_tech != fixed_point_t::_0()) {
				output_multiplier += output_from_tech;

				static const StringName rgo_output_tech_localisation_key = "RGO_OUTPUT_TECH";

				output_string += vformat(
					tech_modifier_template_string,
					tr(rgo_output_tech_localisation_key),
					_make_modifier_effect_value_coloured(
						*modifier_effect_cache.get_rgo_output(), output_from_tech, true
					)
				);
			}

			if (throughput_from_tech != fixed_point_t::_0()) {
				throughput_multiplier += throughput_from_tech;

				static const StringName rgo_throughput_tech_localisation_key = "RGO_THROUGHPUT_TECH";

				throughput_string += vformat(
					tech_modifier_template_string,
					tr(rgo_throughput_tech_localisation_key),
					_make_modifier_effect_value_coloured(
						*modifier_effect_cache.get_rgo_throughput(), throughput_from_tech, true
					)
				);
			}

			output_string += output_modifiers;
			throughput_string += throughput_modifiers;
		}

		static const StringName rgo_production_localisation_key = "PROVINCEVIEW_GOODSINCOME";
		static const String rgo_good_replace_key = "$GOODS$";
		static const String value_replace_key = "$VALUE$";
		static const StringName max_output_localisation_key = "PRODUCTION_OUTPUT_GOODS_TOOLTIP2";
		static const String curr_replace_key = "$CURR$";
		static const StringName output_explanation_localisation_key = "PRODUCTION_OUTPUT_EXPLANATION";
		static const StringName base_output_localisation_key = "PRODUCTION_BASE_OUTPUT_GOODS_TOOLTIP";
		static const String base_replace_key = "$BASE$";
		static const StringName output_efficiency_localisation_key = "PRODUCTION_OUTPUT_EFFICIENCY_TOOLTIP";
		static const StringName base_localisation_key = "PRODUCTION_BASE_OUTPUT";
		static const StringName throughput_efficiency_localisation_key = "PRODUCTION_THROUGHPUT_EFFICIENCY_TOOLTIP";
		static const String rgo_production_template_string = "%s\n%s%s" + get_tooltip_separator() + "%s%s%s\n%s " +
			GUILabel::get_colour_marker() + "G100%%" + GUILabel::get_colour_marker() + "!%s\n\n%s%s%s";

		const fixed_point_t throughput_efficiency = throughput_from_workers * throughput_multiplier;
		const fixed_point_t output_efficiency = output_from_workers * output_multiplier;
		const fixed_point_t base_output = production_type.get_base_output_quantity();
		const fixed_point_t max_output = base_output * throughput_efficiency * output_efficiency;

		ret[province_info_rgo_production_tooltip_key] = vformat(
			rgo_production_template_string,
			tr(rgo_production_localisation_key).replace(
				rgo_good_replace_key, tr(Utilities::std_to_godot_string(rgo_good.get_identifier()))
			).replace(value_replace_key, Utilities::fixed_point_to_string_dp(rgo.get_revenue_yesterday(), 3)),
			tr(max_output_localisation_key).replace(curr_replace_key, Utilities::fixed_point_to_string_dp(max_output, 2)),
			tr(output_explanation_localisation_key),
			tr(base_output_localisation_key).replace(base_replace_key, Utilities::fixed_point_to_string_dp(base_output, 2)),
			tr(output_efficiency_localisation_key),
			_make_modifier_effect_value_coloured(*modifier_effect_cache.get_rgo_output(), output_efficiency, false),
			tr(base_localisation_key),
			output_string,
			tr(throughput_efficiency_localisation_key),
			_make_modifier_effect_value_coloured(*modifier_effect_cache.get_rgo_throughput(), throughput_efficiency, false),
			throughput_string
		);

		static const StringName employment_localisation_key = "PROVINCEVIEW_EMPLOYMENT";
		static const StringName employee_count_localisation_key = "PRODUCTION_FACTORY_EMPLOYEECOUNT_TOOLTIP2";
		static const String employee_replace_key = "$EMPLOYEES$";
		static const String employee_max_replace_key = "$EMPLOYEE_MAX$";
		static const StringName rgo_workforce_localisation_key = "BASE_RGO_SIZE";
		static const StringName province_size_localisation_key = "FROM_PROV_SIZE";
		static const String rgo_employment_template_string = "%s" + get_tooltip_separator() + "%s%s\n%s%d\n%s\n%s" +
			GUILabel::get_colour_marker() + "G%d";

		ret[province_info_rgo_employment_tooltip_key] = vformat(
			rgo_employment_template_string,
			tr(employment_localisation_key).replace(value_replace_key, {}),
			tr(employee_count_localisation_key).replace(
				employee_replace_key, String::num_int64(rgo.get_total_employees_count_cache())
			).replace(
				employee_max_replace_key, String::num_int64(rgo.get_max_employee_count_cache())
			),
			amount_of_employees_by_pop_type,
			tr(rgo_workforce_localisation_key),
			production_type.get_base_workforce_size(),
			size_string,
			tr(province_size_localisation_key),
			static_cast<int32_t>(rgo.get_size_multiplier()) // TODO - remove cast once variable is an int32_t
		);
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
	CountryInstance const* country = PlayerSingleton::get_singleton()->get_player_country();
	if (country == nullptr) {
		return {};
	}

	DefinitionManager const& definition_manager = GameSingleton::get_singleton()->get_definition_manager();
	ModifierEffectCache const& modifier_effect_cache = definition_manager.get_modifier_manager().get_modifier_effect_cache();

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
			static const String state_power_template_string =
				"\n%s: " + GUILabel::get_colour_marker() + "Y%s" + GUILabel::get_colour_marker() + "!";

			industrial_power_tooltip += vformat(
				state_power_template_string,
				state_name,
				Utilities::fixed_point_to_string_dp(power, 3)
			);
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
			static const String investment_power_template_string = "\n" + GUILabel::get_flag_marker() + "%s %s: " +
				GUILabel::get_colour_marker() + "Y%s" + GUILabel::get_colour_marker() + "!";

			industrial_power_tooltip += vformat(
				investment_power_template_string,
				country_identifier,
				country_name,
				Utilities::fixed_point_to_string_dp(power, 3)
			);
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
					+ Utilities::fixed_point_to_string_dp(power, 3) + GUILabel::get_colour_marker() + "!";
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
	{
		static const StringName research_key = "research";
		static const StringName research_tooltip_key = "research_tooltip";
		static const StringName research_progress_key = "research_progress";
		static const StringName literacy_key = "literacy";
		static const StringName literacy_change_key = "literacy_change";
		static const StringName research_points_key = "research_points";
		static const StringName research_points_tooltip_key = "research_points_tooltip";

		Technology const* current_research = country->get_current_research();
		if (current_research != nullptr) {
			static const StringName research_localisation_key = "TECHNOLOGYVIEW_RESEARCH_TOOLTIP";
			static const String tech_replace_key = "$TECH$";
			static const String date_replace_key = "$DATE$";
			static const StringName research_invested_localisation_key = "TECHNOLOGYVIEW_RESEARCH_INVESTED_TOOLTIP";
			static const String invested_replace_key = "$INVESTED$";
			static const String cost_replace_key = "$COST$";

			String current_tech_localised = tr(Utilities::std_to_godot_string(current_research->get_identifier()));

			ret[research_tooltip_key] = tr(research_localisation_key).replace(
				tech_replace_key, current_tech_localised
			).replace(
				date_replace_key, Utilities::date_to_formatted_string(country->get_expected_research_completion_date(), false)
			) + "\n" + tr(research_invested_localisation_key).replace(
				invested_replace_key, String::num_uint64(country->get_invested_research_points().to_int64_t())
			).replace(cost_replace_key, String::num_uint64(country->get_current_research_cost().to_int64_t()));

			ret[research_key] = std::move(current_tech_localised);

			ret[research_progress_key] = country->get_research_progress().to_float();
		} else if (country->is_civilised()) {
			static const StringName civilised_no_research_localisation_key = "TB_TECH_NO_CURRENT";
			static const StringName civilised_no_research_tooltip_localisation_key = "TECHNOLOGYVIEW_NO_RESEARCH_TOOLTIP";

			ret[research_key] = tr(civilised_no_research_localisation_key);
			ret[research_tooltip_key] = tr(civilised_no_research_tooltip_localisation_key);
		} else {
			static const String red_prefix_text = GUILabel::get_colour_marker() + String { "R" };
			static const StringName uncivilised_no_research_localisation_key = "unciv_nation";
			static const StringName uncivilised_no_research_tooltip_localisation_key =
				"TECHNOLOGYVIEW_NO_RESEARCH_UNCIV_TOOLTIP";

			ret[research_key] = red_prefix_text + tr(uncivilised_no_research_localisation_key);
			ret[research_tooltip_key] = tr(uncivilised_no_research_tooltip_localisation_key);
		}

		ret[literacy_key] = country->get_national_literacy().to_float();
		// TODO - set monthly literacy change (test for precision issues)
		ret[literacy_change_key] = 0.0f;

		ret[research_points_key] = country->get_daily_research_points().to_float();

		String research_points_tooltip;

		fixed_point_t daily_base_research_points;

		static const String value_replace_key = "$VALUE$";

		for (auto const& [pop_type, research_points] : country->get_research_points_from_pop_types()) {
			static const StringName pop_type_research_localisation_key = "TECH_DAILY_RESEARCHPOINTS_TOOLTIP";
			static const String pop_type_replace_key = "$POPTYPE$";
			static const String fraction_replace_key = "$FRACTION$";
			static const String optimal_replace_key = "$OPTIMAL$";

			daily_base_research_points += research_points;

			research_points_tooltip += tr(pop_type_research_localisation_key).replace(
				pop_type_replace_key, tr(Utilities::std_to_godot_string(pop_type->get_identifier()))
			).replace(
				value_replace_key, Utilities::fixed_point_to_string_dp(research_points, 2)
			).replace(
				fraction_replace_key, Utilities::fixed_point_to_string_dp(
					100 * country->get_pop_type_proportion(*pop_type) / country->get_total_population(), 2
				)
			).replace(
				optimal_replace_key, Utilities::fixed_point_to_string_dp(100 * pop_type->get_research_leadership_optimum(), 2)
			) + "\n";
		}

		// Empty prefix, "\n" suffix, fitting with the potential trailing "\n" from the pop type contributions and the upcoming
		// guaranteed daily base research points line. All contributions are added to daily_base_research_points.
		research_points_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_research_points(), nullptr, &daily_base_research_points, {}, "\n"
		);

		// The daily base research points line is guaranteed to be present, but those directly above and below it aren't,
		// so this line has no newline characters of its own. Instead, potential lines above finish with newlines and
		// potential (and some guaranteed) lines below start with them.
		static const StringName daily_base_research_points_localisation_key = "TECH_DAILY_RESEARCHPOINTS_BASE_TOOLTIP";
		research_points_tooltip += tr(daily_base_research_points_localisation_key).replace(
			value_replace_key, Utilities::fixed_point_to_string_dp(daily_base_research_points, 2)
		);

		research_points_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_research_points_modifier()
		);

		const fixed_point_t research_points_modifier_from_tech =
			country->get_modifier_effect_value(*modifier_effect_cache.get_increase_research());
		if (research_points_modifier_from_tech != fixed_point_t::_0()) {
			static const StringName from_technology_localisation_key = "FROM_TECHNOLOGY";
			research_points_tooltip += "\n" + tr(from_technology_localisation_key) + ": " +
				_make_modifier_effect_value_coloured(
					*modifier_effect_cache.get_increase_research(), research_points_modifier_from_tech, true
				);
		}

		static const StringName daily_research_points_localisation_key = "TECH_DAILY_RESEARCHPOINTS_TOTAL_TOOLTIP";
		research_points_tooltip += "\n" + tr(daily_research_points_localisation_key).replace(
			value_replace_key, Utilities::fixed_point_to_string_dp(country->get_daily_research_points(), 2)
		);

		// In the base game this section is only shown when no research is set, but it's useful to show it always
		research_points_tooltip += "\n" + get_tooltip_separator();

		static const StringName accumulated_research_points_localisation_key = "RP_ACCUMULATED";
		static const String val_replace_key = "$VAL$";
		research_points_tooltip += tr(accumulated_research_points_localisation_key).replace(
			val_replace_key, Utilities::fixed_point_to_string_dp(country->get_research_point_stockpile(), 1)
		);

		ret[research_points_tooltip_key] = std::move(research_points_tooltip);
	}

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

	{
		static const StringName leadership_key = "leadership";
		static const StringName leadership_tooltip_key = "leadership_tooltip";

		ret[leadership_key] = country->get_leadership_point_stockpile().to_int64_t();

		String leadership_tooltip;

		fixed_point_t monthly_base_leadership_points;

		static const String value_replace_key = "$VALUE$";

		for (auto const& [pop_type, leadership_points] : country->get_leadership_points_from_pop_types()) {
			static const StringName pop_type_leadership_localisation_key = "TECH_DAILY_LEADERSHIP_TOOLTIP";
			static const String pop_type_replace_key = "$POPTYPE$";
			static const String fraction_replace_key = "$FRACTION$";
			static const String optimal_replace_key = "$OPTIMAL$";

			monthly_base_leadership_points += leadership_points;

			leadership_tooltip += tr(pop_type_leadership_localisation_key).replace(
				pop_type_replace_key, tr(Utilities::std_to_godot_string(pop_type->get_identifier()))
			).replace(
				value_replace_key, Utilities::fixed_point_to_string_dp(leadership_points, 2)
			).replace(
				fraction_replace_key, Utilities::fixed_point_to_string_dp(
					100 * country->get_pop_type_proportion(*pop_type) / country->get_total_population(), 2
				)
			).replace(
				optimal_replace_key, Utilities::fixed_point_to_string_dp(100 * pop_type->get_research_leadership_optimum(), 2)
			) + "\n";
		}

		// Empty prefix, "\n" suffix, fitting with the potential trailing "\n" from the pop type contributions and the upcoming
		// guaranteed monthly base leadership points line. All contributions are added to monthly_base_leadership_points.
		leadership_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_leadership(), nullptr, &monthly_base_leadership_points, {}, "\n"
		);

		// The monthly base leadership points line is guaranteed to be present, but those directly above and below it aren't,
		// so this line has no newline characters of its own. Instead, potential lines above finish with newlines and
		// potential (and some guaranteed) lines below start with them.
		static const StringName monthly_base_leadership_localisation_key = "TECH_DAILY_LEADERSHIP_BASE_TOOLTIP";
		leadership_tooltip += tr(monthly_base_leadership_localisation_key).replace(
			value_replace_key, Utilities::fixed_point_to_string_dp(monthly_base_leadership_points, 2)
		);

		leadership_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_leadership_modifier()
		);

		static const StringName monthly_leadership_points_localisation_key = "TECH_DAILY_LEADERSHIP_TOTAL_TOOLTIP";
		leadership_tooltip += "\n" + tr(monthly_leadership_points_localisation_key).replace(
			value_replace_key, Utilities::fixed_point_to_string_dp(country->get_monthly_leadership_points(), 2)
		);

		const fixed_point_t max_leadership_point_stockpile =
			definition_manager.get_define_manager().get_military_defines().get_max_leadership_point_stockpile();
		if (country->get_leadership_point_stockpile() >= max_leadership_point_stockpile) {
			leadership_tooltip += "\n" + get_tooltip_separator() + "\n";

			static const StringName max_leadership_points_localisation_key = "TOPBAR_LEADERSHIP_MAX";
			static const String max_replace_key = "$MAX$";
			leadership_tooltip += tr(max_leadership_points_localisation_key).trim_suffix("\n").replace(
				max_replace_key, Utilities::fixed_point_to_string_dp(max_leadership_point_stockpile, 1)
			);
		}

		ret[leadership_tooltip_key] = std::move(leadership_tooltip);
	}

	return ret;
}

/* TIME/SPEED CONTROL PANEL */

bool MenuSingleton::is_paused() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, true);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, true);

	return instance_manager->get_simulation_clock().is_paused();
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
