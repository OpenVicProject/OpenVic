#pragma once

#include "openvic-extension/classes/BudgetComponent.hpp"

namespace OpenVic {
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;

	struct TariffBudget : public BudgetComponent {
	private:
		GUILabel* rate_label { nullptr };
		GUILabel* value_label { nullptr };
		GUIScrollbar* slider { nullptr };
		void on_slider_value_changed();

	public:
		TariffBudget(GUINode const& parent);
		void update();
	};
}