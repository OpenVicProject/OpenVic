#include "TariffBudget.hpp"

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

TariffBudget::TariffBudget(GUINode const& parent):
	rate_label{parent.get_gui_label_from_nodepath("./country_budget/tariffs_percent")},
	value_label{parent.get_gui_label_from_nodepath("./country_budget/tariff_val")},
	slider{parent.get_gui_scrollbar_from_nodepath("./country_budget/tariff_slider")}
{
	ERR_FAIL_NULL(slider);
	ERR_FAIL_NULL(rate_label);
	ERR_FAIL_NULL(value_label);
	slider->set_step_size_and_limits_fp(fixed_point_t::_0_01, fixed_point_t::minus_one, fixed_point_t::_1);
	slider->value_changed.connect(&TariffBudget::on_slider_value_changed, this);
}

void TariffBudget::on_slider_value_changed() {
	ERR_FAIL_NULL(slider);
	PlayerSingleton::get_singleton()->set_tariff_rate_slider_value(
		slider->get_value_scaled_fp()
	);
	update();
}

void TariffBudget::update() {
	ERR_FAIL_NULL(slider);
	ERR_FAIL_NULL(rate_label);
	ERR_FAIL_NULL(value_label);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	SliderValue const& slider_value = country.get_tariff_rate_slider_value();
	slider->set_range_limits_and_value_from_slider_value(slider_value, 1, false);
	const fixed_point_t tariff_rate = 100 * slider_value.get_value();
	rate_label->set_text(
		godot::vformat(
			"%s%%",
			Utilities::float_to_string_dp(
				tariff_rate.to_float(),
				1
			)
		)
	);

	const fixed_point_t balance = slider->get_value_scaled_fp()
		* country.get_yesterdays_import_value();

	value_label->set_text(
		Utilities::cash_to_string_dp_dynamic(balance)
	);

	set_balance(balance);
}