#pragma once

#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>

#include "openvic-extension/components/budget/abstract/BudgetComponent.hpp"

namespace OpenVic {
	struct CountryInstance;
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;
	struct SliderValue;

	enum BudgetType {
		BALANCE,
		EXPENSES
	};

	struct SliderBudgetComponent : public BudgetComponent {
	private:
		//multiplies balance by -1 for balance_label
		const BudgetType budget_type;
		GUILabel* const percent_label;
		void _on_slider_value_changed();
		void update_labels(CountryInstance const& country, const fixed_point_t scaled_value);
	protected:
		const godot::String slider_tooltip_localisation_key;
		GUIScrollbar& slider;
		GUILabel* const budget_label;

		SliderBudgetComponent(
			GUINode const& parent,
			godot::String&& new_slider_tooltip_localisation_key,
			const BudgetType new_budget_type,
			godot::NodePath const& slider_path,
			godot::NodePath const& budget_label_path = {},
			godot::NodePath const& percent_label_path = {}
		);

		virtual fixed_point_t calculate_budget_and_update_custom(
			CountryInstance const& country,
			const fixed_point_t scaled_value
		) = 0;
		virtual SliderValue const& get_slider_value(CountryInstance const& country) const = 0;
		virtual void on_slider_value_changed(const fixed_point_t scaled_value) = 0;
		virtual void update_slider_tooltip(
			CountryInstance const& country,
			const fixed_point_t scaled_value
		);

	public:
		void full_update(CountryInstance const& country) override;
	};
}