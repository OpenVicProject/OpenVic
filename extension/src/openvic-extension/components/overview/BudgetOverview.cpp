#include "BudgetOverview.hpp"

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUILineChart.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

BudgetOverview::BudgetOverview(GUINode const& parent):
funds_label{*parent.get_gui_label_from_nodepath("./topbar/budget_funds")},
history_chart{*parent.get_gui_line_chart_from_nodepath("./topbar/budget_linechart")}
{}

void BudgetOverview::update() {
	GameSingleton& game_singleton = *GameSingleton::get_singleton();
	CountryInstance* country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	if (country_ptr == nullptr) {
		return;
	}
	CountryInstance& country = *country_ptr;

	const fixed_point_t cash = country.get_cash_stockpile();
	ValueHistory<fixed_point_t> const& balance_history = country.get_balance_history();
	fixed_point_t last_balance;
	fixed_point_t maximum_balance;
	fixed_point_t minimum_balance;
	if (balance_history.empty()) {
		last_balance = maximum_balance = minimum_balance = fixed_point_t::_0;
	} else {
		maximum_balance = fixed_point_t::min;
		minimum_balance = fixed_point_t::max;
		for (fixed_point_t const& balance : balance_history) {
			if (balance > maximum_balance) {
				maximum_balance = balance;
			}
			if (balance < minimum_balance) {
				minimum_balance = balance;
			}
		}
		last_balance = balance_history.back();
		godot::PackedFloat32Array line_values;
		line_values.resize(balance_history.capacity());
		size_t index = line_values.size();
		for (auto it = balance_history.rbegin(); it != balance_history.rend(); ++it) {
			line_values[--index] = *it;
		}
		history_chart.set_gradient_line(line_values);
	}

	funds_label.set_text(
		Utilities::format(
			godot::String::utf8("§Y%s§W (§%s%s§W)"),
			Utilities::cash_to_string_dp_dynamic(cash),
			Utilities::get_colour_and_sign(last_balance),
			Utilities::cash_to_string_dp_dynamic(last_balance)
		)
	);
	funds_label.set_tooltip_string(
		funds_label.tr("TOPBAR_FUNDS")
			.replace("$YESTERDAY$", Utilities::format(
					godot::String::utf8("§%s%s§W"),
					Utilities::get_colour_and_sign(last_balance),
					Utilities::cash_to_string_dp_dynamic(last_balance)
				))
			.replace("$CASH$", Utilities::format(
					godot::String::utf8("§Y%s§W"),
					Utilities::cash_to_string_dp_dynamic(cash)
				))
	);
	history_chart.set_tooltip_string(
		history_chart.tr("TOPBAR_HISTORICAL_INCOME")
			.replace("$DAYS$", godot::String::num_int64(balance_history.size()))
			.replace("$MAX$", Utilities::format(
				godot::String::utf8("§%s%s§W"),
				Utilities::get_colour_and_sign(maximum_balance),
				Utilities::cash_to_string_dp_dynamic(maximum_balance)
			))
			.replace("$MIN$", Utilities::format(
				godot::String::utf8("§%s%s§W"),
				Utilities::get_colour_and_sign(minimum_balance),
				Utilities::cash_to_string_dp_dynamic(minimum_balance)
			))
	);
}