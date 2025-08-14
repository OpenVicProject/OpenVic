#pragma once

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/components/budget/ArmyStockpileBudget.hpp"
#include "openvic-extension/components/budget/ConstructionStockpileBudget.hpp"
#include "openvic-extension/components/budget/NavyStockpileBudget.hpp"
#include "openvic-extension/components/ReactiveComponent.hpp"

namespace OpenVic {
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;

	struct NationalStockpileBudget : public ReactiveComponent, public BudgetExpenseComponent {
	private:
		GUILabel& military_costs_label;
		GUILabel& todays_actual_stockpile_spending_label;
		GUILabel& overseas_costs_label;
		GUILabel& expenses_label;
		ArmyStockpileBudget army_stockpile_budget;
		NavyStockpileBudget navy_stockpile_budget;
		ConstructionStockpileBudget construction_stockpile_budget;
		fixed_point_t _balance;
	public:
		NationalStockpileBudget(GUINode const& parent);
		void update() override;

		template<typename ConnectTemplateType>
		requires std::invocable<ConnectTemplateType, signal_property<ReactiveComponent>&>
		[[nodiscard]] fixed_point_t get_balance(ConnectTemplateType&& connect) {
			connect(marked_dirty);
			update_if_dirty();
			return _balance;
		}
	};
}