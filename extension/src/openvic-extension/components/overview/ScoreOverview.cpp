#include "ScoreOverview.hpp"

#include <cstdint>

#include <godot_cpp/variant/string.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/InstanceManager.hpp>
#include <openvic-simulation/modifier/ModifierEffectCache.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

ScoreOverview::ScoreOverview(GUINode const& parent) :
	flag_button{*parent.get_gui_masked_flag_button_from_nodepath("./topbar/player_flag")},
	flag_icon{*parent.get_gui_icon_from_nodepath("./topbar/topbar_flag_overlay")},
	name_label{*parent.get_gui_label_from_nodepath("./topbar/CountryName")},
	rank_label{*parent.get_gui_label_from_nodepath("./topbar/nation_totalrank")},
	prestige_label{*parent.get_gui_label_from_nodepath("./topbar/country_prestige")},
	prestige_rank_label{*parent.get_gui_label_from_nodepath("./topbar/selected_prestige_rank")},
	industrial_score_label{*parent.get_gui_label_from_nodepath("./topbar/country_economic")},
	industrial_rank_label{*parent.get_gui_label_from_nodepath("./topbar/selected_industry_rank")},
	military_score_label{*parent.get_gui_label_from_nodepath("./topbar/country_military")},
	military_rank_label{*parent.get_gui_label_from_nodepath("./topbar/selected_military_rank")},
	colonial_power_label{*parent.get_gui_label_from_nodepath("./topbar/country_colonial_power")}
{}

void ScoreOverview::update() {
	disconnect_all();
	GameSingleton& game_singleton = *GameSingleton::get_singleton();
	CountryInstance* country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	if (country_ptr == nullptr) {
		return;
	}
	CountryInstance& country = *country_ptr;

	const CountryInstance::country_status_t country_status = country.get_country_status();
	// 1 - Great Power
	// 2 - Secondary Power
	// 3 - Civilised
	// 4 - All Uncivilised
	uint8_t status_icon_index;
	godot::String country_status_translation_key;
	switch (country_status) {
		case CountryInstance::country_status_t::COUNTRY_STATUS_GREAT_POWER:
			country_status_translation_key="DIPLOMACY_GREATNATION_STATUS";
			status_icon_index = 1;
			break;
		case CountryInstance::country_status_t::COUNTRY_STATUS_SECONDARY_POWER:
			country_status_translation_key="DIPLOMACY_COLONIALNATION_STATUS";
			status_icon_index = 2;
			break;
		case CountryInstance::country_status_t::COUNTRY_STATUS_CIVILISED:
			country_status_translation_key="DIPLOMACY_CIVILIZEDNATION_STATUS";
			status_icon_index = 3;
			break;
		case CountryInstance::country_status_t::COUNTRY_STATUS_PARTIALLY_CIVILISED:
			country_status_translation_key="DIPLOMACY_ALMOST_WESTERN_NATION_STATUS";
			status_icon_index = 4;
			break;
		case CountryInstance::country_status_t::COUNTRY_STATUS_UNCIVILISED:
			country_status_translation_key="DIPLOMACY_UNCIVILIZEDNATION_STATUS";
			status_icon_index = 4;
			break;
		case CountryInstance::country_status_t::COUNTRY_STATUS_PRIMITIVE:
			country_status_translation_key="DIPLOMACY_PRIMITIVENATION_STATUS";
			status_icon_index = 4;
			break;
	}
	const godot::String country_name = Utilities::get_country_name(name_label, country);
	const godot::String country_name_rank_tooltip = Utilities::format(
		"%s%s%s",
	flag_button.tr("PLAYER_COUNTRY_TOPBAR_RANK")
			.replace("NAME", country_name)
			.replace("RANK", flag_button.tr(country_status_translation_key)),
		MenuSingleton::get_tooltip_separator(),
		flag_button.tr("RANK_TOTAL_D")
	);

	flag_button.set_flag_country(country_ptr);
	flag_button.set_tooltip_string(country_name_rank_tooltip);
	flag_icon.set_icon_index(status_icon_index);
	name_label.set_text(country_name);
	rank_label.set_text(godot::String::num_int64(country.get_total_rank()));
	rank_label.set_tooltip_string(country_name_rank_tooltip);

	prestige_rank_label.set_text(godot::String::num_int64(country.get_prestige_rank()));
	const godot::String prestige_tooltip = generate_prestige_tooltip(country);
	prestige_rank_label.set_tooltip_string(prestige_tooltip);
	prestige_label.set_tooltip_string(prestige_tooltip);
	on_prestige_changed(
		country.get_prestige(
			[this](signal<fixed_point_t>& prestige_changed) mutable ->void {
				prestige_changed.connect(&ScoreOverview::on_prestige_changed, this);
			}
		)
	);

	industrial_rank_label.set_text(godot::String::num_int64(country.get_industrial_rank()));
	const godot::String industrial_tooltip = generate_industrial_tooltip(country);
	industrial_rank_label.set_tooltip_string(industrial_tooltip);
	industrial_score_label.set_tooltip_string(industrial_tooltip);
	on_industrial_score_changed(
		country.get_industrial_power(
			[this](signal<fixed_point_t>& industrial_score_changed) mutable ->void {
				industrial_score_changed.connect(&ScoreOverview::on_industrial_score_changed, this);
			}
		)
	);

	military_rank_label.set_text(godot::String::num_int64(country.get_military_rank()));
	const godot::String military_tooltip = generate_military_tooltip(country);
	military_rank_label.set_tooltip_string(military_tooltip);
	military_score_label.set_tooltip_string(military_tooltip);
	update_military_score(
		country.military_power.get(
			[this](signal<>& military_score_changed) mutable ->void {
				military_score_changed.connect(&ScoreOverview::on_military_score_changed, this);
			}
		)
	);
	
	colonial_power_label.set_text("TODO");
	godot::String colonial_power_tooltip;
	if (!country.can_colonise()) {
		colonial_power_tooltip = Utilities::format(
			"%s%s%s",
			colonial_power_label.tr("COLONIAL_POINTS"),
			MenuSingleton::get_tooltip_separator(),
			colonial_power_label.tr("NON_COLONIAL_POWER")
		);
	} else {
		//TODO get data from sim

		const long available_colonial_power = 123;
		const long colonial_power_from_technology = 200;
		const fixed_point_t colonial_power_invested = 60;
		const fixed_point_t colonial_power_maintenance = 40;

		const godot::String available_part = colonial_power_label.tr("AVAILABLE_COLONIAL_POWER")
			.replace(
				Utilities::get_long_value_placeholder(),
				godot::String::num_int64(available_colonial_power)
			);

		InstanceManager const& instance_manager = *game_singleton.get_instance_manager();
		ModifierEffect const* const colonial_points_modifier = instance_manager
			.get_definition_manager()
			.get_modifier_manager()
			.get_modifier_effect_cache()
			.get_colonial_points();
		const godot::String from_tech_part = colonial_power_label.tr("FROM_TECHNOLOGY")+": "+ Utilities::make_modifier_effect_value_coloured(
			colonial_power_label,
			*colonial_points_modifier,
			colonial_power_from_technology,
			false
		);

		const godot::String investment_part = colonial_power_invested == 0
			? ""
			: "\n"+colonial_power_label.tr("COLONIAL_INVESTMENT")
				.replace(
					Utilities::get_long_value_placeholder(),
					Utilities::fixed_point_to_string_dp(colonial_power_invested, 1)
				);

		const godot::String maintenance_part = colonial_power_maintenance == 0
			? ""
			: "\n"+colonial_power_label.tr("COLONIAL_MAINTENANCE")
				.replace(
					Utilities::get_long_value_placeholder(),
					Utilities::fixed_point_to_string_dp(colonial_power_maintenance, 1)
				);

		colonial_power_tooltip = Utilities::format(
			"%s%s%s\n%s%s%s",
			colonial_power_label.tr("COLONIAL_POINTS"),
			MenuSingleton::get_tooltip_separator(),
			available_part,
			from_tech_part,
			investment_part,
			maintenance_part
		);
	}
	colonial_power_label.set_tooltip_string(colonial_power_tooltip);
	//TODO colonial tooltip
	// if _country_colonial_power_label:
	// 	var available_colonial_power : int = topbar_info.get(colonial_power_available_key, 0)
	// 	var max_colonial_power : int = topbar_info.get(colonial_power_max_key, 0)
	// 	_country_colonial_power_label.set_text(
	// 		"§%s%s§!/%s" % ["W" if available_colonial_power > 0 else "R", available_colonial_power, max_colonial_power]
	// 	)
	// 	_country_colonial_power_label.set_tooltip_string(tr(&"COLONIAL_POINTS") + MenuSingleton.get_tooltip_separator() + (
	// 		topbar_info.get(colonial_power_tooltip_key, "") if country_status <= CountryStatus.SECONDARY_POWER else tr(&"NON_COLONIAL_POWER")
	// 	))
}

godot::String ScoreOverview::generate_prestige_tooltip(CountryInstance& country) {
	// TODO - list prestige sources (e.g. power status)
	godot::String prestige_tooltip {};
	return prestige_label.tr("RANK_PRESTIGE")
		+ prestige_tooltip
		+ MenuSingleton::get_tooltip_separator()
		+ prestige_label.tr("RANK_PRESTIGE_D");
}

godot::String ScoreOverview::generate_industrial_tooltip(CountryInstance& country) {
	godot::String industrial_power_tooltip;

	// Pair: State name / Power
	std::vector<std::pair<godot::String, fixed_point_t>> industrial_power_states;
	for (auto const& [state, power] : country.get_industrial_power_from_states()) {
		industrial_power_states.emplace_back(
			Utilities::get_state_name(industrial_score_label, *state),
			power
		);
	}
	std::sort(
		industrial_power_states.begin(), industrial_power_states.end(),
		[](auto const& a, auto const& b) -> bool {
			// Sort by greatest power, then by state name alphabetically
			return a.second != b.second ? a.second > b.second : a.first < b.first;
		}
	);
	for (auto const& [state_name, power] : industrial_power_states) {
		static const godot::String state_power_template_string =
			"\n%s: " + GUILabel::get_colour_marker() + "Y%s" + GUILabel::get_colour_marker() + "!";

		industrial_power_tooltip += Utilities::format(
			state_power_template_string,
			state_name,
			Utilities::fixed_point_to_string_dp(power, 3)
		);
	}

	// Tuple: Country identifier / Country name / Power
	std::vector<std::tuple<godot::String, godot::String, fixed_point_t>> industrial_power_from_investments;
	for (auto const& [country, power] : country.get_industrial_power_from_investments()) {
		industrial_power_from_investments.emplace_back(
			Utilities::std_to_godot_string(country->get_identifier()),
			Utilities::get_country_name(industrial_score_label, *country),
			power
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
		static const godot::String investment_power_template_string = "\n" + GUILabel::get_flag_marker() + "%s %s: " +
			GUILabel::get_colour_marker() + "Y%s" + GUILabel::get_colour_marker() + "!";

		industrial_power_tooltip += Utilities::format(
			investment_power_template_string,
			country_identifier,
			country_name,
			Utilities::fixed_point_to_string_dp(power, 3)
		);
	}

	return industrial_score_label.tr("RANK_INDUSTRY")
		+ MenuSingleton::get_tooltip_separator()
		+ industrial_score_label.tr("RANK_INDUSTRY_D")
		+ industrial_power_tooltip;
}

void ScoreOverview::on_prestige_changed(const fixed_point_t new_prestige) {
	prestige_label.set_text(
		Utilities::fixed_point_to_string_dp(new_prestige, 0)
	);
}
void ScoreOverview::on_industrial_score_changed(const fixed_point_t new_industrial_score) {
	industrial_score_label.set_text(
		Utilities::fixed_point_to_string_dp(new_industrial_score, 0)
	);
}

godot::String ScoreOverview::generate_military_tooltip(CountryInstance& country) {
	godot::String military_power_tooltip;

	const godot::StringName military_power_from_land_key = OV_INAME("MIL_FROM_TROOPS");
	const godot::StringName military_power_from_sea_key = OV_INAME("MIL_FROM_CAP_SHIPS");
	const godot::StringName military_power_from_leaders_key = OV_INAME("MIL_FROM_LEADERS");

	for (auto const& [source, power] : {
		std::pair
		{ military_power_from_land_key, country.get_military_power_from_land_untracked() },
		{ military_power_from_sea_key, country.get_military_power_from_sea_untracked() },
		{ military_power_from_leaders_key, country.get_military_power_from_leaders_untracked() }
	}) {
		if (power != 0) {
			military_power_tooltip += "\n" + military_score_label.tr(source) + ": " + GUILabel::get_colour_marker() + "Y"
				+ Utilities::fixed_point_to_string_dp(power, 3) + GUILabel::get_colour_marker() + "!";
		}
	}

	return military_score_label.tr("RANK_MILITARY")
		+ MenuSingleton::get_tooltip_separator()
		+ military_score_label.tr("RANK_MILITARY_D")
		+ military_power_tooltip;
}
void ScoreOverview::on_military_score_changed() {
	CountryInstance* country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	if (country_ptr == nullptr) {
		return;
	}
	CountryInstance& country = *country_ptr;
	update_military_score(country.military_power.get_untracked());
	
}
void ScoreOverview::update_military_score(const fixed_point_t new_military_score) {
	military_score_label.set_text(
		Utilities::fixed_point_to_string_dp(new_military_score, 0)
	);
}