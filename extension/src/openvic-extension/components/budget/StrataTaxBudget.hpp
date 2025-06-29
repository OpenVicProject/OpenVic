#pragma once

#include <openvic-simulation/pop/PopType.hpp>

#include "openvic-extension/components/budget/SliderBudgetComponent.hpp"

namespace OpenVic {
	struct StrataTaxBudget : public SliderBudgetComponent {
	private:
		Strata const& strata;
		fixed_point_t calculate_budget_and_update_custom(
			CountryInstance const& country,
			const fixed_point_t scaled_value
		);
		SliderValue const& get_slider_value(CountryInstance const& country) const;
		void on_slider_value_changed(const fixed_point_t scaled_value);

	public:
		StrataTaxBudget(GUINode const& parent, Strata const& new_strata);
		StrataTaxBudget(StrataTaxBudget&&) = default;
	};
}