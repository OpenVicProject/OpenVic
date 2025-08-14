#pragma once

#include <godot_cpp/variant/array.hpp>

#include "openvic-extension/components/budget/abstract/BudgetExpenseComponent.hpp"
#include "openvic-extension/components/budget/abstract/SliderBudgetComponent.hpp"

namespace OpenVic {
	struct CountryDefines;

	struct AdministrationBudget : public SliderBudgetComponent, public BudgetExpenseComponent {
	private:
		CountryDefines const& country_defines;
		godot::Array administrative_efficiency_tooltip_args;
		GUILabel& administrative_efficiency_label;
		fixed_point_t calculate_budget_and_update_custom(
			CountryInstance& country,
			const fixed_point_t scaled_value
		) override;
		ReadOnlyClampedValue& get_clamped_value(CountryInstance& country) const override;
		void on_slider_scaled_value_changed(const fixed_point_t scaled_value) override;
	public:
		AdministrationBudget(
			GUINode const& parent,
			CountryDefines const& new_country_defines
		);
	};
}