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
			CountryInstance& country,
			const fixed_point_t scaled_value
		) override;
		ReadOnlyClampedValue& get_clamped_value(CountryInstance& country) const override;
		void on_slider_scaled_value_changed(const fixed_point_t scaled_value) override;
	public:
		TariffBudget(
			GUINode const& parent,
			CountryDefines const& country_defines
		);
		void update_slider_tooltip(
			CountryInstance& country,
			const fixed_point_t scaled_value
		) override;
	};
}