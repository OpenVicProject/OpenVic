#pragma once

#include "openvic-extension/components/budget/abstract/BudgetIncomeComponent.hpp"
#include "openvic-extension/components/budget/abstract/SliderBudgetComponent.hpp"

namespace OpenVic {
	struct ModifierEffectCache;
	struct Strata;

	struct StrataTaxBudget : public SliderBudgetComponent, public BudgetIncomeComponent {
	private:
		static godot::StringName generate_summary_localisation_key(Strata const& strata);
		static godot::StringName generate_slider_tooltip_localisation_key(Strata const& strata);

		Strata const& PROPERTY(strata);
		ModifierEffectCache const& modifier_effect_cache;

		fixed_point_t calculate_budget_and_update_custom(
			CountryInstance& country,
			const fixed_point_t scaled_value
		) override;
		ReadOnlyClampedValue& get_clamped_value(CountryInstance& country) const override;
		void on_slider_value_changed(const fixed_point_t scaled_value) override;
		void update_slider_tooltip(
			CountryInstance& country,
			const fixed_point_t scaled_value
		) override;

	public:
		StrataTaxBudget(
			GUINode const& parent,
			Strata const& new_strata,
			ModifierEffectCache const& new_modifier_effect_cache
		);
		StrataTaxBudget(StrataTaxBudget&&) = default;
		fixed_point_t get_income() const override;
	};
}