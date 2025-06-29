#pragma once

#include <godot_cpp/variant/node_path.hpp>

#include "openvic-extension/components/budget/BudgetComponent.hpp"

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
		GUILabel* const budget_label;
		GUILabel* const percent_label;
		void _on_slider_value_changed();
		void update_labels(CountryInstance const& country, const fixed_point_t scaled_value);
	protected:
		GUIScrollbar& slider;
		virtual fixed_point_t calculate_budget_and_update_custom(
			CountryInstance const& country,
			const fixed_point_t scaled_value
		) = 0;
		virtual SliderValue const& get_slider_value(CountryInstance const& country) const = 0;
		virtual void on_slider_value_changed(const fixed_point_t scaled_value) = 0;

	public:
		SliderBudgetComponent(
			GUINode const& parent,
			const BudgetType new_budget_type,
			godot::NodePath const& slider_path,
			godot::NodePath const& budget_label_path = {},
			godot::NodePath const& percent_label_path = {}
		);
		void full_update(CountryInstance const& country);
	};
}