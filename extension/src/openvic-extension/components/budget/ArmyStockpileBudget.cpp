#include "ArmyStockpileBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

ArmyStockpileBudget::ArmyStockpileBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		"MILITARY_SPENDINGS_LAND",
		EXPENSES,
		"./country_budget/land_stockpile_slider"
	),
	BudgetExpenseComponent("MILITARY_SPENDINGS_LAND")
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);
}

fixed_point_t ArmyStockpileBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

fixed_point_t ArmyStockpileBudget::calculate_budget_and_update_custom(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	return scaled_value * 1; //TODO connect with sim once sim has this
}

ReadOnlyClampedValue& ArmyStockpileBudget::get_clamped_value(CountryInstance& country) const {
	return country.get_army_spending_slider_value();
}

void ArmyStockpileBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_national_stockpile_army_spending_slider_value(scaled_value);
}