#include "MenuSingleton.hpp"

#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

Dictionary MenuSingleton::get_budget_menu_setup_info() const {
	static const StringName pop_sprites_by_type_key = "pop_sprites_by_type";
	static const StringName pop_types_by_strata_key = "pop_types_by_strata";
	static const StringName education_pop_types_key = "education_pop_types";
	static const StringName administration_pop_types_key = "administration_pop_types";
	// There are no social spending pop types even though an overlapping elements box for them exists in the GUI file
	// Non-administrative social_reforms reform group names
	static const StringName social_spending_subcategories_key = "social_spending_subcategories";
	static const StringName military_spending_pop_types_key = "military_spending_pop_types";
	// Military spending pop type names
	static const StringName military_spending_subcategories_key = "military_spending_subcategories";

	// TODO - save the social and military spending subcategory ReformGroups and PopTypes for easier future lookup?

	DefinitionManager const& definition_manager = GameSingleton::get_singleton()->get_definition_manager();
	PopManager const& pop_manager = definition_manager.get_pop_manager();

	Dictionary dict;

	{
		using enum PopType::income_type_t;

		PackedByteArray pop_sprites_by_type;
		PackedByteArray education_pop_types;
		PackedByteArray administration_pop_types;
		PackedByteArray military_spending_pop_types;
		PackedStringArray military_spending_subcategories;

		for (PopType const& pop_type : pop_manager.get_pop_types()) {
			pop_sprites_by_type.push_back(pop_type.get_sprite());

			if (pop_type.has_income_type(EDUCATION)) {
				education_pop_types.push_back(pop_type.get_index());
			}

			if (pop_type.has_income_type(ADMINISTRATION)) {
				administration_pop_types.push_back(pop_type.get_index());
			}

			if (pop_type.has_income_type(MILITARY)) {
				military_spending_pop_types.push_back(pop_type.get_index());
				military_spending_subcategories.push_back(Utilities::std_to_godot_string(pop_type.get_identifier()));
			}
		}

		dict[pop_sprites_by_type_key] = std::move(pop_sprites_by_type);
		dict[education_pop_types_key] = std::move(education_pop_types);
		dict[administration_pop_types_key] = std::move(administration_pop_types);
		dict[military_spending_pop_types_key] = std::move(military_spending_pop_types);
		dict[military_spending_subcategories_key] = std::move(military_spending_subcategories);
	}

	{
		TypedArray<PackedByteArray> pop_types_by_strata;

		for (Strata const& strata : pop_manager.get_stratas()) {
			PackedByteArray pop_types;

			for (PopType const* pop_type : strata.get_pop_types()) {
				pop_types.push_back(pop_type->get_index());
			}

			pop_types_by_strata.push_back(pop_types);
		}

		dict[pop_types_by_strata_key] = std::move(pop_types_by_strata);
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

Dictionary MenuSingleton::get_budget_menu_info() const {
	static const StringName pop_type_needs_tooltips_key = "pop_type_needs_tooltips";
	static const StringName pop_type_present_bools_key = "pop_type_present_bools";

	// Tax
	static const StringName tax_info_by_strata_key = "tax_info_by_strata";
	// Per strata:
	static const StringName strata_needs_pie_chart_key = "strata_needs_pie_chart";
	static const StringName strata_tax_slider_key = "strata_tax_slider";
	static const StringName strata_tax_slider_tooltip_key = "strata_tax_slider_tooltip";
	static const StringName strata_tax_value_key = "strata_tax_value";
	static const StringName strata_tax_value_tooltip_key = "strata_tax_value_tooltip";

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
	static const StringName land_spending_slider_key = "land_spending_slider";
	static const StringName land_spending_slider_tooltip_key = "land_spending_slider_tooltip";
	static const StringName naval_spending_slider_key = "naval_spending_slider";
	static const StringName naval_spending_slider_tooltip_key = "naval_spending_slider_tooltip";
	static const StringName construction_spending_slider_key = "construction_spending_slider";
	static const StringName construction_spending_slider_tooltip_key = "construction_spending_slider_tooltip";

	// Education
	static const StringName education_spending_slider_key = "education_spending_slider";
	static const StringName education_spending_slider_tooltip_key = "education_spending_slider_tooltip";
	static const StringName education_spending_value_key = "education_spending_value";

	// Administration
	static const StringName administrative_efficiency_key = "administrative_efficiency";
	static const StringName administrative_efficiency_tooltip_key = "administrative_efficiency_tooltip";
	static const StringName administration_spending_slider_key = "administration_spending_slider";
	static const StringName administration_spending_slider_tooltip_key = "administration_spending_slider_tooltip";
	static const StringName administration_spending_value_key = "administration_spending_value";

	// Social Spending
	static const StringName social_spending_slider_key = "social_spending_slider";
	static const StringName social_spending_slider_tooltip_key = "social_spending_slider_tooltip";
	static const StringName social_spending_value_key = "social_spending_value";
	static const StringName social_spending_subcategories_key = "social_spending_subcategories";

	// Miltiary Spending
	static const StringName military_spending_slider_key = "military_spending_slider";
	static const StringName military_spending_slider_tooltip_key = "military_spending_slider_tooltip";
	static const StringName military_spending_value_key = "military_spending_value";
	static const StringName military_spending_subcategories_key = "military_spending_subcategories";

	// Total Expense
	static const StringName total_expense_key = "total_expense";
	static const StringName total_expense_tooltip_key = "total_expense_tooltip";

	// Tariffs
	static const StringName tariff_slider_key = "tariff_slider";
	static const StringName tariff_slider_tooltip_key = "tariff_slider_tooltip";
	static const StringName tariff_value_key = "tariff_value";

	// Diplomatic Balance
	static const StringName diplomatic_balance_key = "diplomatic_balance";
	static const StringName diplomatic_balance_tooltip_key = "diplomatic_balance_tooltip";

	// Projected Daily Balance
	static const StringName projected_daily_balance_key = "projected_daily_balance";

	// slider data uses Vector3i (min, value, max)

	Dictionary dict;

	return dict;
}
