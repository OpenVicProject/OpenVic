#pragma once

#include "openvic-extension/classes/BudgetComponent.hpp"
#include "openvic-simulation/pop/PopType.hpp"

namespace OpenVic {
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;

	struct StrataTaxBudget : public BudgetComponent {
	private:
		Strata const& strata;
		GUILabel* value_label { nullptr };
		GUIScrollbar* slider { nullptr };
		void on_slider_value_changed();

	public:
		StrataTaxBudget(GUINode const& parent, Strata const& new_strata);
		StrataTaxBudget(StrataTaxBudget&&) = default;
		void update();
	};
}