#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/types/Signal.hpp>
#include <openvic-simulation/utility/ForwardableSpan.hpp>

#include "openvic-extension/components/budget/BudgetMenuExpenses.hpp"
#include "openvic-extension/components/budget/BudgetMenuIncome.hpp"
#include "openvic-extension/components/budget/AdministrationBudget.hpp"
#include "openvic-extension/components/budget/DiplomaticBudget.hpp"
#include "openvic-extension/components/budget/NationalStockpileBudget.hpp"
#include "openvic-extension/components/budget/StrataTaxBudget.hpp"
#include "openvic-extension/components/budget/TariffBudget.hpp"
#include "openvic-extension/components/ReactiveComponent.hpp"

namespace OpenVic {
	struct CountryInstance;
	struct GUINode;
	struct GUILabel;

	struct BudgetMenu : public ReactiveComponent {
	private:
		GUILabel& cash_stockpile_label;
		GUILabel& projected_balance_label;
		DiplomaticBudget diplomatic_budget;
		TariffBudget tariff_budget;

		BudgetMenuExpenses expenses_component;
		BudgetMenuIncome income_component;

		void update() override;
	public:
		BudgetMenu(
			GUINode const& parent,
			utility::forwardable_span<const Strata> strata_keys,
			ModifierEffectCache const& modifier_effect_cache,
			CountryDefines const& country_defines
		);
	};
}