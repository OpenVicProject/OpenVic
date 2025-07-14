#include "SliderBudgetComponent.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/components/budget/BudgetComponent.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

SliderBudgetComponent::SliderBudgetComponent(
	GUINode const& parent,
	const BudgetType new_budget_type,
	godot::NodePath const& slider_path,
	godot::NodePath const& budget_label_path,
	godot::NodePath const& percent_label_path
):
	budget_type{new_budget_type},
	budget_label{
		budget_label_path.is_empty()
		? nullptr
		: parent.get_gui_label_from_nodepath(budget_label_path)
	},
	percent_label{
		percent_label_path.is_empty()
		? nullptr
		: parent.get_gui_label_from_nodepath(percent_label_path)
	},
	slider{*parent.get_gui_scrollbar_from_nodepath(slider_path)}
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);
	slider.value_changed.connect(&SliderBudgetComponent::_on_slider_value_changed, this);
}

void SliderBudgetComponent::_on_slider_value_changed() {
	const fixed_point_t scaled_value = slider.get_value_scaled_fp();
	on_slider_value_changed(scaled_value);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	update_labels(*country_ptr, scaled_value);
}

void SliderBudgetComponent::full_update(CountryInstance const& country) {
	SliderValue const& slider_value = get_slider_value(country);

	slider.set_block_signals(true);
	slider.set_range_limits_and_value_from_slider_value(slider_value);
	slider.set_block_signals(false);
	update_labels(country, slider_value.get_value());
}

void SliderBudgetComponent::update_labels(CountryInstance const& country, const fixed_point_t scaled_value) {
	const fixed_point_t budget = calculate_budget_and_update_custom(country, scaled_value);
	if (budget_label != nullptr) {
		budget_label->set_text(
			Utilities::cash_to_string_dp_dynamic(budget)
		);
	};

	if (percent_label != nullptr) {
		percent_label->set_text(
			godot::vformat(
				"%s%%",
				Utilities::float_to_string_dp(
					(100 * scaled_value).to_float(),
					1
				)
			)
		);
	}

	const fixed_point_t balance = budget_type == EXPENSES
		? -budget 
		: budget;
	set_balance(balance);
}