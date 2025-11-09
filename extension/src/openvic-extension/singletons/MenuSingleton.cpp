#include "MenuSingleton.hpp"

#include <span>

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/economy/GoodDefinition.hpp>
#include <openvic-simulation/modifier/Modifier.hpp>
#include <openvic-simulation/pop/PopType.hpp>
#include <openvic-simulation/types/Colour.hpp>
#include <openvic-simulation/politics/PartyPolicy.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>
#include <openvic-simulation/types/PopSprite.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/components/budget/BudgetMenu.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

MenuSingleton::population_menu_t::population_menu_t()
: pop_type_sort_cache { decltype(pop_type_sort_cache)::create_empty() },
province_sort_cache { decltype(province_sort_cache)::create_empty() },
rebel_type_sort_cache { decltype(rebel_type_sort_cache)::create_empty() } {}

StringName const& MenuSingleton::_signal_population_menu_province_list_changed() {
	return OV_SNAME(population_menu_province_list_changed);
}
StringName const& MenuSingleton::_signal_population_menu_province_list_selected_changed() {
	return OV_SNAME(population_menu_province_list_selected_changed);
}
StringName const& MenuSingleton::_signal_population_menu_pops_changed() {
	return OV_SNAME(population_menu_pops_changed);
}
StringName const& MenuSingleton::_signal_search_cache_changed() {
	return OV_SNAME(search_cache_changed);
}
StringName const& MenuSingleton::_signal_update_tooltip() {
	return OV_SNAME(update_tooltip);
}

String MenuSingleton::_make_modifier_effect_value(
	ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
) const {
	return Utilities::make_modifier_effect_value(
		*this,
		format_effect,
		value,
		plus_for_non_negative
	);
}

String MenuSingleton::_make_modifier_effect_value_coloured(
	ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
) const {
	return Utilities::make_modifier_effect_value_coloured(
		*this,
		format_effect,
		value,
		plus_for_non_negative
	);
}

String MenuSingleton::_make_modifier_effects_tooltip(ModifierValue const& modifier) const {
	String result;

	for (auto const& [effect, value] : modifier.get_values()) {
		if (value != fixed_point_t::_0) {
			result += "\n" + tr(Utilities::std_to_godot_string(effect->get_localisation_key())) + ": " +
				_make_modifier_effect_value_coloured(*effect, value, true);
		}
	}

	return result;
}

template<typename T>
requires std::same_as<T, CountryInstance> || std::same_as<T, ProvinceInstance>
String MenuSingleton::_make_modifier_effect_contributions_tooltip(
	T const& modifier_sum, ModifierEffect const& effect, fixed_point_t* effect_value,
	String const& prefix, String const& suffix
) const {
	String result;

	modifier_sum.for_each_contributing_modifier(
		effect,
		[this, &effect, effect_value, &prefix, &suffix, &result](
			modifier_entry_t const& modifier_entry, fixed_point_t value
		) -> void {
			using enum Modifier::modifier_type_t;

			if (effect_value != nullptr) {
				*effect_value += value;
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
	CountryInstance const&, ModifierEffect const&, fixed_point_t*, String const&, String const&
) const;
template String OpenVic::MenuSingleton::_make_modifier_effect_contributions_tooltip<ProvinceInstance>(
	ProvinceInstance const&, ModifierEffect const&, fixed_point_t*, String const&, String const&
) const;

String MenuSingleton::_make_rules_tooltip(RuleSet const& rules) const {
	if (rules.empty()) {
		return {};
	}

	const String enabled_text = vformat(": %sG%s%s!", GUILabel::get_colour_marker(), tr(OV_SNAME(YES)), GUILabel::get_colour_marker());
	const String disabled_text = vformat(": %sR%s%s!", GUILabel::get_colour_marker(), tr(OV_SNAME(NO)), GUILabel::get_colour_marker());

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

	static const String mobilisation_impact_tooltip_replace_impact_key = "$IMPACT$";
	static const String mobilisation_impact_tooltip_replace_policy_key = "$POLICY$";
	static const String mobilisation_impact_tooltip_replace_units_key = "$UNITS$";

	static const String mobilisation_impact_tooltip2_replace_curr_key = "$CURR$";
	static const String mobilisation_impact_tooltip2_replace_impact_key = "$IMPACT$";

	PartyPolicyGroup const* war_policy_issue_group = issue_manager.get_party_policy_group_by_identifier("war_policy");
	PartyPolicy const* war_policy_issue = war_policy_issue_group == nullptr
		? nullptr
		: country->get_ruling_party_untracked()->get_policies(*war_policy_issue_group);

	const String impact_string = Utilities::fixed_point_to_string_dp(country->get_mobilisation_impact() * 100, 1) + "%";

	return tr(
		OV_SNAME(MOBILIZATION_IMPACT_LIMIT_DESC)
	).replace(
		mobilisation_impact_tooltip_replace_impact_key, impact_string
	).replace(
		mobilisation_impact_tooltip_replace_policy_key, tr(
			war_policy_issue != nullptr
				? StringName { Utilities::std_to_godot_string(war_policy_issue->get_identifier()) }
				: OV_INAME("noIssue")
		)
	).replace(
		mobilisation_impact_tooltip_replace_units_key,
		String::num_uint64(country->get_mobilisation_max_regiment_count())
	) + "\n" + tr(
		OV_SNAME(MOBILIZATION_IMPACT_LIMIT_DESC2)
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
	OV_BIND_METHOD(MenuSingleton::get_province_info_from_number, { "province_number" });
	OV_BIND_METHOD(MenuSingleton::get_province_building_count);
	OV_BIND_METHOD(MenuSingleton::get_province_building_identifier, { "building_index" });
	OV_BIND_METHOD(MenuSingleton::get_slave_pop_icon_index);
	OV_BIND_METHOD(MenuSingleton::get_administrative_pop_icon_index);
	OV_BIND_METHOD(MenuSingleton::get_rgo_owner_pop_icon_index);

	/* TOPBAR */
	OV_BIND_METHOD(MenuSingleton::get_topbar_info);
	OV_BIND_METHOD(MenuSingleton::link_top_bar_to_cpp, { "godot_top_bar" });
	OV_BIND_METHOD(MenuSingleton::unlink_top_bar_from_cpp);

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
	OV_BIND_METHOD(MenuSingleton::population_menu_select_province, { "province_number" });
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
	OV_BIND_METHOD(MenuSingleton::get_military_menu_info);

	/* BUDGET MENU */
	OV_BIND_METHOD(MenuSingleton::link_budget_menu_to_cpp, { "godot_budget_menu" });
	OV_BIND_METHOD(MenuSingleton::unlink_budget_menu_from_cpp);

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

MenuSingleton::MenuSingleton() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

MenuSingleton::~MenuSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

String MenuSingleton::get_issue_identifier_suffix() {
	static const String issue_identifier_suffix = "_l";
	return issue_identifier_suffix;
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

	return Utilities::get_country_name(*this,*country);
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

	return Utilities::get_country_adjective(*this,*country);
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
	std::span<const BuildingInstance> buildings = province->get_buildings();

	if (buildings.empty()) {
		return {};
	}

	/* This system relies on the province buildings all being present in the right order. It will have to
	 * be changed if we want to support variable combinations and permutations of province buildings. */
	TypedArray<Dictionary> buildings_array;

	if (buildings_array.resize(buildings.size()) == OK) {
		for (size_t idx = 0; idx < buildings.size(); ++idx) {
			BuildingInstance const& building = buildings[idx];

			Dictionary building_dict;
			building_dict[OV_SNAME(level)] = static_cast<int32_t>(building.get_level());
			building_dict[OV_INAME("expansion_state")] = static_cast<int32_t>(building.get_expansion_state());
			building_dict[OV_SNAME(start_date)] = Utilities::date_to_string(building.get_start_date());
			building_dict[OV_SNAME(end_date)] = Utilities::date_to_string(building.get_end_date());
			building_dict[OV_INAME("expansion_progress")] = static_cast<real_t>(building.get_expansion_progress());

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

Dictionary MenuSingleton::get_province_info_from_number(int32_t province_number) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	ProvinceInstance const* province = instance_manager->get_map_instance().get_province_instance_from_number(province_number);
	if (province == nullptr) {
		return {};
	}
	Dictionary ret;

	ret[OV_SNAME(province)] = Utilities::std_to_godot_string(province->get_identifier());

	State const* state = province->get_state();
	if (state != nullptr) {
		ret[OV_SNAME(state)] = Utilities::get_state_name(*this,*state);
	}

	ret[OV_INAME("slave_status")] = province->get_slave();

	ret[OV_INAME("colony_status")] = static_cast<int32_t>(province->get_colony_status());

	TerrainType const* terrain_type = province->get_terrain_type();
	if (terrain_type != nullptr) {
		String terrain_type_string = Utilities::std_to_godot_string(terrain_type->get_identifier());

		static const String terrain_type_replace_key = "$TERRAIN$";
		static const String terrain_type_template_string = "%s" + get_tooltip_separator() + "%s" +
			GUILabel::get_colour_marker() + "Y%s" + GUILabel::get_colour_marker() + "!%s";

		ret[OV_INAME("terrain_type_tooltip")] = Utilities::format(
			terrain_type_template_string,
			tr(OV_INAME("PROVINCEVIEW_TERRAIN")).replace(terrain_type_replace_key, tr(terrain_type_string)),
			tr(OV_INAME("TERRAIN_MOVEMENT_COST")),
			Utilities::fixed_point_to_string_dp(terrain_type->get_movement_cost(), 2),
			_make_modifier_effects_tooltip(*terrain_type)
		);

		ret[OV_INAME("terrain_type")] = std::move(terrain_type_string);
	}

	ret[OV_SNAME(life_rating)] = province->get_life_rating();

	CountryInstance const* controller = province->get_controller();
	if (controller != nullptr) {
		ret[OV_SNAME(controller)] = Utilities::std_to_godot_string(controller->get_identifier());

		static const String controller_template_string = "%s %s";
		ret[OV_INAME("controller_tooltip")] = Utilities::format(
			controller_template_string,
			tr(OV_INAME("PV_CONTROLLER")),
			Utilities::get_country_name(*this,*controller)
		);
	}

	ResourceGatheringOperation const& rgo = province->get_rgo();

	if (rgo.is_valid()) {
		ProductionType const& production_type = *rgo.get_production_type_nullable();
		GoodDefinition const& rgo_good = *province->get_rgo_good();

		ret[OV_INAME("rgo_icon")] = static_cast<int32_t>(rgo_good.get_index());

		ret[OV_INAME("rgo_output_quantity_yesterday")] = static_cast<real_t>(rgo.get_output_quantity_yesterday());
		ret[OV_INAME("rgo_revenue_yesterday")] = static_cast<real_t>(rgo.get_revenue_yesterday());
		ret[OV_INAME("rgo_total_employees")] = rgo.get_total_employees_count_cache();
		const pop_size_t max_employee_count = rgo.get_max_employee_count_cache();
		if (max_employee_count == 0) {
			ret[OV_SNAME(rgo_employment_percentage)] = 100.0f;
		} else {
			ret[OV_SNAME(rgo_employment_percentage)] =
				static_cast<real_t>(rgo.get_total_employees_count_cache() * fixed_point_t::_100 / max_employee_count);
		}

		ModifierEffectCache const& modifier_effect_cache =
			game_singleton->get_definition_manager().get_modifier_manager().get_modifier_effect_cache();

		fixed_point_t output_from_workers = 1, throughput_from_workers = 0,
			output_multiplier = 1, throughput_multiplier = 1;
		String size_string, output_string, throughput_string;

		static const String employee_effect_template_string = "\n  -%s: " + GUILabel::get_colour_marker() + "Y%s" +
			GUILabel::get_colour_marker() + "!: %s";

		using enum Job::effect_t;

		std::optional<Job> const& owner_job = production_type.get_owner();
		if (owner_job.has_value()) {
			PopType const& owner_pop_type = *owner_job->get_pop_type();
			State const* state = province->get_state();

			if (unlikely(state == nullptr)) {
				spdlog::error_s(
					"Province \"{}\" has no state, preventing calculation of state-wide population proportion of RGO owner pop type \"{}\"",
					*province, owner_pop_type
				);
			} else {
				fixed_point_t effect_value = owner_job->get_effect_multiplier() * state->get_population_by_type(owner_pop_type);

				if (effect_value != fixed_point_t::_0) {
					effect_value /= state->get_total_population();
				}

				switch (owner_job->get_effect_type()) {
				case OUTPUT:
					output_multiplier += effect_value;
					output_string += Utilities::format(
						employee_effect_template_string,
						tr(OV_SNAME(PRODUCTION_FACTOR_OWNER)),
						tr(Utilities::std_to_godot_string(owner_pop_type.get_identifier())),
						_make_modifier_effect_value_coloured(
							*modifier_effect_cache.get_rgo_output_country(), effect_value, true
						)
					);
					break;
				case THROUGHPUT:
					throughput_multiplier += effect_value;
					throughput_string += Utilities::format(
						employee_effect_template_string,
						tr(OV_SNAME(PRODUCTION_FACTOR_OWNER)),
						tr(Utilities::std_to_godot_string(owner_pop_type.get_identifier())),
						_make_modifier_effect_value_coloured(
							*modifier_effect_cache.get_rgo_throughput_country(), effect_value, true
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

			amount_of_employees_by_pop_type += Utilities::format(
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
				const fixed_point_t effect_value = effect_multiplier == fixed_point_t::_1
					? relative_to_workforce
					: effect_multiplier * std::min(relative_to_workforce, job.get_amount());

				switch (job.get_effect_type()) {
					case OUTPUT:
						output_from_workers += effect_value;
						output_string += Utilities::format(
							employee_effect_template_string,
							tr(OV_SNAME(PRODUCTION_FACTOR_WORKER)),
							tr(Utilities::std_to_godot_string(pop_type.get_identifier())),
							_make_modifier_effect_value_coloured(
								*modifier_effect_cache.get_rgo_output_country(), effect_value, true
							)
						);
						break;
					case THROUGHPUT:
						throughput_from_workers += effect_value;
						throughput_string += Utilities::format(
							employee_effect_template_string,
							tr(OV_SNAME(PRODUCTION_FACTOR_WORKER)),
							tr(Utilities::std_to_godot_string(pop_type.get_identifier())),
							_make_modifier_effect_value_coloured(
								*modifier_effect_cache.get_rgo_throughput_country(), effect_value, true
							)
						);
						break;
					default:
						break;
				}
			}
		}

		{
			String output_modifiers, throughput_modifiers;

			static const String modifier_effect_contributions_prefix = "\n  -";

			output_modifiers += _make_modifier_effect_contributions_tooltip(
				*province, *modifier_effect_cache.get_rgo_output_country(), &output_multiplier,
				modifier_effect_contributions_prefix
			);
			output_modifiers += _make_modifier_effect_contributions_tooltip(
				*province, *modifier_effect_cache.get_local_rgo_output(), &output_multiplier,
				modifier_effect_contributions_prefix
			);
			throughput_modifiers += _make_modifier_effect_contributions_tooltip(
				*province, *modifier_effect_cache.get_rgo_throughput_country(), &throughput_multiplier,
				modifier_effect_contributions_prefix
			);
			throughput_modifiers += _make_modifier_effect_contributions_tooltip(
				*province, *modifier_effect_cache.get_local_rgo_throughput(), &throughput_multiplier,
				modifier_effect_contributions_prefix
			);

			fixed_point_t size_from_terrain, size_from_province;

			if (production_type.get_is_farm_for_non_tech()) {
				output_modifiers += _make_modifier_effect_contributions_tooltip(
					*province, *modifier_effect_cache.get_farm_rgo_output_global(), &output_multiplier,
					modifier_effect_contributions_prefix
				);
				output_modifiers += _make_modifier_effect_contributions_tooltip(
					*province, *modifier_effect_cache.get_farm_rgo_output_local(), &output_multiplier,
					modifier_effect_contributions_prefix
				);

				if (terrain_type != nullptr) {
					size_from_terrain = terrain_type->get_effect(*modifier_effect_cache.get_farm_rgo_size_local());
				}
				size_from_province = province->get_modifier_effect_value(*modifier_effect_cache.get_farm_rgo_size_local());
			}

			if (production_type.get_is_mine_for_non_tech()) {
				output_modifiers += _make_modifier_effect_contributions_tooltip(
					*province, *modifier_effect_cache.get_mine_rgo_output_global(), &output_multiplier,
					modifier_effect_contributions_prefix
				);
				output_modifiers += _make_modifier_effect_contributions_tooltip(
					*province, *modifier_effect_cache.get_mine_rgo_output_local(), &output_multiplier,
					modifier_effect_contributions_prefix
				);

				if (terrain_type != nullptr) {
					size_from_terrain += terrain_type->get_effect(*modifier_effect_cache.get_mine_rgo_size_local());
				}
				size_from_province += province->get_modifier_effect_value(*modifier_effect_cache.get_mine_rgo_size_local());
			}

			static const String size_modifier_template_string = "%s: %s\n";

			if (size_from_terrain != fixed_point_t::_0) {
				size_string = Utilities::format(
					size_modifier_template_string,
					tr(Utilities::std_to_godot_string(province->get_terrain_type()->get_identifier())),
					_make_modifier_effect_value_coloured(
						*modifier_effect_cache.get_farm_rgo_size_local(), size_from_terrain, false
					)
				);
			}

			if (size_from_province != fixed_point_t::_0) {
				size_string += Utilities::format(
					size_modifier_template_string,
					tr(OV_INAME("RGO_SIZE")),
					_make_modifier_effect_value_coloured(
						*modifier_effect_cache.get_farm_rgo_size_local(), size_from_province, false
					)
				);
			}

			ModifierEffectCache::good_effects_t const& good_effects =
				modifier_effect_cache.get_good_effects(production_type.get_output_good());

			fixed_point_t output_from_tech =
				province->get_modifier_effect_value(*modifier_effect_cache.get_rgo_output_tech()) +
				province->get_modifier_effect_value(*good_effects.get_rgo_goods_output());
			fixed_point_t throughput_from_tech =
				province->get_modifier_effect_value(*modifier_effect_cache.get_rgo_throughput_tech()) +
				province->get_modifier_effect_value(*good_effects.get_rgo_goods_throughput());

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

			if (size_from_tech != fixed_point_t::_0) {
				size_string += tr(OV_INAME("employ_from_tech")) + _make_modifier_effect_value_coloured(
					*modifier_effect_cache.get_farm_rgo_size_global(), size_from_tech, false
				);
			}

			static const String tech_modifier_template_string = modifier_effect_contributions_prefix + String { "%s: %s" };

			if (output_from_tech != fixed_point_t::_0) {
				output_multiplier += output_from_tech;

				output_string += Utilities::format(
					tech_modifier_template_string,
					tr(OV_INAME("RGO_OUTPUT_TECH")),
					_make_modifier_effect_value_coloured(
						*modifier_effect_cache.get_rgo_output_tech(), output_from_tech, true
					)
				);
			}

			if (throughput_from_tech != fixed_point_t::_0) {
				throughput_multiplier += throughput_from_tech;

				throughput_string += Utilities::format(
					tech_modifier_template_string,
					tr(OV_INAME("RGO_THROUGHPUT_TECH")),
					_make_modifier_effect_value_coloured(
						*modifier_effect_cache.get_rgo_throughput_tech(), throughput_from_tech, true
					)
				);
			}

			output_string += output_modifiers;
			throughput_string += throughput_modifiers;
		}

		static const String rgo_good_replace_key = "$GOODS$";
		static const String curr_replace_key = "$CURR$";
		static const String base_replace_key = "$BASE$";
		static const String rgo_production_template_string = "%s\n%s%s" + get_tooltip_separator() + "%s%s%s\n%s " +
			GUILabel::get_colour_marker() + "G100%%" + GUILabel::get_colour_marker() + "!%s\n\n%s%s%s";

		const fixed_point_t throughput_efficiency = throughput_from_workers * throughput_multiplier;
		const fixed_point_t output_efficiency = output_from_workers * output_multiplier;
		const fixed_point_t base_output = production_type.get_base_output_quantity();
		const fixed_point_t max_output = base_output * throughput_efficiency * output_efficiency;

		ret[OV_INAME("rgo_production_tooltip")] = Utilities::format(
			rgo_production_template_string,
			tr(OV_INAME("PROVINCEVIEW_GOODSINCOME")).replace(
				rgo_good_replace_key, tr(Utilities::std_to_godot_string(rgo_good.get_identifier()))
			).replace(
				Utilities::get_long_value_placeholder(),
				Utilities::fixed_point_to_string_dp(rgo.get_revenue_yesterday(), 3)
			),
			tr(OV_INAME("PRODUCTION_OUTPUT_GOODS_TOOLTIP2")).replace(curr_replace_key, Utilities::fixed_point_to_string_dp(max_output, 2)),
			tr(OV_INAME("PRODUCTION_OUTPUT_EXPLANATION")),
			tr(OV_INAME("PRODUCTION_BASE_OUTPUT_GOODS_TOOLTIP")).replace(base_replace_key, Utilities::fixed_point_to_string_dp(base_output, 2)),
			tr(OV_INAME("PRODUCTION_OUTPUT_EFFICIENCY_TOOLTIP")),
			_make_modifier_effect_value_coloured(*modifier_effect_cache.get_rgo_output_country(), output_efficiency, false),
			tr(OV_INAME("PRODUCTION_BASE_OUTPUT")),
			output_string,
			tr(OV_INAME("PRODUCTION_THROUGHPUT_EFFICIENCY_TOOLTIP")),
			_make_modifier_effect_value_coloured(
				*modifier_effect_cache.get_rgo_throughput_country(), throughput_efficiency, false
			),
			throughput_string
		);

		static const String rgo_employment_template_string = "%s" + get_tooltip_separator() + "%s%s\n%s%d\n%s\n%s" +
			GUILabel::get_colour_marker() + "G%d";

		ret[OV_INAME("rgo_employment_tooltip")] = Utilities::format(
			rgo_employment_template_string,
			tr(OV_INAME("PROVINCEVIEW_EMPLOYMENT")).replace(Utilities::get_long_value_placeholder(), {}),
			tr(OV_INAME("PRODUCTION_FACTORY_EMPLOYEECOUNT_TOOLTIP2")).replace(
				OV_INAME("$EMPLOYEES$"), String::num_int64(rgo.get_total_employees_count_cache())
			).replace(
				OV_INAME("$EMPLOYEE_MAX$"), String::num_int64(rgo.get_max_employee_count_cache())
			),
			amount_of_employees_by_pop_type,
			tr(OV_INAME("BASE_RGO_SIZE")),
			production_type.get_base_workforce_size(),
			size_string,
			tr(OV_INAME("FROM_PROV_SIZE")),
			static_cast<int32_t>(rgo.get_size_multiplier()) // TODO - remove cast once variable is an int32_t
		);
	}

	Crime const* crime = province->get_crime();
	if (crime != nullptr) {
		ret[OV_INAME("crime_name")] = Utilities::std_to_godot_string(crime->get_identifier());
		ret[OV_INAME("crime_icon")] = static_cast<int32_t>(crime->get_icon());
	}

	ret[OV_INAME("total_population")] = province->get_total_population();

	const auto make_pie_chart_tooltip = [this](
		has_get_identifier_and_colour auto const* key, String const& identifier, float weight, float total_weight
	) -> String {
		static const String format_key = "%d%% %s";
		return Utilities::format(
			format_key,
			static_cast<int32_t>(100.0f * weight / total_weight),
			tr(identifier)
		);
	};

	GFXPieChartTexture::godot_pie_chart_data_t pop_types =
		GFXPieChartTexture::distribution_to_slices_array(province->get_population_by_type(), make_pie_chart_tooltip);
	if (!pop_types.is_empty()) {
		ret[OV_INAME("pop_types")] = std::move(pop_types);
	}

	GFXPieChartTexture::godot_pie_chart_data_t ideologies =
		GFXPieChartTexture::distribution_to_slices_array(province->get_supporter_equivalents_by_ideology(), make_pie_chart_tooltip);
	if (!ideologies.is_empty()) {
		ret[OV_INAME("pop_ideologies")] = std::move(ideologies);
	}

	GFXPieChartTexture::godot_pie_chart_data_t cultures =
		GFXPieChartTexture::distribution_to_slices_array(province->get_population_by_culture(), make_pie_chart_tooltip);
	if (!cultures.is_empty()) {
		ret[OV_INAME("pop_cultures")] = std::move(cultures);
	}

	ordered_set<CountryInstance*> const& cores = province->get_cores();
	if (!cores.empty()) {
		PackedStringArray cores_array;
		if (cores_array.resize(cores.size()) == OK) {
			for (size_t idx = 0; idx < cores.size(); ++idx) {
				cores_array[idx] = Utilities::std_to_godot_string(cores.data()[idx]->get_identifier());
			}
			ret[OV_SNAME(cores)] = std::move(cores_array);
		} else {
			UtilityFunctions::push_error(
				"Failed to resize cores array to the correct size (", static_cast<int64_t>(cores.size()), ") for province ",
				Utilities::std_to_godot_string(province->get_identifier())
			);
		}
	}

	TypedArray<Dictionary> building_dict_array = _make_buildings_dict_array(province);
	if (!building_dict_array.is_empty()) {
		ret[OV_SNAME(buildings)] = std::move(building_dict_array);
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

	std::span<const BuildingType* const> province_building_types = game_singleton->get_definition_manager()
		.get_economy_manager().get_building_type_manager().get_province_building_types();
	ERR_FAIL_COND_V_MSG(
		building_index < 0 || building_index >= province_building_types.size(), {},
		Utilities::format("Invalid province building index: %d", building_index)
	);
	return Utilities::std_to_godot_string(province_building_types[building_index]->get_identifier());
}

int32_t MenuSingleton::get_slave_pop_icon_index() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);

	const pop_sprite_t sprite = game_singleton->get_definition_manager().get_pop_manager().get_slave_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "Slave sprite unset!");
	return sprite;
}

int32_t MenuSingleton::get_administrative_pop_icon_index() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);

	const pop_sprite_t sprite = game_singleton->get_definition_manager().get_pop_manager().get_administrative_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "Administrative sprite unset!");
	return sprite;
}

int32_t MenuSingleton::get_rgo_owner_pop_icon_index() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);

	const pop_sprite_t sprite = game_singleton->get_definition_manager().get_economy_manager().get_production_type_manager().get_rgo_owner_sprite();
	ERR_FAIL_COND_V_MSG(sprite <= 0, 0, "RGO owner sprite unset!");
	return sprite;
}

/* TOPBAR */

Dictionary MenuSingleton::get_topbar_info() const {
	CountryInstance* country = PlayerSingleton::get_singleton()->get_player_country();
	if (country == nullptr) {
		return {};
	}

	DefinitionManager const& definition_manager = GameSingleton::get_singleton()->get_definition_manager();
	ModifierEffectCache const& modifier_effect_cache = definition_manager.get_modifier_manager().get_modifier_effect_cache();

	Dictionary ret;

	// Country / Ranking
	ret[OV_SNAME(country)] = Utilities::std_to_godot_string(country->get_identifier());
	ret[OV_SNAME(country_status)] = static_cast<int32_t>(country->get_country_status());
	ret[OV_SNAME(total_rank)] = static_cast<uint64_t>(country->get_total_rank());

	// Production

	// Budget

	// Technology
	{
		Technology const* current_research = country->get_current_research_untracked();
		if (current_research != nullptr) {
			static const String tech_replace_key = "$TECH$";
			static const String date_replace_key = "$DATE$";
			static const String invested_replace_key = "$INVESTED$";
			static const String cost_replace_key = "$COST$";

			String current_tech_localised = tr(Utilities::std_to_godot_string(current_research->get_identifier()));

			ret[OV_SNAME(research_tooltip)] = tr(OV_INAME("TECHNOLOGYVIEW_RESEARCH_TOOLTIP")).replace(
				tech_replace_key, current_tech_localised
			).replace(
				date_replace_key, Utilities::date_to_formatted_string(country->get_expected_research_completion_date_untracked(), false)
			) + "\n" + tr(OV_INAME("TECHNOLOGYVIEW_RESEARCH_INVESTED_TOOLTIP")).replace(
				invested_replace_key, String::num_uint64(country->get_invested_research_points_untracked().truncate<int64_t>())
			).replace(cost_replace_key, String::num_uint64(country->get_current_research_cost_untracked().truncate<int64_t>()));

			ret[OV_SNAME(research)] = std::move(current_tech_localised);

			ret[OV_INAME("research_progress")] = static_cast<real_t>(country->research_progress.get_untracked());
		} else if (country->is_civilised()) {
			ret[OV_SNAME(research)] = tr(OV_INAME("TB_TECH_NO_CURRENT"));
			ret[OV_SNAME(research_tooltip)] = tr(OV_INAME("TECHNOLOGYVIEW_NO_RESEARCH_TOOLTIP"));
		} else {
			ret[OV_SNAME(research)] = vformat("%sR%s", GUILabel::get_colour_marker(), tr(OV_INAME("unciv_nation")));
			ret[OV_SNAME(research_tooltip)] = tr(OV_INAME("TECHNOLOGYVIEW_NO_RESEARCH_UNCIV_TOOLTIP"));
		}

		ret[OV_SNAME(literacy)] = static_cast<real_t>(country->get_average_literacy());
		// TODO - set monthly literacy change (test for precision issues)
		ret[OV_INAME("literacy_change")] = 0.0f;

		ret[OV_INAME("research_points")] = static_cast<real_t>(country->get_daily_research_points_untracked());

		String research_points_tooltip;

		fixed_point_t daily_base_research_points;

		for (auto const& [pop_type, research_points] : country->get_research_points_from_pop_types()) {
			static const String pop_type_replace_key = "$POPTYPE$";
			static const String fraction_replace_key = "$FRACTION$";
			static const String optimal_replace_key = "$OPTIMAL$";

			daily_base_research_points += research_points;

			research_points_tooltip += tr(OV_INAME("TECH_DAILY_RESEARCHPOINTS_TOOLTIP")).replace(
				pop_type_replace_key, tr(Utilities::std_to_godot_string(pop_type->get_identifier()))
			).replace(
				Utilities::get_long_value_placeholder(),
				Utilities::fixed_point_to_string_dp(research_points, 2)
			).replace(
				fraction_replace_key, Utilities::fixed_point_to_string_dp(
					fixed_point_t::_100 * country->get_population_by_type(*pop_type) / country->get_total_population(), 2
				)
			).replace(
				optimal_replace_key, Utilities::fixed_point_to_string_dp(100 * pop_type->get_research_leadership_optimum(), 2)
			) + "\n";
		}

		// Empty prefix, "\n" suffix, fitting with the potential trailing "\n" from the pop type contributions and the upcoming
		// guaranteed daily base research points line. All contributions are added to daily_base_research_points.
		research_points_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_research_points(), &daily_base_research_points, {}, "\n"
		);

		// The daily base research points line is guaranteed to be present, but those directly above and below it aren't,
		// so this line has no newline characters of its own. Instead, potential lines above finish with newlines and
		// potential (and some guaranteed) lines below start with them.
		research_points_tooltip += tr(OV_INAME("TECH_DAILY_RESEARCHPOINTS_BASE_TOOLTIP")).replace(
			Utilities::get_long_value_placeholder(),
			Utilities::fixed_point_to_string_dp(daily_base_research_points, 2)
		);

		research_points_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_research_points_modifier()
		);

		const fixed_point_t research_points_modifier_from_tech =
			country->get_modifier_effect_value(*modifier_effect_cache.get_increase_research());
		if (research_points_modifier_from_tech != fixed_point_t::_0) {
			research_points_tooltip += "\n" + tr(OV_INAME("FROM_TECHNOLOGY")) + ": " +
				_make_modifier_effect_value_coloured(
					*modifier_effect_cache.get_increase_research(), research_points_modifier_from_tech, true
				);
		}

		research_points_tooltip += "\n" + tr(OV_INAME("TECH_DAILY_RESEARCHPOINTS_TOTAL_TOOLTIP")).replace(
			Utilities::get_long_value_placeholder(),
			Utilities::fixed_point_to_string_dp(country->get_daily_research_points_untracked(), 2)
		);

		// In the base game this section is only shown when no research is set, but it's useful to show it always
		research_points_tooltip += "\n" + get_tooltip_separator();

		research_points_tooltip += tr(OV_INAME("RP_ACCUMULATED")).replace(
			Utilities::get_short_value_placeholder(),
			Utilities::fixed_point_to_string_dp(country->get_research_point_stockpile_untracked(), 1)
		);

		ret[OV_INAME("research_points_tooltip")] = std::move(research_points_tooltip);
	}

	// Politics

	// Population

	// Trade

	// Diplomacy

	// Military
	ret[OV_INAME("regiment_count")] = static_cast<uint64_t>(country->get_regiment_count());
	ret[OV_INAME("max_supported_regiments")] = static_cast<uint64_t>(country->get_max_supported_regiment_count());

	if (country->is_mobilised()) {
		ret[OV_SNAME(is_mobilised)] = true;
	} else {
		ret[OV_INAME("mobilisation_regiments")] = static_cast<uint64_t>(country->get_mobilisation_potential_regiment_count());
		ret[OV_SNAME(mobilisation_impact_tooltip)] = _make_mobilisation_impact_tooltip();
	}

	{
		ret[OV_SNAME(leadership)] = country->get_leadership_point_stockpile().truncate<int64_t>();

		String leadership_tooltip;

		fixed_point_t monthly_base_leadership_points;

		for (auto const& [pop_type, leadership_points] : country->get_leadership_points_from_pop_types()) {
			static const String pop_type_replace_key = "$POPTYPE$";
			static const String fraction_replace_key = "$FRACTION$";
			static const String optimal_replace_key = "$OPTIMAL$";

			monthly_base_leadership_points += leadership_points;

			leadership_tooltip += tr(OV_INAME("TECH_DAILY_LEADERSHIP_TOOLTIP")).replace(
				pop_type_replace_key, tr(Utilities::std_to_godot_string(pop_type->get_identifier()))
			).replace(
				Utilities::get_long_value_placeholder(),
				Utilities::fixed_point_to_string_dp(leadership_points, 2)
			).replace(
				fraction_replace_key, Utilities::fixed_point_to_string_dp(
					fixed_point_t::_100 * country->get_population_by_type(*pop_type) / country->get_total_population(), 2
				)
			).replace(
				optimal_replace_key, Utilities::fixed_point_to_string_dp(100 * pop_type->get_research_leadership_optimum(), 2)
			) + "\n";
		}

		// Empty prefix, "\n" suffix, fitting with the potential trailing "\n" from the pop type contributions and the upcoming
		// guaranteed monthly base leadership points line. All contributions are added to monthly_base_leadership_points.
		leadership_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_leadership(), &monthly_base_leadership_points, {}, "\n"
		);

		// The monthly base leadership points line is guaranteed to be present, but those directly above and below it aren't,
		// so this line has no newline characters of its own. Instead, potential lines above finish with newlines and
		// potential (and some guaranteed) lines below start with them.
		leadership_tooltip += tr(OV_INAME("TECH_DAILY_LEADERSHIP_BASE_TOOLTIP")).replace(
			Utilities::get_long_value_placeholder(),
			Utilities::fixed_point_to_string_dp(monthly_base_leadership_points, 2)
		);

		leadership_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_leadership_modifier()
		);

		leadership_tooltip += "\n" + tr(OV_INAME("TECH_DAILY_LEADERSHIP_TOTAL_TOOLTIP")).replace(
			Utilities::get_long_value_placeholder(),
			Utilities::fixed_point_to_string_dp(country->get_monthly_leadership_points(), 2)
		);

		const fixed_point_t max_leadership_point_stockpile =
			definition_manager.get_define_manager().get_military_defines().get_max_leadership_point_stockpile();
		if (country->get_leadership_point_stockpile() >= max_leadership_point_stockpile) {
			leadership_tooltip += "\n" + get_tooltip_separator() + "\n";

			static const String max_replace_key = "$MAX$";
			leadership_tooltip += tr(OV_INAME("TOPBAR_LEADERSHIP_MAX")).trim_suffix("\n").replace(
				max_replace_key, Utilities::fixed_point_to_string_dp(max_leadership_point_stockpile, 1)
			);
		}

		ret[OV_INAME("leadership_tooltip")] = std::move(leadership_tooltip);
	}

	return ret;
}
void MenuSingleton::link_top_bar_to_cpp(GUINode const* const godot_top_bar) {
	ERR_FAIL_NULL(godot_top_bar);
	if (top_bar) {
		UtilityFunctions::push_error(
			"Trying to link new C++ and GDScript TopBar instances without unlinking the old instances first! "
			"The unlinking must happen just before the GDScript TopBar is freed, "
			"otherwise the C++ TopBar will continue running despite all its UI node pointers now being invalid."
		);

		unlink_top_bar_from_cpp();
	}
	top_bar = memory::make_unique<TopBar>(
		*godot_top_bar
	);
	GameSingleton::get_singleton()->gamestate_updated.connect(&TopBar::update, top_bar.get());
}
void MenuSingleton::unlink_top_bar_from_cpp() {
	if (top_bar) {
		GameSingleton::get_singleton()->gamestate_updated.disconnect(&TopBar::update, top_bar.get());
		top_bar.reset();
	} else {
		UtilityFunctions::push_warning("unlink_top_bar_from_cpp called but no C++ TopBar instance was linked!");
	}
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

/* BUDGET MENU */
void MenuSingleton::link_budget_menu_to_cpp(GUINode const* const godot_budget_menu) {
	ERR_FAIL_NULL(godot_budget_menu);
	GameSingleton& game_singleton = *GameSingleton::get_singleton();

	if (budget_menu) {
		UtilityFunctions::push_error(
			"Trying to link new C++ and GDScript BudgetMenu instances without unlinking the old instances first! "
			"The unlinking must happen just before the GDScript BudgetMenu is freed, "
			"otherwise the C++ BudgetMenu will continue running despite all its UI node pointers now being invalid."
		);

		unlink_budget_menu_from_cpp();
	}

	auto const& strata_keys = game_singleton.get_definition_manager().get_pop_manager().get_stratas();
	ModifierEffectCache const& modifier_effect_cache = game_singleton.get_definition_manager().get_modifier_manager().get_modifier_effect_cache();
	CountryDefines const& country_defines = game_singleton.get_definition_manager().get_define_manager().get_country_defines();
	budget_menu = memory::make_unique<BudgetMenu>(
		*godot_budget_menu,
		strata_keys,
		modifier_effect_cache,
		country_defines
	);
	game_singleton.gamestate_updated.connect(&BudgetMenu::update, budget_menu.get());
}

void MenuSingleton::unlink_budget_menu_from_cpp() {
	if (budget_menu) {
		GameSingleton::get_singleton()->gamestate_updated.disconnect(&BudgetMenu::update, budget_menu.get());
		budget_menu.reset();
	} else {
		UtilityFunctions::push_warning("unlink_budget_menu_from_cpp called but no C++ BudgetMenu instance was linked!");
	}
}

/* Find/Search Panel */

Error MenuSingleton::generate_search_cache() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, FAILED);

	search_panel.entry_cache.clear();

	std::span<const ProvinceInstance> provinces = instance_manager->get_map_instance().get_province_instances();
	std::span<const StateSet> state_sets = instance_manager->get_map_instance().get_state_manager().get_state_sets();
	std::span<const CountryInstance> countries = instance_manager->get_country_instance_manager().get_country_instances();

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
			String display_name = Utilities::get_state_name(*this,state);
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
			String display_name = Utilities::get_country_name(*this,country);
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
		Utilities::format("Invalid start for search panel result rows: %d", start)
	);
	ERR_FAIL_COND_V_MSG(count <= 0, {}, Utilities::format("Invalid count for search panel result rows: %d", count));

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
