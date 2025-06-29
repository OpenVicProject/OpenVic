#include "MilitaryBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

MilitaryBudget::MilitaryBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		EXPENSES,
		"./country_budget/exp_3_slider",
		"./country_budget/exp_val_3"
	)
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(fixed_point_t::_0, 1, 100);
	slider.set_block_signals(false);
}

fixed_point_t MilitaryBudget::calculate_budget_and_update_custom(
	CountryInstance const& country,
	const fixed_point_t scaled_value
) {
	return scaled_value * country.get_projected_military_spending_unscaled_by_slider();
}
SliderValue const& MilitaryBudget::get_slider_value(CountryInstance const& country) const {
	return country.get_military_spending_slider_value();
}
void MilitaryBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_military_spending_slider_value(scaled_value);
}