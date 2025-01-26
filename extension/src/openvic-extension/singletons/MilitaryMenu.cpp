#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/military/UnitInstanceGroup.hpp>

#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

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

			UnitInstanceGroup<Branch> const* group = leader.get_unit_instance_group();
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
		DefinitionManager const& definition_manager = GameSingleton::get_singleton()->get_definition_manager();

		const fixed_point_t prestige = leader.get_prestige();

		modifier_value =
			definition_manager.get_military_manager().get_leader_trait_manager().get_leader_prestige_modifier() * prestige;

		static const StringName prestige_localisation_key = "PRESTIGE_SCORE";
		static const String value_replace_key = "$VAL$";

		String prestige_tooltip = tr(prestige_localisation_key).replace(
			value_replace_key, Utilities::float_to_string_dp(prestige * 100, 2) + "%"
		);

		tooltip += "\n" + prestige_tooltip;

		ModifierEffectCache const& modifier_effect_cache =
			definition_manager.get_modifier_manager().get_modifier_effect_cache();

		ModifierEffect const& morale_effect = *modifier_effect_cache.get_morale_leader();
		ModifierEffect const& organisation_effect = *modifier_effect_cache.get_organisation();

		static const StringName morale_localisation_key = "PRESTIGE_MORALE_BONUS";
		static const StringName organisation_localisation_key = "PRESTIGE_MAX_ORG_BONUS";

		prestige_tooltip += "\n" + tr(morale_localisation_key).replace(
			value_replace_key,
			_make_modifier_effect_value_coloured(morale_effect, modifier_value.get_effect(morale_effect), true)
		) + "\n" + tr(organisation_localisation_key).replace(
			value_replace_key,
			_make_modifier_effect_value_coloured(organisation_effect, modifier_value.get_effect(organisation_effect), true)
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

static inline int32_t _scale_land_unit_strength(fixed_point_t strength) {
	static constexpr int32_t LAND_UNIT_STRENGTH_FACTOR = 1000;

	return (strength * LAND_UNIT_STRENGTH_FACTOR).to_int32_t();
}

template<UnitType::branch_t Branch>
Dictionary MenuSingleton::make_unit_group_dict(UnitInstanceGroup<Branch> const& unit_group) {
	static const StringName military_info_unit_group_leader_picture_key = "unit_group_leader_picture";
	static const StringName military_info_unit_group_leader_tooltip_key = "unit_group_leader_tooltip";
	static const StringName military_info_unit_group_name_key           = "unit_group_name";
	static const StringName military_info_unit_group_location_key       = "unit_group_location";
	static const StringName military_info_unit_group_unit_count_key     = "unit_group_unit_count";
	static const StringName military_info_unit_group_men_count_key      = "unit_group_men_count"; // armies only
	static const StringName military_info_unit_group_max_men_count_key  = "unit_group_max_men_count"; // armies only
	static const StringName military_info_unit_group_organisation_key   = "unit_group_organisation";
	static const StringName military_info_unit_group_strength_key       = "unit_group_strength";
	static const StringName military_info_unit_group_moving_tooltip_key = "unit_group_moving_tooltip";
	static const StringName military_info_unit_group_dig_in_tooltip_key  = "unit_group_dig_in_tooltip"; // armies only
	static const StringName military_info_unit_group_combat_key         = "unit_group_combat";

	using enum UnitType::branch_t;

	Dictionary unit_group_dict;

	if (unit_group.get_leader() != nullptr) {
		static const StringName military_info_leader_picture_key = "leader_picture";
		static const StringName military_info_leader_tooltip_key = "leader_tooltip";

		const Dictionary leader_dict = make_leader_dict(*unit_group.get_leader());

		unit_group_dict[military_info_unit_group_leader_picture_key] =
			leader_dict.get(military_info_leader_picture_key, Ref<Texture2D> {});
		unit_group_dict[military_info_unit_group_leader_tooltip_key] =
			leader_dict.get(military_info_leader_tooltip_key, String {});
	}

	unit_group_dict[military_info_unit_group_name_key] = Utilities::std_to_godot_string(unit_group.get_name());
	if (unit_group.get_position() != nullptr) {
		unit_group_dict[military_info_unit_group_location_key] =
			Utilities::std_to_godot_string(unit_group.get_position()->get_identifier());
	}
	unit_group_dict[military_info_unit_group_unit_count_key] = static_cast<uint64_t>(unit_group.get_unit_count());

	unit_group_dict[military_info_unit_group_organisation_key] = unit_group.get_organisation_proportion().to_float();
	unit_group_dict[military_info_unit_group_strength_key] = unit_group.get_strength_proportion().to_float();

	if (unit_group.is_moving()) {
		static const StringName moving_localisation_key = "MILITARY_MOVING_TOOLTIP";
		static const String moving_location_replace_key = "$LOCATION$";
		static const String moving_date_replace_key = "$DATE$";

		ProvinceInstance const* destination = unit_group.get_movement_destination_province();

		unit_group_dict[military_info_unit_group_moving_tooltip_key] = tr(moving_localisation_key).replace(
			moving_location_replace_key,
			tr(GUINode::format_province_name(
				destination != nullptr ? Utilities::std_to_godot_string(destination->get_identifier()) : String {}, false
			))
		).replace(moving_date_replace_key, Utilities::date_to_string(unit_group.get_movement_arrival_date()));
	}

	if constexpr (Branch == LAND) {
		ArmyInstance const& army = static_cast<ArmyInstance const&>(unit_group);

		unit_group_dict[military_info_unit_group_men_count_key] = _scale_land_unit_strength(army.get_total_strength());
		unit_group_dict[military_info_unit_group_max_men_count_key] = _scale_land_unit_strength(army.get_total_max_strength());

		const ArmyInstance::dig_in_level_t dig_in_level = army.get_dig_in_level();

		if (dig_in_level > 0) {
			static const StringName dig_in_localisation_key = "MILITARY_DIGIN_TOOLTIP";
			static const String moving_days_replace_key = "$DAYS$";

			unit_group_dict[military_info_unit_group_dig_in_tooltip_key] = tr(dig_in_localisation_key).replace(
				moving_days_replace_key, String::num_uint64(dig_in_level)
			);
		}
	}

	if (unit_group.is_in_combat()) {
		unit_group_dict[military_info_unit_group_combat_key] = true;
	}

	return unit_group_dict;
}

Dictionary MenuSingleton::make_in_progress_unit_dict() const {
	static const StringName military_info_unit_progress_key = "unit_progress";
	static const StringName military_info_unit_icon_key = "unit_icon";
	static const StringName military_info_unit_name_key = "unit_name";
	static const StringName military_info_unit_location_key = "unit_location";
	static const StringName military_info_unit_eta_key = "unit_eta";
	static const StringName military_info_unit_tooltip_key = "unit_tooltip";

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	DefinitionManager const& definition_manager = game_singleton->get_definition_manager();
	GoodDefinitionManager const& good_definition_manager =
		definition_manager.get_economy_manager().get_good_definition_manager();

	// TODO - remove test data, read actual in-progress units from SIM
	UnitType const* unit_type = definition_manager.get_military_manager().get_unit_type_manager().get_unit_type_by_index(0);
	ProvinceInstance const* location = game_singleton->get_viewed_country()->get_capital();
	const Date eta { 1900 };
	const fixed_point_t progress = fixed_point_t::_0_50();
	const ordered_map<GoodDefinition const*, std::pair<fixed_point_t, fixed_point_t>> required_goods {
		{
			good_definition_manager.get_good_definition_by_index(0),
			{ fixed_point_t::parse(1234) / 100, fixed_point_t::parse(1900) / 100 }
		}, {
			good_definition_manager.get_good_definition_by_index(1),
			{ fixed_point_t::parse(888) / 100, fixed_point_t::parse(1444) / 100 }
		}, {
			good_definition_manager.get_good_definition_by_index(2),
			{ fixed_point_t::parse(1622) / 100, fixed_point_t::parse(1622) / 100 }
		}, {
			good_definition_manager.get_good_definition_by_index(3),
			{ fixed_point_t::parse(211) / 100, fixed_point_t::parse(805) / 100 }
		}
	};

	Dictionary in_progress_unit_dict;

	in_progress_unit_dict[military_info_unit_progress_key] = progress.to_float();
	in_progress_unit_dict[military_info_unit_icon_key] = unit_type->get_icon();
	in_progress_unit_dict[military_info_unit_name_key] = Utilities::std_to_godot_string(unit_type->get_identifier());
	in_progress_unit_dict[military_info_unit_location_key] = Utilities::std_to_godot_string(location->get_identifier());
	in_progress_unit_dict[military_info_unit_eta_key] = Utilities::date_to_string(eta);

	String tooltip;

	for (auto const& [good, required_amounts] : required_goods) {
		if (required_amounts.first < required_amounts.second) {
			tooltip += "\n" + tr(Utilities::std_to_godot_string(good->get_identifier())) + " - " +
				Utilities::fixed_point_to_string_dp(required_amounts.first, 2) + "/" +
				Utilities::fixed_point_to_string_dp(required_amounts.second, 2);
		}
	}

	if (!tooltip.is_empty()) {
		static const StringName gathering_goods_localisation_key = "GOODS_PROJECT_LACK_GOODS";
		in_progress_unit_dict[military_info_unit_tooltip_key] = tr(gathering_goods_localisation_key) + tooltip;
	}

	return in_progress_unit_dict;
}

using leader_sort_func_t = bool (*)(LeaderBase const*, LeaderBase const*);

static leader_sort_func_t _get_leader_sort_func(MenuSingleton::LeaderSortKey leader_sort_key) {
	static const auto get_assignment = [](LeaderBase const* leader) -> std::string_view {
		static const auto get_assignment_template =
			[]<UnitType::branch_t Branch>(LeaderBranched<Branch> const* leader) -> std::string_view {
				UnitInstanceGroup<Branch> const* group = leader->get_unit_instance_group();
				return group != nullptr ? group->get_name() : std::string_view {};
			};

		using enum UnitType::branch_t;
		switch (leader->get_branch()) {
		case LAND:
			return get_assignment_template(static_cast<General const*>(leader));
		case NAVAL:
			return get_assignment_template(static_cast<Admiral const*>(leader));
		default:
			return {};
		}
	};

	using enum MenuSingleton::LeaderSortKey;

	switch (leader_sort_key) {
	case LEADER_SORT_PRESTIGE:
		return [](LeaderBase const* a, LeaderBase const* b) -> bool {
			return a->get_prestige() < b->get_prestige();
		};
	case LEADER_SORT_TYPE:
		return [](LeaderBase const* a, LeaderBase const* b) -> bool {
			return a->get_branch() < b->get_branch();
		};
	case LEADER_SORT_NAME:
		return [](LeaderBase const* a, LeaderBase const* b) -> bool {
			return a->get_name() < b->get_name();
		};
	case LEADER_SORT_ASSIGNMENT:
		return [](LeaderBase const* a, LeaderBase const* b) -> bool {
			return get_assignment(a) < get_assignment(b);
		};
	default:
		UtilityFunctions::push_error("Invalid miltiary menu leader sort key: ", leader_sort_key);
		return [](LeaderBase const* a, LeaderBase const* b) -> bool { return false; };
	}
}

template<UnitType::branch_t Branch>
using unit_group_sort_func_t = bool (*)(UnitInstanceGroup<Branch> const*, UnitInstanceGroup<Branch> const*);

template<UnitType::branch_t Branch>
static unit_group_sort_func_t<Branch> _get_unit_group_sort_func(MenuSingleton::UnitGroupSortKey unit_group_sort_key) {
	using enum MenuSingleton::UnitGroupSortKey;

	switch (unit_group_sort_key) {
	case UNIT_GROUP_SORT_NAME:
		return [](UnitInstanceGroup<Branch> const* a, UnitInstanceGroup<Branch> const* b) -> bool {
			return a->get_name() < b->get_name();
		};
	case UNIT_GROUP_SORT_STRENGTH:
		return [](UnitInstanceGroup<Branch> const* a, UnitInstanceGroup<Branch> const* b) -> bool {
			return a->get_unit_count() < b->get_unit_count();
		};
	default:
		UtilityFunctions::push_error(
			"Invalid miltiary menu ", Utilities::std_to_godot_string(UnitType::get_branched_unit_group_name(Branch)),
			" sort key: ", unit_group_sort_key
		);
		return [](UnitInstanceGroup<Branch> const* a, UnitInstanceGroup<Branch> const* b) -> bool { return false; };
	}
}

Dictionary MenuSingleton::get_military_menu_info(
	LeaderSortKey leader_sort_key, bool sort_leaders_descending,
	UnitGroupSortKey army_sort_key, bool sort_armies_descending,
	UnitGroupSortKey navy_sort_key, bool sort_navies_descending
) {
	cached_leader_dicts.clear();

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	DefinitionManager const& definition_manager = game_singleton->get_definition_manager();
	ModifierEffectCache const& modifier_effect_cache = definition_manager.get_modifier_manager().get_modifier_effect_cache();
	StaticModifierCache const& static_modifier_cache = definition_manager.get_modifier_manager().get_static_modifier_cache();
	IssueManager const& issue_manager = definition_manager.get_politics_manager().get_issue_manager();

	CountryInstance const* country = game_singleton->get_viewed_country();
	if (country == nullptr) {
		return {};
	}

	Dictionary ret;

	// Military stats
	static const StringName military_info_war_exhaustion_key = "war_exhaustion";
	static const StringName military_info_war_exhaustion_tooltip_key = "war_exhaustion_tooltip";
	static const StringName military_info_supply_consumption_key = "supply_consumption";
	static const StringName military_info_supply_consumption_tooltip_key = "supply_consumption_tooltip";
	static const StringName military_info_organisation_regain_key = "organisation_regain";
	static const StringName military_info_organisation_regain_tooltip_key = "organisation_regain_tooltip";
	static const StringName military_info_land_organisation_key = "land_organisation";
	static const StringName military_info_land_organisation_tooltip_key = "land_organisation_tooltip";
	static const StringName military_info_naval_organisation_key = "naval_organisation";
	static const StringName military_info_naval_organisation_tooltip_key = "naval_organisation_tooltip";
	static const StringName military_info_land_unit_start_experience_key = "land_unit_start_experience";
	static const StringName military_info_naval_unit_start_experience_key = "naval_unit_start_experience";
	static const StringName military_info_unit_start_experience_tooltip_key = "unit_start_experience_tooltip";
	static const StringName military_info_recruit_time_key = "recruit_time";
	static const StringName military_info_recruit_time_tooltip_key = "recruit_time_tooltip";
	static const StringName military_info_combat_width_key = "combat_width";
	static const StringName military_info_combat_width_tooltip_key = "combat_width_tooltip";
	static const StringName military_info_dig_in_cap_key = "dig_in_cap";
	static const StringName military_info_military_tactics_key = "military_tactics";

	static const String war_exhaustion_template_string = "%s/%s";
	const String war_exhaustion_string = Utilities::fixed_point_to_string_dp(country->get_war_exhaustion(), 2);
	const String max_war_exhaustion_string = Utilities::fixed_point_to_string_dp(country->get_war_exhaustion_max(), 2);
	ret[military_info_war_exhaustion_key] = vformat(
		war_exhaustion_template_string, war_exhaustion_string, max_war_exhaustion_string
	);

	static const StringName war_exhaution_localisation_key = "MILITARY_WAR_EXHAUSTION_TOOLTIP";
	static const StringName max_war_exhaution_localisation_key = "MILITARY_MAX_WAR_EXHAUSTION_TOOLTIP";
	static const StringName value_replace_key = "$VALUE$";
	static const String current_effects_localisation_key = "WEX_EFFECTS";
	static const String war_exhaustion_tooltip_template_string = "%s%s\n\n%s%s" + get_tooltip_separator() + "%s%s";

	ret[military_info_war_exhaustion_tooltip_key] = vformat(
		war_exhaustion_tooltip_template_string,
		tr(war_exhaution_localisation_key).replace(value_replace_key, war_exhaustion_string),
		_make_modifier_effect_contributions_tooltip(*country, *modifier_effect_cache.get_war_exhaustion()),
		tr(max_war_exhaution_localisation_key).replace(value_replace_key, max_war_exhaustion_string),
		_make_modifier_effect_contributions_tooltip(*country, *modifier_effect_cache.get_max_war_exhaustion()),
		tr(current_effects_localisation_key),
		_make_modifier_effects_tooltip(static_modifier_cache.get_war_exhaustion() * country->get_war_exhaustion())
	);

	static const StringName base_value_percent_localisation_key = "MILITARY_BASEVALUE_PERCENT";
	const String base_value_percent_tooltip = tr(base_value_percent_localisation_key);

	ret[military_info_supply_consumption_key] = country->get_supply_consumption().to_float();
	ret[military_info_supply_consumption_tooltip_key] = base_value_percent_tooltip + _make_modifier_effect_contributions_tooltip(
		*country, *modifier_effect_cache.get_supply_consumption()
	);

	static const StringName from_tech_localisation_key = "FROM_TECHNOLOGY";
	const String from_technology_tooltip = "\n" + tr(from_tech_localisation_key) + ": ";

	ret[military_info_organisation_regain_key] = country->get_organisation_regain().to_float();
	{
		String organisation_regain_tooltip = base_value_percent_tooltip;
		const fixed_point_t morale = country->get_modifier_effect_value(*modifier_effect_cache.get_morale_global());
		if (morale != fixed_point_t::_0()) {
			organisation_regain_tooltip += from_technology_tooltip + _make_modifier_effect_value_coloured(
				*modifier_effect_cache.get_morale_global(), morale, true
			);
		}
		organisation_regain_tooltip += _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_org_regain()
		);
		ret[military_info_organisation_regain_tooltip_key] = std::move(organisation_regain_tooltip);
	}

	ret[military_info_land_organisation_key] = country->get_land_organisation().to_float();
	ret[military_info_land_organisation_tooltip_key] = base_value_percent_tooltip + _make_modifier_effect_contributions_tooltip(
		*country, *modifier_effect_cache.get_land_organisation()
	);

	ret[military_info_naval_organisation_key] = country->get_naval_organisation().to_float();
	ret[military_info_naval_organisation_tooltip_key] = base_value_percent_tooltip + _make_modifier_effect_contributions_tooltip(
		*country, *modifier_effect_cache.get_naval_organisation()
	);

	ret[military_info_land_unit_start_experience_key] = country->get_land_unit_start_experience().to_float();
	ret[military_info_naval_unit_start_experience_key] = country->get_naval_unit_start_experience().to_float();
	{
		static const StringName base_value_localisation_key = "MILITARY_BASEVALUE";
		String unit_start_experience_tooltip = tr(base_value_localisation_key);
		const fixed_point_t regular_experience_level = country->get_modifier_effect_value(
			*modifier_effect_cache.get_regular_experience_level()
		);
		if (regular_experience_level != fixed_point_t::_0()) {
			unit_start_experience_tooltip += from_technology_tooltip + _make_modifier_effect_value_coloured(
				*modifier_effect_cache.get_regular_experience_level(), regular_experience_level, true
			);
		}
		const String land_unit_start_experience_tooltip = _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_land_unit_start_experience()
		);
		const String naval_unit_start_experience_tooltip = _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_naval_unit_start_experience()
		);
		unit_start_experience_tooltip += land_unit_start_experience_tooltip;
		if (!land_unit_start_experience_tooltip.is_empty() && !naval_unit_start_experience_tooltip.is_empty()) {
			unit_start_experience_tooltip += "\n";
		}
		unit_start_experience_tooltip += naval_unit_start_experience_tooltip;
		ret[military_info_unit_start_experience_tooltip_key] = std::move(unit_start_experience_tooltip);
	}

	ret[military_info_recruit_time_key] = country->get_recruit_time().to_float();
	ret[military_info_recruit_time_tooltip_key] = base_value_percent_tooltip + _make_modifier_effect_contributions_tooltip(
		*country, *modifier_effect_cache.get_unit_recruitment_time()
	);

	ret[military_info_combat_width_key] = country->get_combat_width();
	{
		static const StringName base_value_combat_width_localisation_key = "COMWID_BASE";
		static const String val_replace_key = "$VAL$";
		String combat_width_tooltip = tr(base_value_combat_width_localisation_key).replace(
			val_replace_key, String::num_uint64(
				definition_manager.get_define_manager().get_military_defines().get_base_combat_width()
			)
		);
		const fixed_point_t combat_width = country->get_modifier_effect_value(
			*modifier_effect_cache.get_combat_width_additive()
		);
		if (combat_width != fixed_point_t::_0()) {
			combat_width_tooltip += from_technology_tooltip + GUILabel::get_colour_marker() + "G" +
				String::num_int64(combat_width.to_int64_t());
		}
		ret[military_info_combat_width_tooltip_key] = std::move(combat_width_tooltip);
	}

	ret[military_info_dig_in_cap_key] = country->get_dig_in_cap();
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
	// TODO - get mobilisation progress from SIM
	// ret[military_info_mobilisation_progress_key] = country->get_mobilisation_progress().to_float();
	ret[military_info_mobilisation_size_key] = static_cast<uint64_t>(country->get_mobilisation_potential_regiment_count());

	static const StringName mobilisation_size_tooltip_localisation_key = "MOB_SIZE_IRO";
	static const String mobilisation_size_tooltip_replace_value_key = "$VALUE$";

	ret[military_info_mobilisation_size_tooltip_key] = tr(mobilisation_size_tooltip_localisation_key).replace(
		mobilisation_size_tooltip_replace_value_key, Utilities::fixed_point_to_string_dp(
			country->get_modifier_effect_value(*modifier_effect_cache.get_mobilisation_size()) * 100, 2
		)
	) + _make_modifier_effect_contributions_tooltip(*country, *modifier_effect_cache.get_mobilisation_size());

	if (!country->is_mobilised()) {
		ret[military_info_mobilisation_impact_tooltip_key] = _make_mobilisation_impact_tooltip();
	}

	ret[military_info_mobilisation_economy_impact_key] = country->get_mobilisation_economy_impact().to_float();

	{
		fixed_point_t research_contribution;

		String mobilisation_economy_impact_tooltip = _make_modifier_effect_contributions_tooltip(
			*country, *modifier_effect_cache.get_mobilisation_economy_impact(), &research_contribution
		);

		if (research_contribution != fixed_point_t::_0()) {
			static const StringName research_contribution_negative_key = "MOB_ECO_IMPACT";
			static const StringName research_contribution_positive_key = "MOB_ECO_PENALTY";
			static const String replace_value_key = "$VALUE$";

			mobilisation_economy_impact_tooltip = tr(
				research_contribution < fixed_point_t::_0()
					? research_contribution_negative_key
					: research_contribution_positive_key
			).replace(
				replace_value_key, _make_modifier_effect_value(
					*modifier_effect_cache.get_mobilisation_economy_impact(), research_contribution.abs(), false
				)
			) + mobilisation_economy_impact_tooltip;
		} else if (!mobilisation_economy_impact_tooltip.is_empty()) {
			// Remove leading newline
			mobilisation_economy_impact_tooltip = mobilisation_economy_impact_tooltip.substr(1);
		}

		ret[military_info_mobilisation_economy_impact_tooltip_key] = mobilisation_economy_impact_tooltip;
	}

	// Leaders
	static const StringName military_info_general_count_key = "general_count";
	static const StringName military_info_admiral_count_key = "admiral_count";
	static const StringName military_info_create_leader_count_key = "create_leader_count";
	static const StringName military_info_create_leader_cost_key = "create_leader_cost";
	static const StringName military_info_auto_create_leaders_key = "auto_create_leaders";
	static const StringName military_info_auto_assign_leaders_key = "auto_assign_leaders";
	static const StringName military_info_leaders_list_key = "leaders_list";

	ret[military_info_general_count_key] = static_cast<uint64_t>(country->get_general_count());
	ret[military_info_admiral_count_key] = static_cast<uint64_t>(country->get_admiral_count());
	const uint64_t create_leader_count = country->get_create_leader_count();
	if (create_leader_count > 0) {
		ret[military_info_create_leader_count_key] = static_cast<uint64_t>(country->get_create_leader_count());
	} else {
		ret[military_info_create_leader_cost_key] =
			definition_manager.get_define_manager().get_military_defines().get_leader_recruit_cost().to_float();
	}
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

		if (leader_sort_key != LEADER_SORT_NONE) {
			const leader_sort_func_t leader_sort_func = _get_leader_sort_func(leader_sort_key);

			if (sort_leaders_descending) {
				std::sort(
					sorted_leaders.begin(), sorted_leaders.end(),
					[leader_sort_func](LeaderBase const* a, LeaderBase const* b) -> bool {
						return leader_sort_func(b, a);
					}
				);
			} else {
				std::sort(sorted_leaders.begin(), sorted_leaders.end(), leader_sort_func);
			}
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

	using enum UnitType::branch_t;

	if (country->has_armies()) {
		std::vector<ArmyInstance const*> sorted_armies;
		sorted_armies.reserve(country->get_army_count());
		for (ArmyInstance const* army : country->get_armies()) {
			sorted_armies.push_back(army);
		}

		if (army_sort_key != UNIT_GROUP_SORT_NONE) {
			const unit_group_sort_func_t<LAND> army_sort_func = _get_unit_group_sort_func<LAND>(army_sort_key);

			if (sort_armies_descending) {
				std::sort(
					sorted_armies.begin(), sorted_armies.end(),
					[army_sort_func](UnitInstanceGroup<LAND> const* a, UnitInstanceGroup<LAND> const* b) -> bool {
						return army_sort_func(b, a);
					}
				);
			} else {
				std::sort(sorted_armies.begin(), sorted_armies.end(), army_sort_func);
			}
		}

		TypedArray<Dictionary> armies;
		if (armies.resize(sorted_armies.size()) == OK) {

			for (size_t index = 0; index < sorted_armies.size(); ++index) {
				armies[index] = make_unit_group_dict(*sorted_armies[index]);
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

	{
		TypedArray<Dictionary> in_progress_brigades;

		in_progress_brigades.push_back(make_in_progress_unit_dict());

		ret[military_info_in_progress_brigades_key] = in_progress_brigades;
	}

	if (country->has_navies()) {
		std::vector<NavyInstance const*> sorted_navies;
		sorted_navies.reserve(country->get_navy_count());
		for (NavyInstance const* navy : country->get_navies()) {
			sorted_navies.push_back(navy);
		}

		if (navy_sort_key != UNIT_GROUP_SORT_NONE) {
			const unit_group_sort_func_t<NAVAL> navy_sort_func = _get_unit_group_sort_func<NAVAL>(navy_sort_key);

			if (sort_navies_descending) {
				std::sort(
					sorted_navies.begin(), sorted_navies.end(),
					[navy_sort_func](UnitInstanceGroup<NAVAL> const* a, UnitInstanceGroup<NAVAL> const* b) -> bool {
						return navy_sort_func(b, a);
					}
				);
			} else {
				std::sort(sorted_navies.begin(), sorted_navies.end(), navy_sort_func);
			}
		}

		TypedArray<Dictionary> navies;
		if (navies.resize(sorted_navies.size()) == OK) {

			for (size_t index = 0; index < sorted_navies.size(); ++index) {
				navies[index] = make_unit_group_dict(*sorted_navies[index]);
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

	{
		TypedArray<Dictionary> in_progress_ships;

		in_progress_ships.push_back(make_in_progress_unit_dict());

		ret[military_info_in_progress_ships_key] = in_progress_ships;
	}

	return ret;
}
