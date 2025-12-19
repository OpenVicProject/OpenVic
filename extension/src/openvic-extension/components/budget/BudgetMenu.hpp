#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/core/portable/ForwardableSpan.hpp>
#include <openvic-simulation/types/Signal.hpp>

#include "openvic-extension/components/budget/AdministrationBudget.hpp"
#include "openvic-extension/components/budget/DiplomaticBudget.hpp"
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
		static godot::String generate_projected_income_template(const size_t tax_budgets_size);

		godot::String projected_income_template;
		godot::Array projected_income_args;
		godot::Array projected_expenses_args;
		memory::vector<connection> connections;
		GUILabel& cash_stockpile_label;
		GUILabel& gold_income_label;
		GUILabel& projected_balance_label;
		GUILabel& projected_expenses_label;
		GUILabel& projected_income_label;
		AdministrationBudget administration_budget;
		DiplomaticBudget diplomatic_budget;
		EducationBudget education_budget;
		MilitaryBudget military_budget;
		NationalStockpileBudget national_stockpile_budget;
		SocialBudget social_budget;
		TariffBudget tariff_budget;
		memory::vector<StrataTaxBudget> tax_budgets;

		void update_projected_balance();
		void update_projected_expenses();
		void update_projected_expenses_and_balance();
		void update_projected_income();
		void update_projected_income_and_balance();
		void update_all_projections();
	public:
		BudgetMenu(
			GUINode const& parent,
			forwardable_span<const Strata> strata_keys,
			ModifierEffectCache const& modifier_effect_cache,
			CountryDefines const& country_defines
		);
		void update();
	};
}