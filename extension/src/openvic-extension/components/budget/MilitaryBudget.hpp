#pragma once

#include "openvic-extension/classes/BudgetComponent.hpp"

namespace OpenVic {
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;

	struct MilitaryBudget : public BudgetComponent {
	private:
		GUILabel* value_label { nullptr };
		GUIScrollbar* slider { nullptr };
		void on_slider_value_changed();

	public:
		MilitaryBudget(GUINode const& parent);
		void update();
	};
}