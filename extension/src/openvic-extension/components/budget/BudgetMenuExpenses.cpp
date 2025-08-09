#include "BudgetMenuExpenses.hpp"

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/components/budget/DiplomaticBudget.hpp"
#include "openvic-extension/components/budget/TariffBudget.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

BudgetMenuExpenses::BudgetMenuExpenses(
	GUINode const& parent,
	CountryDefines const& country_defines,
	DiplomaticBudget& new_diplomatic_budget,
	TariffBudget& new_tariff_budget	
) : projected_expenses_label{*parent.get_gui_label_from_nodepath("./country_budget/total_exp")},
	administration_budget{parent,country_defines},
	diplomatic_budget{new_diplomatic_budget},
	education_budget{parent},
	military_budget{parent},
	national_stockpile_budget{parent},
	social_budget{parent},
	tariff_budget{new_tariff_budget}
	{		
		GUILabel::set_text_and_tooltip(
			parent, "./country_budget/ind_sub_desc",
			"BUDGET_INDUSTRIAL_SUBSIDIES","IND_SUP_DESC"
		);
		projected_expenses_args.resize(8);
	}

void BudgetMenuExpenses::update() {
	static const godot::StringName projected_expenses_template = "%s\n--------------\n%s\n%s\n%s\n%s\n%s\n%s%s";

	const fixed_point_t administration_expenses = administration_budget.get_balance(connect_property_to_mark_dirty());
	const fixed_point_t diplomatic_expenses = diplomatic_budget.get_balance(connect_property_to_mark_dirty());
	const fixed_point_t education_expenses = education_budget.get_balance(connect_property_to_mark_dirty());
	const fixed_point_t military_expenses = military_budget.get_balance(connect_property_to_mark_dirty());
	const fixed_point_t national_stockpile_expenses= national_stockpile_budget.get_balance(connect_property_to_mark_dirty());
	const fixed_point_t social_expenses = social_budget.get_balance(connect_property_to_mark_dirty());
	const fixed_point_t tariff_expenses = -std::min(
		tariff_budget.get_balance(connect_property_to_mark_dirty()),
		fixed_point_t::_0
	);
	const fixed_point_t projected_expenses_total =  administration_expenses
		+ diplomatic_expenses
		+ education_expenses
		+ military_expenses
		+ national_stockpile_expenses
		+ social_expenses
		+ tariff_expenses;
		//TODO: + import subsidies?
		//TODO: + factory subsidies
		//TODO: + interest
	const fixed_point_t interest_expenses = 0;

	projected_expenses_label.set_text(
		Utilities::cash_to_string_dp_dynamic(projected_expenses_total)
	);

	projected_expenses_args[0] = projected_expenses_label.tr("BUDGET_TOTAL_EXPENSE").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(projected_expenses_total, 3)
	);
	projected_expenses_args[1] = education_budget.generate_expenses_summary_text(
		education_expenses,
		projected_expenses_label
	);
	projected_expenses_args[2] = administration_budget.generate_expenses_summary_text(
		administration_expenses,
		projected_expenses_label
	);
	projected_expenses_args[3] = social_budget.generate_expenses_summary_text(
		social_expenses,
		projected_expenses_label
	);
	projected_expenses_args[4] = military_budget.generate_expenses_summary_text(
		military_expenses,
		projected_expenses_label
	);
	projected_expenses_args[5] = projected_expenses_label.tr("BUDGET_INTEREST").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(interest_expenses, 1)
	);
	projected_expenses_args[6] = national_stockpile_budget.generate_expenses_summary_text(
		national_stockpile_expenses,
		projected_expenses_label
	);
	projected_expenses_args[7] = diplomatic_budget.generate_expenses_summary_text(
		diplomatic_expenses,
		projected_expenses_label
	);
	projected_expenses_label.set_tooltip_string(
		projected_expenses_template % projected_expenses_args
	);
	
	_projected_expenses = projected_expenses_total;
}