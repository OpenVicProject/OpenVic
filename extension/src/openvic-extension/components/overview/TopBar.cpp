#include "TopBar.hpp"

using namespace OpenVic;

TopBar::TopBar(GUINode const& parent):
budget_overview{parent},
score_overview{parent}
{}

void TopBar::update() {
	budget_overview.update();
	score_overview.update();
}