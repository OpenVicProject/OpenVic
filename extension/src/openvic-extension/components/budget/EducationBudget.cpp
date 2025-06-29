#include "EducationBudget.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/BudgetComponent.hpp"
#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

EducationBudget::EducationBudget(GUINode const& parent):
	value_label{parent.get_gui_label_from_nodepath("./country_budget/exp_val_0")},
	slider{parent.get_gui_scrollbar_from_nodepath("./country_budget/exp_0_slider")}
{
	ERR_FAIL_NULL(slider);
	ERR_FAIL_NULL(value_label);
	slider->set_step_size_and_limits_fp(fixed_point_t::_0_01, fixed_point_t::_0, fixed_point_t::_1);
	slider->value_changed.connect(&EducationBudget::on_slider_value_changed, this);
}

void EducationBudget::on_slider_value_changed() {
	ERR_FAIL_NULL(slider);
	PlayerSingleton::get_singleton()->set_education_spending_slider_value(
		slider->get_value_scaled_fp()
	);
	update();
}

void EducationBudget::update() {
	ERR_FAIL_NULL(slider);
	ERR_FAIL_NULL(value_label);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	SliderValue const& slider_value = country.get_education_spending_slider_value();
	slider->set_range_limits_and_value_from_slider_value(slider_value, 1, false);
	const fixed_point_t expenses = slider_value.get_value()	* country.get_projected_education_spending_unscaled_by_slider();

	value_label->set_text(
		Utilities::cash_to_string_dp_dynamic(expenses)
	);

	set_balance(-expenses);
}