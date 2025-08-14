#pragma once

#include <openvic-simulation/types/Signal.hpp>

namespace OpenVic {
	struct GUILabel;
	struct GUILineChart;
	struct GUINode;

	struct BudgetOverview : public observer {
	private:
		GUILabel& funds_label;
		GUILineChart& history_chart;

		void on_gamestate_updated();
	public:
		BudgetOverview(GUINode const& parent);
	};
}