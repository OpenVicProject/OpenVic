#pragma once

#include <godot_cpp/variant/array.hpp>

#include "openvic-extension/components/budget/abstract/BudgetIncomeComponent.hpp"
#include "openvic-extension/components/budget/abstract/BudgetExpenseComponent.hpp"
#include "openvic-extension/components/budget/abstract/SliderBudgetComponent.hpp"

namespace OpenVic {
	struct CountryDefines;

	struct TariffBudget:
		public SliderBudgetComponent,
		public BudgetIncomeComponent,
		public BudgetExpenseComponent {
	private:
		godot::Array slider_tooltip_args;

		fixed_point_t calculate_budget_and_update_custom(
			CountryInstance const& country,
			const fixed_point_t scaled_value
		) override;
		SliderValue const& get_slider_value(CountryInstance const& country) const override;
		void on_slider_value_changed(const fixed_point_t scaled_value) override;

	public:
		TariffBudget(
			GUINode const& parent,
			CountryDefines const& country_defines
		);
		fixed_point_t get_expenses() const override;
		fixed_point_t get_income() const override;
		void update_slider_tooltip(
			CountryInstance const& country,
			const fixed_point_t scaled_value
		) override;
	};
}