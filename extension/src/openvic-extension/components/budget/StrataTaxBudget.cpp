#include "StrataTaxBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/components/budget/SliderBudgetComponent.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

StrataTaxBudget::StrataTaxBudget(GUINode const& parent, Strata const& new_strata):
	SliderBudgetComponent(
		parent,
		BALANCE,
		godot::vformat("./country_budget/tax_%s_slider", static_cast<uint64_t>(new_strata.get_index())),
		godot::vformat("./country_budget/tax_%s_inc", static_cast<uint64_t>(new_strata.get_index()))
	),
	strata{new_strata}
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);
}

fixed_point_t StrataTaxBudget::calculate_budget_and_update_custom(
	CountryInstance const& country,
	const fixed_point_t scaled_value
) {
	return scaled_value
		* country.get_strata_taxable_income(strata)
		* country.get_tax_efficiency();
}

SliderValue const& StrataTaxBudget::get_slider_value(CountryInstance const& country) const {
	return country.get_tax_rate_slider_value_by_strata()[strata];
}

void StrataTaxBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_strata_tax_rate_slider_value(
		strata,
		scaled_value
	);
}