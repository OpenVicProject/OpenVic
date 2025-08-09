#include "NationalStockpileBudget.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

NationalStockpileBudget::NationalStockpileBudget(GUINode const& parent):
	BudgetExpenseComponent("BUDGET_IMPORTS"),
	military_costs_label{*parent.get_gui_label_from_nodepath("./country_budget/mil_cost_val")},
	todays_actual_stockpile_spending_label{*parent.get_gui_label_from_nodepath("./country_budget/nat_stock_val")},
	overseas_costs_label{*parent.get_gui_label_from_nodepath("./country_budget/overseas_cost_val")},
	expenses_label{*parent.get_gui_label_from_nodepath("./country_budget/nat_stock_est")},
	army_stockpile_budget{parent},
	navy_stockpile_budget{parent},
	construction_stockpile_budget{parent}
{	
	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/nat_stock_desc",
		"BUDGET_NATIONAL_STOCKPILE","NAT_STOCK_DESC"
	);
}

void NationalStockpileBudget::update() {
	CountryInstance* const player_country_ptr = PlayerSingleton::get_singleton()->get_player_country_untracked();
	if (player_country_ptr == nullptr) {
		return;
	}
	CountryInstance& country = *player_country_ptr;

	const fixed_point_t military_balance = army_stockpile_budget.get_balance(connect_property_to_mark_dirty())
		+ navy_stockpile_budget.get_balance(connect_property_to_mark_dirty());
	const fixed_point_t new_balance = military_balance
		+ construction_stockpile_budget.get_balance(connect_property_to_mark_dirty());

	military_costs_label.set_text(
		Utilities::cash_to_string_dp_dynamic(-military_balance)
	);
	todays_actual_stockpile_spending_label.set_text(
		Utilities::cash_to_string_dp_dynamic(2) //TODO
	);
	overseas_costs_label.set_text(
		Utilities::cash_to_string_dp_dynamic(3) //TODO
	);
	expenses_label.set_text(
		Utilities::cash_to_string_dp_dynamic(-new_balance)
	);

	_balance = new_balance;
}