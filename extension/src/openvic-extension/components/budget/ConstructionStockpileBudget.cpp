#include "ConstructionStockpileBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

ConstructionStockpileBudget::ConstructionStockpileBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		EXPENSES,
		"./country_budget/projects_stockpile_slider"
	)
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(fixed_point_t::_0, 1, 100);
	slider.set_block_signals(false);
}

fixed_point_t ConstructionStockpileBudget::calculate_budget_and_update_custom(
	CountryInstance const& country,
	const fixed_point_t scaled_value
) {
	return scaled_value * 3; //TODO connect with sim once sim has this
}
SliderValue const& ConstructionStockpileBudget::get_slider_value(CountryInstance const& country) const {
	return country.get_construction_spending_slider_value();
}
void ConstructionStockpileBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_national_stockpile_construction_spending_slider_value(scaled_value);
}