#include "NationalStockpileBudget.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/economy/GoodInstance.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

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
	connections[0] = army_stockpile_budget.balance_changed.connect(&NationalStockpileBudget::on_slider_value_changed, this);
	connections[1] = navy_stockpile_budget.balance_changed.connect(&NationalStockpileBudget::on_slider_value_changed, this);
	connections[2] = construction_stockpile_budget.balance_changed.connect(&NationalStockpileBudget::on_slider_value_changed, this);
	
	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/nat_stock_desc",
		"BUDGET_NATIONAL_STOCKPILE","NAT_STOCK_DESC"
	);
}

fixed_point_t NationalStockpileBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

void NationalStockpileBudget::on_slider_value_changed() {
	CountryInstance* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	update_labels(*country_ptr);
}

void NationalStockpileBudget::full_update(CountryInstance& country) {\
	for (connection& c : connections) {
		c.block();
	};
	army_stockpile_budget.full_update(country);
	navy_stockpile_budget.full_update(country);
	construction_stockpile_budget.full_update(country);
	for (connection& c : connections) {
		c.unblock();
	};

	update_labels(country);
}

void NationalStockpileBudget::update_labels(CountryInstance& country) {
	const fixed_point_t military_balance = army_stockpile_budget.get_balance()
		+ navy_stockpile_budget.get_balance();
	const fixed_point_t balance = military_balance
		+ construction_stockpile_budget.get_balance();

	military_costs_label.set_text(
		Utilities::cash_to_string_dp_dynamic(-military_balance)
	);
	const fixed_point_t actual_stockpile_spending = country.get_actual_national_stockpile_spending();
	todays_actual_stockpile_spending_label.set_text(
		Utilities::cash_to_string_dp_dynamic(actual_stockpile_spending)
	);

	godot::String todays_actual_stockpile_spending_tooltip = todays_actual_stockpile_spending_label.tr("STOCKPILE_COST_ACTUAL");
	if (actual_stockpile_spending > 0) {
		todays_actual_stockpile_spending_tooltip += "\n"; //This is for a whiteline, not just a linebreak. Just like Victoria 2.

		memory::vector<std::pair<GoodInstance const*, CountryInstance::good_data_t const*>> bought_goods {};
		for (auto const& [good_instance, good_data] : country.get_goods_data()) {
			if (good_data.money_traded_yesterday < 0) {
				bought_goods.push_back({&good_instance, &good_data});
			}
		}

		std::ranges::sort(
			bought_goods, 
			[](auto const& lhs, auto const& rhs) {
				return lhs.second->money_traded_yesterday < rhs.second->money_traded_yesterday;
			}
		);

		for (auto const& [good_instance_ptr, good_data_ptr] : bought_goods) {
			GoodInstance const& good_instance = *good_instance_ptr;
			CountryInstance::good_data_t const& good_data = *good_data_ptr;
			todays_actual_stockpile_spending_tooltip += Utilities::format(
				godot::String::utf8("\n %s: §R%s§! (%s)"),
				todays_actual_stockpile_spending_label.tr(Utilities::std_to_godot_string(good_instance.get_identifier())),
				Utilities::cash_to_string_dp_dynamic(-good_data.money_traded_yesterday),
				Utilities::float_to_string_dp_dynamic(static_cast<float>(good_data.quantity_traded_yesterday))
			);
		}
	}

	todays_actual_stockpile_spending_label.set_tooltip_string(todays_actual_stockpile_spending_tooltip);

	overseas_costs_label.set_text(
		Utilities::cash_to_string_dp_dynamic(0) //TODO
	);
	expenses_label.set_text(
		Utilities::cash_to_string_dp_dynamic(-balance)
	);

	set_balance(balance);
}