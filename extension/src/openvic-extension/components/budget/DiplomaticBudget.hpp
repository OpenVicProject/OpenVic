#pragma once

#include <cstdint>

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>
#include <openvic-simulation/utility/reactive/MutableState.hpp>

#include "openvic-extension/components/budget/abstract/BudgetIncomeComponent.hpp"
#include "openvic-extension/components/budget/abstract/BudgetExpenseComponent.hpp"
#include "openvic-extension/components/ReactiveComponent.hpp"

namespace OpenVic {
	struct CountryInstance;
	struct GUILabel;
	struct GUINode;

	struct DiplomaticBudget :
		public ReactiveComponent,
		public BudgetExpenseComponent,
		public BudgetIncomeComponent {
	private:
		static const int32_t decimal_places = 1;

		GUILabel& balance_label;
		fixed_point_t reparations_income;
		fixed_point_t reparations_expenses;
		fixed_point_t war_subsidies_income;
		fixed_point_t war_subsidies_expenses;
		fixed_point_t _balance;

		fixed_point_t get_income() const;
		fixed_point_t get_expenses() const;
	protected:
		void update() override;
	public:
		DiplomaticBudget(GUINode const& parent);

		godot::String generate_income_summary_text(
			const fixed_point_t income,
			godot::Object const& translation_object
		) const override;
		godot::String generate_expenses_summary_text(
			const fixed_point_t expenses,
			godot::Object const& translation_object
		) const override;

		godot::String generate_balance_summary_income_text(godot::Object const& translation_object) const;
		godot::String generate_balance_summary_expenses_text(godot::Object const& translation_object) const;

		template<typename ConnectTemplateType>
		requires std::invocable<ConnectTemplateType, signal_property<ReactiveComponent>&>
		[[nodiscard]] fixed_point_t get_balance(ConnectTemplateType&& connect) {
			connect(marked_dirty);
			update_if_dirty();
			return _balance;
		}
	};
}