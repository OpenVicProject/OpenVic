#include "BudgetOverview.hpp"

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUILineChart.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

BudgetOverview::BudgetOverview(GUINode const& parent):
funds_label{*parent.get_gui_label_from_nodepath("./topbar/budget_funds")},
history_chart{*parent.get_gui_line_chart_from_nodepath("./topbar/budget_linechart")}
{
	GameSingleton& game_singleton = *GameSingleton::get_singleton();
	game_singleton.gamestate_updated.connect(&BudgetOverview::on_gamestate_updated, this);
}

void BudgetOverview::on_gamestate_updated() {
	GameSingleton& game_singleton = *GameSingleton::get_singleton();
	CountryInstance* country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	if (country_ptr == nullptr) {
		return;
	}
	CountryInstance& country = *country_ptr;

	const fixed_point_t cash = country.get_cash_stockpile();

	//TOPBAR_HISTORICAL_INCOME;The last §Y$DAYS$§W days, our highest income was $MAX$¤ and our lowest was $MIN$¤.;
	//TOPBAR_FUNDS;Yesterday we earned $YESTERDAY$¤. Our current cash reserve is §Y$CASH$§W¤.;
	//"§Y%s§!¤(§%s%s§!¤)" % [
	// 	GUINode.float_to_string_suffixed(cash),
	// 	"G+" if earnings > 0.0 else "R" if earnings < 0.0 else "Y+",
	// 	GUINode.float_to_string_suffixed(earnings)
	// ])
}