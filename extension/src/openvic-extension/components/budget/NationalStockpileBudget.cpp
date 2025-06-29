#include "NationalStockpileBudget.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

NationalStockpileBudget::NationalStockpileBudget(GUINode const& parent):
	military_costs_label{parent.get_gui_label_from_nodepath("./country_budget/mil_cost_val")},
	todays_actual_stockpile_spending_label{parent.get_gui_label_from_nodepath("./country_budget/nat_stock_val")},
	overseas_costs_label{parent.get_gui_label_from_nodepath("./country_budget/overseas_cost_val")},
	value_label{parent.get_gui_label_from_nodepath("./country_budget/nat_stock_est")}
{
	ERR_FAIL_NULL(military_costs_label);
	ERR_FAIL_NULL(todays_actual_stockpile_spending_label);
	ERR_FAIL_NULL(overseas_costs_label);
	ERR_FAIL_NULL(value_label);
	//TODO add components for military, navy & construction sliders
	//TODO connect to their signals
}

void NationalStockpileBudget::update() {
	ERR_FAIL_NULL(value_label);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;

	const fixed_point_t expenses = 0;

	military_costs_label->set_text(
		Utilities::cash_to_string_dp_dynamic(1)
	);
	todays_actual_stockpile_spending_label->set_text(
		Utilities::cash_to_string_dp_dynamic(2)
	);
	overseas_costs_label->set_text(
		Utilities::cash_to_string_dp_dynamic(3)
	);
	value_label->set_text(
		Utilities::cash_to_string_dp_dynamic(expenses)
	);

	set_balance(-expenses);
}