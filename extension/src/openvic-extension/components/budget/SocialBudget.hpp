#pragma once

#include "openvic-extension/components/budget/SliderBudgetComponent.hpp"

namespace OpenVic {
	struct SocialBudget : public SliderBudgetComponent {
	private:
		GUILabel& pensions_label;
		GUILabel& unemployment_subsidies_label;
		fixed_point_t calculate_budget_and_update_custom(
			CountryInstance const& country,
			const fixed_point_t scaled_value
		);
		SliderValue const& get_slider_value(CountryInstance const& country) const;
		void on_slider_value_changed(const fixed_point_t scaled_value);

	public:
		SocialBudget(GUINode const& parent);
	};
}