#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/types/IndexedFlatMap.hpp>
#include <openvic-simulation/types/Signal.hpp>
#include <openvic-simulation/utility/ForwardableSpan.hpp>

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
		IndexedFlatMap<Strata, StrataTaxBudget> tax_budgets;

		void update_projected_balance();
		void update_projected_expenses_tooltip_localisation();
		void update_projected_expenses();
		void update_projected_expenses_and_balance();
		void update_projected_income();
		void update_projected_income_and_balance();
		void update_all_projections();
		void update_gold_income_description_localisation();
		void update_projected_income_localisation();

	public:
		BudgetMenu(
			GUINode const& parent,
			decltype(tax_budgets)::keys_span_type strata_keys,
			ModifierEffectCache const& modifier_effect_cache,
			CountryDefines const& country_defines
		);

		void update();
		void update_localisation();
	};
}
