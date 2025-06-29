#include "StrataTaxBudget.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/BudgetComponent.hpp"
#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

StrataTaxBudget::StrataTaxBudget(GUINode const& parent, Strata const& new_strata):
	strata{new_strata},
	slider{parent.get_gui_scrollbar_from_nodepath(
		godot::vformat("./country_budget/tax_%s_slider", static_cast<uint64_t>(new_strata.get_index()))
	)},
	value_label{parent.get_gui_label_from_nodepath(
		godot::vformat("./country_budget/tax_%s_inc", static_cast<uint64_t>(new_strata.get_index()))
	)}
{
	ERR_FAIL_NULL(slider);
	ERR_FAIL_NULL(value_label);
	slider->set_step_size_and_limits_fp(fixed_point_t::_0_01, fixed_point_t::_0, fixed_point_t::_1);
	slider->value_changed.connect(&StrataTaxBudget::on_slider_value_changed, this);
}

void StrataTaxBudget::on_slider_value_changed() {
	ERR_FAIL_NULL(slider);
	PlayerSingleton::get_singleton()->set_strata_tax_rate_slider_value(
		strata,
		slider->get_value_scaled_fp()
	);
	update();
}

void StrataTaxBudget::update() {
	ERR_FAIL_NULL(slider);
	ERR_FAIL_NULL(value_label);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	SliderValue const& slider_value = country.get_tax_rate_slider_value_by_strata()[strata];
	slider->set_range_limits_and_value_from_slider_value(slider_value, 1, false);
	const fixed_point_t revenue = slider_value.get_value() * country.get_strata_taxable_income(strata);

	value_label->set_text(
		Utilities::cash_to_string_dp_dynamic(revenue)
	);

	set_balance(revenue);
}