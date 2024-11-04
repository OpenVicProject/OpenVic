#include "MenuSingleton.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "openvic-simulation/research/Technology.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

/* TECHNOLOGY MENU */
Dictionary MenuSingleton::get_technology_menu_defines() const {
	static const StringName tech_folders_key = "tech_folders";
	static const StringName tech_areas_key = "tech_areas";
	static const StringName technologies_key = "technologies";
	static const StringName folder_tech_count_key = "folder_tech_count";

	Dictionary ret;

	std::vector<OpenVic::TechnologyFolder> const& tech_folders = GameSingleton::get_singleton()->get_definition_manager()
		.get_research_manager().get_technology_manager().get_technology_folders();

	PackedStringArray tech_folder_identifiers {};
	TypedArray<PackedStringArray> tech_area_identifiers {};
	Array tech_identifiers {};
	PackedInt32Array folder_tech_count {};
	for (TechnologyFolder const& folder : tech_folders) {
		tech_folder_identifiers.push_back(Utilities::std_to_godot_string(folder.get_identifier()));
		int32_t num_in_folder = 0;

		PackedStringArray folder_areas {};
		Array tech_folder_nested_array {}; // tech_identifiers has three levels of nested arrays :P
		for (TechnologyArea const* area : folder.get_technology_areas()) {
			folder_areas.push_back(Utilities::std_to_godot_string(area->get_identifier()));

			PackedStringArray area_technologies {};
			for (Technology const* tech : area->get_technologies()) {
				area_technologies.push_back(Utilities::std_to_godot_string(tech->get_identifier()));
				num_in_folder++;
			}
			tech_folder_nested_array.push_back(std::move(area_technologies));
		}
		tech_area_identifiers.push_back(std::move(folder_areas));
		tech_identifiers.push_back(std::move(tech_folder_nested_array));
		folder_tech_count.push_back(num_in_folder);
	}
	ret[tech_folders_key] = std::move(tech_folder_identifiers);
	ret[tech_areas_key] = std::move(tech_area_identifiers);
	ret[technologies_key] = std::move(tech_identifiers);
	ret[folder_tech_count_key] = std::move(folder_tech_count);

	return ret;
}

Dictionary MenuSingleton::get_technology_menu_info() const {
	GameSingleton const& game_singleton = *GameSingleton::get_singleton();
	DefinitionManager const& definition_manager = game_singleton.get_definition_manager();
	InstanceManager const* instance_manager = game_singleton.get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	TechnologyManager const& tech_manager = definition_manager.get_research_manager().get_technology_manager();

	static const StringName tech_school_key = "tech_school";
	static const StringName tech_school_mod_values = "tech_school_mod_values";
	static const StringName tech_school_mod_icons = "tech_school_mod_icons";
	static const StringName tech_school_mod_tt = "tech_school_mod_tt";

	static const StringName researched_technologies_key = "researched_technologies";
	static const StringName researchable_technologies_key = "researchable_technologies";

	static const StringName current_research_tech = "current_research_tech";
	static const StringName current_research_cat = "current_research_cat";
	static const StringName current_research_finish_date = "current_research_finish_date";
	static const StringName current_research_invested = "current_research_invested";
	static const StringName current_research_cost = "current_research_cost";
	static const StringName current_research_effect_tt = "current_research_effect_tt";
	static const StringName current_research_progress = "current_research_progress";

	Dictionary ret;

	CountryInstance const* country = PlayerSingleton::get_singleton()->get_player_country();
	if (country == nullptr) {
		return ret;
	}

		TechnologySchool const* tech_school = country->get_tech_school();
	if (tech_school == nullptr) {
		tech_school = tech_manager.get_technology_school_by_index(0);
	}
	if (tech_school != nullptr) {
		ret[tech_school_key] = Utilities::std_to_godot_string(tech_school->get_identifier());
		PackedFloat32Array school_modifier_values;
		PackedInt32Array school_modifier_icons;
		PackedStringArray school_modifier_tt;
		for (
			auto [tech_folder, research_bonus_effect] :
				definition_manager.get_modifier_manager().get_modifier_effect_cache().get_research_bonus_effects()
		) {
			const fixed_point_t research_bonus_value = tech_school->get_effect(*research_bonus_effect);
			if (research_bonus_value != fixed_point_t::_0()) {
				school_modifier_values.push_back(research_bonus_value.to_float());
				school_modifier_icons.push_back(1 + tech_folder.get_index());
				school_modifier_tt.push_back(_make_modifier_effect_tooltip(*research_bonus_effect, research_bonus_value));
			}
		}
		ret[tech_school_mod_values] = std::move(school_modifier_values);
		ret[tech_school_mod_icons] = std::move(school_modifier_icons);
		ret[tech_school_mod_tt] = std::move(school_modifier_tt);
	}

	PackedStringArray researched_technologies {};
	PackedStringArray researchable_technologies {};
	for (Technology const& tech : tech_manager.get_technologies()) {
		if (country->is_technology_unlocked(tech)) {
			researched_technologies.push_back(Utilities::std_to_godot_string(tech.get_identifier()));
		}
		if (country->can_research_tech(tech, instance_manager->get_today())) {
			researchable_technologies.push_back(Utilities::std_to_godot_string(tech.get_identifier()));
		}
	}
	ret[researched_technologies_key] = std::move(researched_technologies);
	ret[researchable_technologies_key] = std::move(researchable_technologies);

	Technology const* current_research = country->get_current_research();
	if (current_research != nullptr) {
		ret[current_research_tech] = Utilities::std_to_godot_string(current_research->get_identifier());
		ret[current_research_cat] =
			tr(Utilities::std_to_godot_string(current_research->get_area().get_folder().get_identifier())) + ", " +
			tr(Utilities::std_to_godot_string(current_research->get_area().get_identifier()));
		ret[current_research_finish_date] = Utilities::date_to_string(country->get_expected_research_completion_date());
		ret[current_research_invested] = country->get_invested_research_points().to_int32_t();
		ret[current_research_cost] = country->get_current_research_cost().to_int32_t();
		ret[current_research_effect_tt] = _make_modifier_effects_tooltip(*current_research).trim_prefix("\n");
		ret[current_research_progress] = country->get_research_progress().to_float();
	}

	return ret;
}

Dictionary MenuSingleton::get_specific_technology_info(String technology_identifier) const {
	DefinitionManager const& definition_manager = GameSingleton::get_singleton()->get_definition_manager();

	static const StringName effect_tooltip_key = "effects";
	static const StringName research_points_key = "research_points";
	static const StringName start_year_key = "start_year";
	static const StringName prerequisite_key = "prerequisite";

	Dictionary ret;

	Technology const* technology =
		definition_manager.get_research_manager().get_technology_manager().get_technology_by_identifier(
			Utilities::godot_to_std_string(technology_identifier)
		);
	CountryInstance const* player_country = PlayerSingleton::get_singleton()->get_player_country();
	if (technology == nullptr || player_country == nullptr) {
		return ret;
	}

	ret[effect_tooltip_key] = _make_modifier_effects_tooltip(*technology).trim_prefix("\n");
	ret[research_points_key] = player_country->calculate_research_cost(
		*technology, definition_manager.get_modifier_manager().get_modifier_effect_cache()
	).to_int32_t();
	ret[start_year_key] = technology->get_year();
	if (technology->get_index_in_area() > 0) {
		ret[prerequisite_key] = Utilities::std_to_godot_string(
			technology->get_area().get_technologies()[technology->get_index_in_area() - 1]->get_identifier()
		);
	}

	return ret;
}
