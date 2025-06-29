#include "SocialBudget.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/BudgetComponent.hpp"
#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

SocialBudget::SocialBudget(GUINode const& parent):
	pensions_label{parent.get_gui_label_from_nodepath("./country_budget/exp_val_2")}, //TODO figure out how to get their path
	unemployment_subsidies_label{parent.get_gui_label_from_nodepath("./country_budget/exp_val_2")},
	value_label{parent.get_gui_label_from_nodepath("./country_budget/exp_val_2")},
	slider{parent.get_gui_scrollbar_from_nodepath("./country_budget/exp_2_slider")}
{
	ERR_FAIL_NULL(pensions_label);
	ERR_FAIL_NULL(unemployment_subsidies_label);
	ERR_FAIL_NULL(value_label);
	ERR_FAIL_NULL(slider);
	slider->set_step_size_and_limits_fp(fixed_point_t::_0_01, fixed_point_t::_0, fixed_point_t::_1);
	slider->value_changed.connect(&SocialBudget::on_slider_value_changed, this);
}

void SocialBudget::on_slider_value_changed() {
	ERR_FAIL_NULL(slider);
	PlayerSingleton::get_singleton()->set_social_spending_slider_value(
		slider->get_value_scaled_fp()
	);
	update();
}

void SocialBudget::update() {
	ERR_FAIL_NULL(pensions_label);
	ERR_FAIL_NULL(unemployment_subsidies_label);
	ERR_FAIL_NULL(value_label);
	ERR_FAIL_NULL(slider);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	SliderValue const& slider_value = country.get_social_spending_slider_value();
	slider->set_range_limits_and_value_from_slider_value(slider_value, 1, false);
	const fixed_point_t value = slider->get_value_scaled_fp();
	const fixed_point_t pensions = value * country.get_projected_pensions_spending_unscaled_by_slider();
	pensions_label->set_text(
		Utilities::cash_to_string_dp_dynamic(pensions)
	);
	const fixed_point_t unemployment_subsidies = value * country.get_projected_unemployment_subsidies_spending_unscaled_by_slider();
	unemployment_subsidies_label->set_text(
		Utilities::cash_to_string_dp_dynamic(unemployment_subsidies)
	);
	const fixed_point_t expenses = pensions + unemployment_subsidies;

	value_label->set_text(
		Utilities::cash_to_string_dp_dynamic(expenses)
	);

	set_balance(-expenses);
}