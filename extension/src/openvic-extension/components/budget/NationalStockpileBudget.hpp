#pragma once

#include "openvic-extension/classes/BudgetComponent.hpp"

namespace OpenVic {
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;

	struct NationalStockpileBudget : public BudgetComponent {
	private:
		GUILabel* military_costs_label { nullptr };
		GUILabel* todays_actual_stockpile_spending_label { nullptr };
		GUILabel* overseas_costs_label { nullptr };
		GUILabel* value_label { nullptr };

	public:
		NationalStockpileBudget(GUINode const& parent);
		void update();
	};
}