#pragma once

namespace OpenVic {
	struct GUILabel;
	struct GUILineChart;
	struct GUINode;

	struct BudgetOverview {
	private:
		GUILabel& funds_label;
		GUILineChart& history_chart;
	public:
		BudgetOverview(GUINode const& parent);
		void update();
	};
}