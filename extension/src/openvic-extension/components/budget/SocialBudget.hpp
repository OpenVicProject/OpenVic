#pragma once

#include "openvic-extension/components/budget/abstract/BudgetExpenseComponent.hpp"
#include "openvic-extension/components/budget/abstract/SliderBudgetComponent.hpp"

namespace OpenVic {
	struct SocialBudget : public SliderBudgetComponent, public BudgetExpenseComponent {
	private:
		GUILabel& pensions_label;
		GUILabel& unemployment_subsidies_label;
		fixed_point_t calculate_budget_and_update_custom(
			CountryInstance& country,
			const fixed_point_t scaled_value
		) override;
		ReadOnlyClampedValue& get_clamped_value(CountryInstance& country) const override;
		void on_slider_scaled_value_changed(const fixed_point_t scaled_value) override;
	public:
		SocialBudget(GUINode const& parent);
	};
}