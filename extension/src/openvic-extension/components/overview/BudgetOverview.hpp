#pragma once

namespace OpenVic {
	struct GUILabel;
	struct GUILineChart;
	class GUINode;

	struct BudgetOverview {
	private:
		GUILabel& funds_label;
		GUILineChart& history_chart;
	public:
		BudgetOverview(GUINode const& parent);
		void update();
	};
}