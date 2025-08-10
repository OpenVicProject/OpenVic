#pragma once

#include "openvic-extension/components/budget/abstract/BudgetExpenseComponent.hpp"
#include "openvic-extension/components/budget/abstract/SliderBudgetComponent.hpp"

namespace OpenVic {
	struct ArmyStockpileBudget : public SliderBudgetComponent, public BudgetExpenseComponent {
	private:
		fixed_point_t calculate_budget_and_update_custom(
			CountryInstance& country,
			const fixed_point_t scaled_value
		) override;
		ReadOnlyClampedValue& get_clamped_value(CountryInstance& country) const override;
		void on_slider_value_changed(const fixed_point_t scaled_value) override;

	public:
		ArmyStockpileBudget(GUINode const& parent);
		fixed_point_t get_expenses() const override;
	};
}