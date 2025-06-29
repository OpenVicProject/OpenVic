#pragma once

#include "openvic-extension/classes/BudgetComponent.hpp"

namespace OpenVic {
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;

	struct SocialBudget : public BudgetComponent {
	private:
		GUILabel* pensions_label { nullptr };
		GUILabel* unemployment_subsidies_label { nullptr };
		GUILabel* value_label { nullptr };
		GUIScrollbar* slider { nullptr };
		void on_slider_value_changed();

	public:
		SocialBudget(GUINode const& parent);
		void update();
	};
}