#pragma once

#include "openvic-extension/components/budget/BudgetComponent.hpp"
#include "openvic-extension/components/budget/ArmyStockpileBudget.hpp"
#include "openvic-extension/components/budget/ConstructionStockpileBudget.hpp"
#include "openvic-extension/components/budget/NavyStockpileBudget.hpp"

namespace OpenVic {
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;

	struct NationalStockpileBudget : public BudgetComponent {
	private:
		connection connections[3];
		GUILabel* military_costs_label = nullptr;
		GUILabel* todays_actual_stockpile_spending_label = nullptr;
		GUILabel* overseas_costs_label = nullptr;
		GUILabel* value_label = nullptr;
		ArmyStockpileBudget army_stockpile_budget;
		NavyStockpileBudget navy_stockpile_budget;
		ConstructionStockpileBudget construction_stockpile_budget;
		void on_slider_value_changed();
		void update_labels(CountryInstance const& country);

	public:
		NationalStockpileBudget(GUINode const& parent);
		void full_update(CountryInstance const& country);
	};
}