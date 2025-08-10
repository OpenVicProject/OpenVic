#pragma once

#include <cstdint>

#include "openvic-extension/components/budget/abstract/BudgetComponent.hpp"
#include "openvic-extension/components/budget/abstract/BudgetIncomeComponent.hpp"
#include "openvic-extension/components/budget/abstract/BudgetExpenseComponent.hpp"

namespace OpenVic {
	struct GUILabel;
	struct GUINode;

	struct DiplomaticBudget :
		public BudgetComponent,
		public BudgetExpenseComponent,
		public BudgetIncomeComponent {
	private:
		static const int32_t decimal_places = 1;

		GUILabel& balance_label;
		fixed_point_t reparations_income;
		fixed_point_t reparations_expenses;
		fixed_point_t war_subsidies_income;
		fixed_point_t war_subsidies_expenses;
	public:
		fixed_point_t get_income() const override;
		fixed_point_t get_expenses() const override;

		DiplomaticBudget(GUINode const& parent);

		void full_update(CountryInstance& country) override;
		godot::String generate_income_summary_text(godot::Object const& translation_object) const override;
		godot::String generate_expenses_summary_text(godot::Object const& translation_object) const override;

		godot::String generate_balance_summary_income_text(godot::Object const& translation_object) const;
		godot::String generate_balance_summary_expenses_text(godot::Object const& translation_object) const;
	};
}