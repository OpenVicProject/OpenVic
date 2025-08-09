#include "SliderBudgetComponent.hpp"

#include <godot_cpp/core/error_macros.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/types/ClampedValue.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-simulation/types/Signal.hpp"

using namespace OpenVic;

SliderBudgetComponent::SliderBudgetComponent(
	GUINode const& parent,
	godot::String&& new_slider_tooltip_localisation_key,
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
	slider_scaled_value_cached = slider.get_scaled_value(
		[this](signal<fixed_point_t>& slider_scaled_value_changed) mutable -> void {
			slider_scaled_value_connection = std::move(
				slider_scaled_value_changed.connect(&SliderBudgetComponent::_on_slider_scaled_value_changed, this)
			);
		}
	);
	player_country_cached = PlayerSingleton::get_singleton()->get_player_country(
		[this](signal<CountryInstance*>& player_country_changed) mutable ->void {
			player_country_connection = std::move(
				player_country_changed.connect(&SliderBudgetComponent::_on_player_country_changed, this)
			);
		}
	);
}

void SliderBudgetComponent::initialise() {
	_on_player_country_changed(player_country_cached);
}

void SliderBudgetComponent::_on_player_country_changed(CountryInstance* new_player_country) {
	player_country_cached = new_player_country;
	if (new_player_country == nullptr) {
		slider.unlink();
	} else {
		ReadOnlyClampedValue& clamped_value = get_clamped_value(*new_player_country);
		slider.set_block_signals(true);
		slider.link_to(clamped_value);
		slider.set_block_signals(false);
		slider_scaled_value_cached = clamped_value.get_value_untracked();
	}
	mark_dirty();
}

void SliderBudgetComponent::_on_slider_scaled_value_changed(const fixed_point_t scaled_value) {
	slider_scaled_value_cached = scaled_value;
	on_slider_scaled_value_changed(scaled_value);
	mark_dirty();
}

void SliderBudgetComponent::update() {
	if (player_country_cached == nullptr) {
		return;
	}
	CountryInstance& country = *player_country_cached;
	const fixed_point_t scaled_value = slider_scaled_value_cached;
	const fixed_point_t budget = calculate_budget_and_update_custom(country, scaled_value);
	if (budget_label != nullptr) {
		budget_label->set_text(
			Utilities::cash_to_string_dp_dynamic(budget)
		);
	};

	if (percent_label != nullptr) {
		percent_label->set_text(
			Utilities::percentage_to_string_dp(scaled_value, 1)
		);
	}

	update_slider_tooltip(country, scaled_value);

	_balance = budget_type == EXPENSES
		? -budget 
		: budget;
}

void SliderBudgetComponent::update_slider_tooltip(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	godot::String tooltip;
	const godot::String percentage_text = Utilities::float_to_string_dp(
		(100 * scaled_value).to_float(),
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