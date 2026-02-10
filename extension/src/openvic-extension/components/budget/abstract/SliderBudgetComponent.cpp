#include "SliderBudgetComponent.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

SliderBudgetComponent::SliderBudgetComponent(
	GUINode const& parent,
	godot::StringName&& new_slider_tooltip_localisation_key,
	const BudgetType new_budget_type,
	godot::NodePath const& slider_path,
	godot::NodePath const& budget_label_path,
	godot::NodePath const& percent_label_path
) : budget_type{new_budget_type},
	slider_tooltip_localisation_key{std::move(new_slider_tooltip_localisation_key)},
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
	CountryInstance* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	update_labels(*country_ptr, scaled_value);
}

void SliderBudgetComponent::full_update(CountryInstance& country) {
	ReadOnlyClampedValue& clamped_value = get_clamped_value(country);

	slider.set_block_signals(true);
	slider.set_range_limits_and_value_from_slider_value(clamped_value);
	slider.set_block_signals(false);
	update_labels(country, clamped_value.get_value_untracked());
}

void SliderBudgetComponent::update_labels(CountryInstance& country, const fixed_point_t scaled_value) {
	const fixed_point_t budget = calculate_budget_and_update_custom(country, scaled_value);
	if (budget_label != nullptr) {
		const godot::String budget_text = Utilities::cash_to_string_dp_dynamic(budget);
		budget_label->set_text(
			was_budget_cut(country)
				? Utilities::format(godot::String::utf8("§R%s§W"), budget_text)
				: budget_text
		);
	};

	if (percent_label != nullptr) {
		percent_label->set_text(
			Utilities::percentage_to_string_dp(scaled_value, 1)
		);
	}

	update_slider_tooltip(country, scaled_value);

	const fixed_point_t balance = budget_type == EXPENSES
		? -budget 
		: budget;
	set_balance(balance);
}

void SliderBudgetComponent::update_slider_tooltip(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	godot::String tooltip;
	const godot::String percentage_text = Utilities::float_to_string_dp(
		static_cast<float>(100 * scaled_value),
		1
	);
	const godot::String localised = slider.tr(slider_tooltip_localisation_key);
	if (localised.contains(Utilities::get_short_value_placeholder())) {
		tooltip = Utilities::format(
			"%s%%",
			localised.replace(
				Utilities::get_short_value_placeholder(),
				percentage_text
			)
		);
	} else {
		tooltip = Utilities::format(
			"%s%s%%",
			localised,
			percentage_text
		);
	}
	slider.set_tooltip_string(tooltip);
}