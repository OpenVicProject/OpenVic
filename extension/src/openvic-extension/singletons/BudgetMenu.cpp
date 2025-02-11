#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

Dictionary MenuSingleton::get_budget_menu_setup_info(GUIScrollbar* tariff_slider) const {
	static const StringName stratas_key = "stratas";
	static const StringName pop_types_by_strata_key = "pop_types_by_strata";
	static const StringName pop_sprites_by_type_key = "pop_sprites_by_type";
	static const StringName education_spending_pop_types_key = "education_spending_pop_types";
	static const StringName administration_spending_pop_types_key = "administration_spending_pop_types";
	// There are no social spending pop types even though an overlapping elements box for them exists in the GUI file
	// Non-administrative social_reforms reform group names
	static const StringName social_spending_subcategories_key = "social_spending_subcategories";
	static const StringName military_spending_pop_types_key = "military_spending_pop_types";
	// Military spending pop type names
	static const StringName military_spending_subcategories_key = "military_spending_subcategories";

	// TODO - save the social and military spending subcategory ReformGroups and PopTypes for easier future lookup?

	DefinitionManager const& definition_manager = GameSingleton::get_singleton()->get_definition_manager();
	PopManager const& pop_manager = definition_manager.get_pop_manager();

	if (tariff_slider != nullptr) {
		tariff_slider->set_step_size_and_limits_fp(
			fixed_point_t::_1(),
			-fixed_point_t::_1() * SLIDER_SCALE,
			fixed_point_t::_1() * SLIDER_SCALE
		);
	}

	Dictionary dict;

	{
		PackedStringArray stratas;
		TypedArray<PackedByteArray> pop_types_by_strata;

		for (Strata const& strata : pop_manager.get_stratas()) {
			stratas.push_back(Utilities::std_to_godot_string(strata.get_identifier()));

			PackedByteArray pop_types;

			for (PopType const* pop_type : strata.get_pop_types()) {
				pop_types.push_back(pop_type->get_index());
			}

			pop_types_by_strata.push_back(pop_types);
		}

		dict[stratas_key] = std::move(stratas);
		dict[pop_types_by_strata_key] = std::move(pop_types_by_strata);
	}

	{
		using enum PopType::income_type_t;

		PackedByteArray pop_sprites_by_type;
		PackedByteArray education_spending_pop_types;
		PackedByteArray administration_spending_pop_types;
		PackedByteArray military_spending_pop_types;
		PackedStringArray military_spending_subcategories;

		for (PopType const& pop_type : pop_manager.get_pop_types()) {
			pop_sprites_by_type.push_back(pop_type.get_sprite());

			if (pop_type.has_income_type(EDUCATION)) {
				education_spending_pop_types.push_back(pop_type.get_index());
			}

			if (pop_type.has_income_type(ADMINISTRATION)) {
				administration_spending_pop_types.push_back(pop_type.get_index());
			}

			if (pop_type.has_income_type(MILITARY)) {
				military_spending_pop_types.push_back(pop_type.get_index());
				military_spending_subcategories.push_back(Utilities::std_to_godot_string(pop_type.get_identifier()));
			}
		}

		dict[pop_sprites_by_type_key] = std::move(pop_sprites_by_type);
		dict[education_spending_pop_types_key] = std::move(education_spending_pop_types);
		dict[administration_spending_pop_types_key] = std::move(administration_spending_pop_types);
		dict[military_spending_pop_types_key] = std::move(military_spending_pop_types);
		dict[military_spending_subcategories_key] = std::move(military_spending_subcategories);
	}

	{
		static constexpr std::string_view social_reforms_identifier = "social_reforms";

		ReformType const* social_reforms =
			definition_manager.get_politics_manager().get_issue_manager().get_reform_type_by_identifier(
				social_reforms_identifier
			);

		if (social_reforms != nullptr) {
			PackedStringArray social_spending_subcategories;

			for (ReformGroup const* reform_group : social_reforms->get_reform_groups()) {
				if (!reform_group->is_administrative()) {
					social_spending_subcategories.push_back(Utilities::std_to_godot_string(reform_group->get_identifier()));
				}
			}

			dict[social_spending_subcategories_key] = std::move(social_spending_subcategories);
		}
	}

	return dict;
}

Dictionary MenuSingleton::get_budget_menu_info(
	TypedArray<GUIScrollbar> const& strata_tax_sliders,
	GUIScrollbar* land_spending_slider,
	GUIScrollbar* naval_spending_slider,
	GUIScrollbar* construction_spending_slider,
	GUIScrollbar* education_spending_slider,
	GUIScrollbar* administration_spending_slider,
	GUIScrollbar* social_spending_slider,
	GUIScrollbar* military_spending_slider,
	GUIScrollbar* tariff_slider
) const {
	CountryInstance const* country = PlayerSingleton::get_singleton()->get_player_country();
	if (country == nullptr) {
		return {};
	}

	static const StringName no_needs_localisation_key = "NO_NEED";
	static const StringName some_life_needs_localisation_key = "SOME_LIFE_NEEDS";
	static const StringName life_needs_localisation_key = "LIFE_NEEDS";
	static const StringName everyday_needs_localisation_key = "EVERYDAY_NEEDS";
	static const StringName luxury_needs_localisation_key = "LUXURY_NEEDS";

	Dictionary dict;

	{
		static const StringName pop_type_needs_tooltips_key = "pop_type_needs_tooltips";
		static const StringName pop_type_present_bools_key = "pop_type_present_bools";

		IndexedMap<PopType, pop_size_t> const& pop_type_distribution = country->get_pop_type_distribution();

		PackedStringArray pop_type_needs_tooltips;
		PackedByteArray pop_type_present_bools;

		if (
			pop_type_needs_tooltips.resize(pop_type_distribution.size()) == OK &&
			pop_type_present_bools.resize(pop_type_distribution.size()) == OK
		) {
			static const StringName no_pops_localisation_key = "NO_POPS_OF_TYPE";
			static const StringName getting_needs_localisation_key = "GETTING_NEEDS";
			static const StringName getting_only_needs_localisation_key = "GETTING_ONLY_NEEDS";
			static const String need_replace_key = "$NEED$";
			static const String val_replace_key = "$VAL$";

			for (int64_t index = 0; index < pop_type_distribution.size(); ++index) {
				PopType const& pop_type = pop_type_distribution(index);

				String tooltip = tr(Utilities::std_to_godot_string(pop_type.get_identifier())) + get_tooltip_separator();
				const bool present = pop_type_distribution[index] > 0;

				if (present) {
					const String getting_needs = tr(getting_needs_localisation_key);
					const String getting_only_needs = tr(getting_only_needs_localisation_key) + "\n";

					// TODO - replace with actual needs breakdown by pop type
					const fixed_point_t no_needs_proportion = fixed_point_t::parse(1) / 16;
					const fixed_point_t only_partial_life_needs_proportion = fixed_point_t::parse(7) / 16;
					const fixed_point_t only_life_needs_proportion = fixed_point_t::parse(4) / 16;
					const fixed_point_t only_everyday_needs_proportion = fixed_point_t::parse(3) / 16;
					const fixed_point_t luxury_needs_proportion = fixed_point_t::parse(1) / 16;

					tooltip += getting_needs.replace(
						need_replace_key, tr(no_needs_localisation_key)
					).replace(
						val_replace_key, Utilities::fixed_point_to_string_dp(no_needs_proportion * 100, 1)
					) + "\n" + getting_only_needs.replace(
						need_replace_key, tr(some_life_needs_localisation_key)
					).replace(
						val_replace_key, Utilities::fixed_point_to_string_dp(only_partial_life_needs_proportion * 100, 1)
					) + getting_only_needs.replace(
						need_replace_key, tr(life_needs_localisation_key)
					).replace(
						val_replace_key, Utilities::fixed_point_to_string_dp(only_life_needs_proportion * 100, 1)
					) + getting_only_needs.replace(
						need_replace_key, tr(everyday_needs_localisation_key)
					).replace(
						val_replace_key, Utilities::fixed_point_to_string_dp(only_everyday_needs_proportion * 100, 1)
					) + getting_needs.replace(
						need_replace_key, tr(luxury_needs_localisation_key)
					).replace(
						val_replace_key, Utilities::fixed_point_to_string_dp(luxury_needs_proportion * 100, 1)
					);
				} else {
					tooltip += tr(no_pops_localisation_key);
				}

				pop_type_needs_tooltips[index] = std::move(tooltip);
				pop_type_present_bools[index] = present;
			}

			dict[pop_type_needs_tooltips_key] = std::move(pop_type_needs_tooltips);
			dict[pop_type_present_bools_key] = std::move(pop_type_present_bools);
		} else {
			UtilityFunctions::push_error("Failed to resize pop type needs tooltips and present bools arrays!");
		}
	}

	{
		// Tax

		IndexedMap<Strata, SliderValue> const& tax_rate_by_strata = country->get_tax_rate_slider_value_by_strata();
		const int64_t strata_tax_slider_count = std::min(
			strata_tax_sliders.size(), static_cast<int64_t>(tax_rate_by_strata.size())
		);
		for (int64_t index = 0; index < strata_tax_slider_count; ++index) {
			GUIScrollbar* strata_tax_slider = Object::cast_to<GUIScrollbar>(strata_tax_sliders[index]);

			if (strata_tax_slider != nullptr) {
				strata_tax_slider->set_range_limits_and_value_from_slider_value(
					tax_rate_by_strata[index], SLIDER_SCALE, false
				);

				// TODO - tooltip
			}
		}

		static const StringName tax_info_by_strata_key = "tax_info_by_strata";

		TypedArray<Dictionary> tax_info_by_strata;
		if (tax_info_by_strata.resize(tax_rate_by_strata.size()) == OK) {
			static const StringName strata_needs_pie_chart_key = "strata_needs_pie_chart";
			static const StringName strata_tax_value_key = "strata_tax_value";
			static const StringName strata_tax_value_tooltip_key = "strata_tax_value_tooltip";

			// BUDGET_STRATA_NO_NEED;�Y$VAL$�W% are getting none of their needs.;
			// BUDGET_STRATA_NEED;�Y$VAL$�W% are getting their $TYPE$;
			static const StringName getting_no_needs_localisation_key = "BUDGET_STRATA_NO_NEED";
			static const StringName getting_needs_localisation_key = "BUDGET_STRATA_NEED";
			static const String type_replace_key = "$TYPE$";
			static const String val_replace_key = "$VAL$";

			for (int64_t index = 0; index < tax_rate_by_strata.size(); ++index) {
				Strata const& strata = tax_rate_by_strata(index);

				Dictionary strata_dict;



				tax_info_by_strata[index] = std::move(strata_dict);
			}

			dict[tax_info_by_strata_key] = std::move(tax_info_by_strata);
		} else {
			UtilityFunctions::push_error("Failed to resize tax info by strata array!");
		}
	}

	// Gold
	static const StringName gold_key = "gold";
	static const StringName gold_tooltip_key = "gold_tooltip";

	// Total Income
	static const StringName total_income_key = "total_income";
	static const StringName total_income_tooltip_key = "total_income_tooltip";

	// Funds
	static const StringName national_bank_key = "national_bank";
	static const StringName national_bank_tooltip_key = "national_bank_tooltip";
	static const StringName total_funds_key = "total_funds";

	// Debt
	static const StringName total_debt_key = "total_debt";
	static const StringName interest_key = "interest";
	static const StringName repaying_debts_key = "repaying_debts";
	// TODO - should these be dictionaries from country name (or "SHADOWY_INVESTOR") to amount?
	// Also needed for pie charts. Table version has flag prefix, but not pie chart tooltips ("name" + get_tooltip_separator() + "amount£")
	static const StringName loans_taken_key = "loans_taken";
	static const StringName loans_given_key = "loans_given";

	// Industrial Subsidies
	static const StringName industrial_subsidies_key = "industrial_subsidies";
	static const StringName industrial_subsidies_tooltip_key = "industrial_subsidies_tooltip";

	// National Stockpile
	static const StringName military_costs_key = "military_costs";
	static const StringName military_costs_tooltip_key = "military_costs_tooltip";
	static const StringName overseas_maintenance_key = "overseas_maintenance";
	static const StringName overseas_maintenance_tooltip_key = "overseas_maintenance_tooltip";
	static const StringName national_stockpile_today_key = "national_stockpile_today";
	static const StringName national_stockpile_today_tooltip_key = "national_stockpile_today_tooltip";
	static const StringName national_stockpile_tomorrow_key = "national_stockpile_tomorrow";
	static const StringName national_stockpile_tomorrow_tooltip_key = "national_stockpile_tomorrow_tooltip";
	if (land_spending_slider != nullptr) {
		land_spending_slider->set_range_limits_and_value_from_slider_value(
			country->get_land_spending_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}
	if (naval_spending_slider != nullptr) {
		naval_spending_slider->set_range_limits_and_value_from_slider_value(
			country->get_naval_spending_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}
	if (construction_spending_slider != nullptr) {
		construction_spending_slider->set_range_limits_and_value_from_slider_value(
			country->get_construction_spending_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}

	// Education
	if (education_spending_slider != nullptr) {
		education_spending_slider->set_value_from_slider_value(
			country->get_education_spending_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}
	static const StringName education_spending_value_key = "education_spending_value";

	// Administration
	static const StringName administrative_efficiency_key = "administrative_efficiency";
	static const StringName administrative_efficiency_tooltip_key = "administrative_efficiency_tooltip";
	if (administration_spending_slider != nullptr) {
		administration_spending_slider->set_value_from_slider_value(
			country->get_administration_spending_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}
	static const StringName administration_spending_value_key = "administration_spending_value";

	// Social Spending
	if (social_spending_slider != nullptr) {
		social_spending_slider->set_range_limits_and_value_from_slider_value(
			country->get_social_spending_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}
	static const StringName social_spending_value_key = "social_spending_value";
	static const StringName social_spending_subcategories_key = "social_spending_subcategories";

	// Miltiary Spending
	if (military_spending_slider != nullptr) {
		military_spending_slider->set_range_limits_and_value_from_slider_value(
			country->get_military_spending_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}
	static const StringName military_spending_value_key = "military_spending_value";
	static const StringName military_spending_subcategories_key = "military_spending_subcategories";

	// Total Expense
	static const StringName total_expense_key = "total_expense";
	static const StringName total_expense_tooltip_key = "total_expense_tooltip";

	// Tariffs
	if (tariff_slider != nullptr) {
		tariff_slider->set_range_limits_and_value_from_slider_value(
			country->get_tariff_rate_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}
	static const StringName tariff_value_key = "tariff_value";

	// Diplomatic Balance
	static const StringName diplomatic_balance_key = "diplomatic_balance";
	static const StringName diplomatic_balance_tooltip_key = "diplomatic_balance_tooltip";

	// Projected Daily Balance
	static const StringName projected_daily_balance_key = "projected_daily_balance";

	return dict;
}
