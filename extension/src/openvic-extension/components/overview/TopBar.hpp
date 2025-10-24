#pragma once

#include "openvic-extension/components/overview/BudgetOverview.hpp"
#include "openvic-extension/components/overview/ScoreOverview.hpp"

namespace OpenVic {
	struct GUINode;

	struct TopBar {
	private:
		BudgetOverview budget_overview;
		ScoreOverview score_overview;
	public:
		TopBar(GUINode const& parent);
		void update();
	};
}