#include "StrataTaxBudget.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include "openvic-extension/classes/BudgetComponent.hpp"
#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"

#include "openvic-simulation/country/CountryInstance.hpp"

using namespace OpenVic;

StrataTaxBudget::StrataTaxBudget(GUINode const& parent, Strata const& new_strata):
	strata{new_strata},
	slider{parent.get_gui_scrollbar_from_nodepath(godot::vformat("./country_budget/tax_%s_slider", new_strata.get_index()))},
	value_label{parent.get_gui_label_from_nodepath(godot::vformat("./country_budget/tax_%s_inc", new_strata.get_index()))}
{
	ERR_FAIL_NULL(slider);	
	ERR_FAIL_NULL(value_label);
	slider->value_changed.connect(&StrataTaxBudget::on_slider_value_changed, this);
}

void StrataTaxBudget::on_slider_value_changed() {
	ERR_FAIL_NULL(country_ptr);
	ERR_FAIL_NULL(country_ptr);
	ERR_FAIL_NULL(slider);
	country_ptr->set_strata_tax_rate_slider_value(
		strata,
		slider->get_value_scaled_fp()
	);
	update();
}

void StrataTaxBudget::update() {
	ERR_FAIL_NULL(country_ptr);
	ERR_FAIL_NULL(slider);
	ERR_FAIL_NULL(value_label);

	const fixed_point_t revenue = slider->get_value_scaled_fp();
	//TODO project actual tax revenue

	value_label->set_text(
		Utilities::cash_to_string_dp_dynamic(revenue)
	);

	set_balance(revenue);
}