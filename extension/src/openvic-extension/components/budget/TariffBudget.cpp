#include "TariffBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

TariffBudget::TariffBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		BALANCE,
		"./country_budget/tariff_slider",
		"./country_budget/tariff_val",
		"./country_budget/tariffs_percent"
	)
{
	slider.set_block_signals(true);
	slider.set_step_count(200);
	slider.set_scale(fixed_point_t::minus_one, 1, 100);
	slider.set_block_signals(false);
}

fixed_point_t TariffBudget::calculate_budget_and_update_custom(
	CountryInstance const& country,
	const fixed_point_t scaled_value
) {
	return scaled_value * country.get_yesterdays_import_value();
}
SliderValue const& TariffBudget::get_slider_value(CountryInstance const& country) const {
	return country.get_tariff_rate_slider_value();
}
void TariffBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_tariff_rate_slider_value(scaled_value);
}