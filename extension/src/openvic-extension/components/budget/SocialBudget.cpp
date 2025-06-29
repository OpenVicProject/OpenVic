#include "SocialBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

SocialBudget::SocialBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		EXPENSES,
		"./country_budget/exp_2_slider",
		"./country_budget/exp_val_2"
	),
	pensions_label{*parent.get_gui_label_from_nodepath("./country_budget/exp_val_2")}, //TODO figure out how to get their path
	unemployment_subsidies_label{*parent.get_gui_label_from_nodepath("./country_budget/exp_val_2")}
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(fixed_point_t::_0, 1, 100);
	slider.set_block_signals(false);
}

fixed_point_t SocialBudget::calculate_budget_and_update_custom(
	CountryInstance const& country,
	const fixed_point_t scaled_value
) {
	const fixed_point_t pensions = scaled_value * country.get_projected_pensions_spending_unscaled_by_slider();
	pensions_label.set_text(
		Utilities::cash_to_string_dp_dynamic(pensions)
	);
	const fixed_point_t unemployment_subsidies = scaled_value * country.get_projected_unemployment_subsidies_spending_unscaled_by_slider();
	unemployment_subsidies_label.set_text(
		Utilities::cash_to_string_dp_dynamic(unemployment_subsidies)
	);
	return pensions + unemployment_subsidies;
}
SliderValue const& SocialBudget::get_slider_value(CountryInstance const& country) const {
	return country.get_social_spending_slider_value();
}
void SocialBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_social_spending_slider_value(scaled_value);
}