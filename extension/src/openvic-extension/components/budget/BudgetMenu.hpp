#pragma once

#include <vector>

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/types/Signal.hpp>
#include <openvic-simulation/utility/ForwardableSpan.hpp>

#include "openvic-extension/components/budget/AdministrationBudget.hpp"
#include "openvic-extension/components/budget/EducationBudget.hpp"
#include "openvic-extension/components/budget/MilitaryBudget.hpp"
#include "openvic-extension/components/budget/NationalStockpileBudget.hpp"
#include "openvic-extension/components/budget/SocialBudget.hpp"
#include "openvic-extension/components/budget/StrataTaxBudget.hpp"
#include "openvic-extension/components/budget/TariffBudget.hpp"

namespace OpenVic {
	struct CountryInstance;
	struct GUINode;
	struct GUILabel;

	struct BudgetMenu {
	private:
		std::vector<connection> connections;
		GUILabel* administrative_efficiency_label { nullptr };
		GUILabel* gold_income_label { nullptr };
		GUILabel* projected_balance_label { nullptr };
		GUILabel* projected_expenses_label { nullptr };
		GUILabel* projected_income_label { nullptr };
		AdministrationBudget administration_budget;
		EducationBudget education_budget;
		MilitaryBudget military_budget;
		NationalStockpileBudget national_stockpile_budget;
		SocialBudget social_budget;
		TariffBudget tariff_budget;
		std::vector<StrataTaxBudget> tax_budgets;

		void update_projected_balance();
		void update_projected_expenses();
		void update_projected_expenses_and_balance();
		void update_projected_income();
		void update_projected_income_and_balance();
		void update_all_projections();
	public:
		BudgetMenu(GUINode const& parent, utility::forwardable_span<const Strata> strata_keys);
		void update();
	};
}