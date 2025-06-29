#include "BudgetMenu.hpp"

#include <godot_cpp/core/error_macros.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

BudgetMenu::BudgetMenu(GUINode const& parent, utility::forwardable_span<const Strata> strata_keys)
	: administrative_efficiency_label{parent.get_gui_label_from_nodepath("./country_budget/admin_efficiency")},
	gold_income_label{parent.get_gui_label_from_nodepath("./country_budget/gold_inc")},
	projected_balance_label{parent.get_gui_label_from_nodepath("./country_budget/balance")},
	projected_expenses_label{parent.get_gui_label_from_nodepath("./country_budget/total_exp")},
	projected_income_label{parent.get_gui_label_from_nodepath("./country_budget/total_inc")},
	administration_budget{parent},
	education_budget{parent},
	military_budget{parent},
	national_stockpile_budget{parent},
	social_budget{parent},
	tariff_budget{parent}
	{
		ERR_FAIL_NULL(administrative_efficiency_label);
		ERR_FAIL_NULL(gold_income_label);
		ERR_FAIL_NULL(projected_balance_label);
		ERR_FAIL_NULL(projected_expenses_label);
		ERR_FAIL_NULL(projected_income_label);
		tax_budgets.reserve(strata_keys.size());
		connections.reserve(6 + strata_keys.size());
		connections.push_back(administration_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(education_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(military_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(national_stockpile_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(social_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(tariff_budget.balance_changed.connect(&BudgetMenu::update_all_projections, this));
		for (Strata const& strata : strata_keys) {
			StrataTaxBudget& strata_tax_budget = tax_budgets.emplace_back(parent, strata);
			connections.push_back(strata_tax_budget.balance_changed.connect(&BudgetMenu::update_projected_income_and_balance, this));
		}
	}

void BudgetMenu::update_projected_balance() {
	ERR_FAIL_NULL(projected_balance_label);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	fixed_point_t projected_balance = country.get_gold_income()
		+ administration_budget.get_balance()
		+ education_budget.get_balance()
		+ military_budget.get_balance()
		+ national_stockpile_budget.get_balance()
		+ social_budget.get_balance()
		+ tariff_budget.get_balance();

	for (StrataTaxBudget const& strata_tax_budget : tax_budgets) {
		projected_balance += strata_tax_budget.get_balance();
	}

	projected_balance_label->set_text(
		godot::vformat(
			godot::String::utf8("§%s%s"),
			projected_balance > 0
				? "G+"
				: projected_balance < 0
					? "R"
					: "Y+",
			Utilities::cash_to_string_dp_dynamic(projected_balance)
		)
	);
}

void BudgetMenu::update_projected_expenses() {
	ERR_FAIL_NULL(projected_expenses_label);
	fixed_point_t projected_expenses =  administration_budget.get_balance()
		+ education_budget.get_balance()
		+ military_budget.get_balance()
		+ national_stockpile_budget.get_balance()
		+ social_budget.get_balance();
		//+ import subsidies?
		//+ factory subsidies
		//+ interest
		//+ diplomatic costs

	const fixed_point_t tariff_balance = tariff_budget.get_balance();
	if (tariff_balance < fixed_point_t::_0) {
		projected_expenses += tariff_balance;
	}

	projected_expenses_label->set_text(
		Utilities::cash_to_string_dp_dynamic(-projected_expenses)
	);
}

void BudgetMenu::update_projected_expenses_and_balance() {
	update_projected_expenses();
	update_projected_balance();
}

void BudgetMenu::update_projected_income() {
	ERR_FAIL_NULL(projected_income_label);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	fixed_point_t projected_income = country.get_gold_income();
	for (StrataTaxBudget const& strata_tax_budget : tax_budgets) {
		projected_income += strata_tax_budget.get_balance();
	}
	//+ stockpile sales
	//+ diplomatic income

	const fixed_point_t tariff_balance = tariff_budget.get_balance();
	if (tariff_balance > fixed_point_t::_0) {
		projected_income += tariff_balance;
	}

	projected_income_label->set_text(
		Utilities::cash_to_string_dp_dynamic(projected_income)
	);
}

void BudgetMenu::update_projected_income_and_balance() {
	update_projected_income();
	update_projected_balance();
}

void BudgetMenu::update_all_projections() {
	update_projected_balance();
	update_projected_expenses();
	update_projected_income();
}

#define DO_FOR_ALL_COMPONENTS(F) \
	administration_budget.F; \
	education_budget.F; \
	military_budget.F; \
	national_stockpile_budget.F; \
	social_budget.F; \
	tariff_budget.F; \
	for (StrataTaxBudget& strata_tax_budget : tax_budgets) { \
		strata_tax_budget.F; \
	}

void BudgetMenu::update() {
	ERR_FAIL_NULL(administrative_efficiency_label);
	ERR_FAIL_NULL(gold_income_label);
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	administrative_efficiency_label->set_text(
		godot::vformat(
			"%s%%",
			Utilities::fixed_point_to_string_dp(
				100 * country.get_administrative_efficiency(),
				1
			)
		)
	);
	const fixed_point_t gold_income = country.get_gold_income();
	gold_income_label->set_text(
		Utilities::cash_to_string_dp_dynamic(gold_income)
	);

	//this will trigger a lot of signals we should ignore
	for (connection& c : connections) {
		c.block();
	};
	DO_FOR_ALL_COMPONENTS(update())
	for (connection& c : connections) {
		c.unblock();
	};
	update_all_projections();
}

#undef DO_FOR_ALL_COMPONENTS