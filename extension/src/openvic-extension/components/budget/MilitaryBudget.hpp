#pragma once

#include "openvic-extension/components/budget/abstract/BudgetExpenseComponent.hpp"
#include "openvic-extension/components/budget/abstract/SliderBudgetComponent.hpp"

namespace OpenVic {
	struct MilitaryBudget : public SliderBudgetComponent, public BudgetExpenseComponent {
	private:
		bool was_budget_cut(CountryInstance const& country) const override;
		fixed_point_t calculate_budget_and_update_custom(
			CountryInstance& country,
			const fixed_point_t scaled_value
		) override;
		ReadOnlyClampedValue& get_clamped_value(CountryInstance& country) const override;
		void on_slider_value_changed(const fixed_point_t scaled_value) override;

	public:
		MilitaryBudget(GUINode const& parent);
		fixed_point_t get_expenses() const override;
	};
}