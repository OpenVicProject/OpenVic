#include "BudgetMenu.hpp"

#include <cstddef>

#include <godot_cpp/core/error_macros.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

BudgetMenu::BudgetMenu(
	GUINode const& parent,
	utility::forwardable_span<const Strata> strata_keys,
	ModifierEffectCache const& modifier_effect_cache,
	CountryDefines const& country_defines
) : cash_stockpile_label{*parent.get_gui_label_from_nodepath("./country_budget/total_funds_val")},
	gold_income_label{*parent.get_gui_label_from_nodepath("./country_budget/gold_inc")},
	projected_balance_label{*parent.get_gui_label_from_nodepath("./country_budget/balance")},
	projected_expenses_label{*parent.get_gui_label_from_nodepath("./country_budget/total_exp")},
	projected_income_label{*parent.get_gui_label_from_nodepath("./country_budget/total_inc")},
	administration_budget{parent,country_defines},
	diplomatic_budget{parent},
	education_budget{parent},
	military_budget{parent},
	national_stockpile_budget{parent},
	social_budget{parent},
	tariff_budget{parent,country_defines},
	projected_income_template{generate_projected_income_template(strata_keys.size())}
	{
		tax_budgets.reserve(strata_keys.size());
		connections.reserve(7 + strata_keys.size());
		connections.push_back(administration_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(diplomatic_budget.balance_changed.connect(&BudgetMenu::update_all_projections, this));
		connections.push_back(education_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(military_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(national_stockpile_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(social_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(tariff_budget.balance_changed.connect(&BudgetMenu::update_all_projections, this));
		for (Strata const& strata : strata_keys) {
			StrataTaxBudget& strata_tax_budget = tax_budgets.emplace_back(parent, strata, modifier_effect_cache);
			connections.push_back(strata_tax_budget.balance_changed.connect(&BudgetMenu::update_projected_income_and_balance, this));
		}
		
		GUILabel::set_text_and_tooltip(
			parent, "./country_budget/gold_desc",
			"GOLD","precious_metal_desc"
		);
		GUILabel::set_text_and_tooltip(
			parent, "./country_budget/ind_sub_desc",
			"BUDGET_INDUSTRIAL_SUBSIDIES","IND_SUP_DESC"
		);

		projected_income_args.resize(5 + strata_keys.size());
		projected_expenses_args.resize(8);
	}

void BudgetMenu::update_projected_balance() {
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	fixed_point_t projected_balance = country.get_gold_income()
		+ administration_budget.get_balance()
		+ diplomatic_budget.get_balance()
		+ education_budget.get_balance()
		+ military_budget.get_balance()
		+ national_stockpile_budget.get_balance()
		+ social_budget.get_balance()
		+ tariff_budget.get_balance();

	for (StrataTaxBudget const& strata_tax_budget : tax_budgets) {
		projected_balance += strata_tax_budget.get_balance();
	}

	projected_balance_label.set_text(
		Utilities::format(
			godot::String::utf8("§%s%s§W"),
			Utilities::get_colour_and_sign(projected_balance),
			Utilities::cash_to_string_dp_dynamic(projected_balance)
		)
	);
}

void BudgetMenu::update_projected_expenses() {
	static const godot::StringName projected_expenses_template = "%s\n--------------\n%s\n%s\n%s\n%s\n%s\n%s%s";
	const fixed_point_t projected_expenses =  administration_budget.get_expenses()
		+ diplomatic_budget.get_expenses()
		+ education_budget.get_expenses()
		+ military_budget.get_expenses()
		+ national_stockpile_budget.get_expenses()
		+ social_budget.get_expenses()
		+ tariff_budget.get_expenses();
		//TODO: + import subsidies?
		//TODO: + factory subsidies
		//TODO: + interest
	const fixed_point_t interest_expenses = 0;

	projected_expenses_label.set_text(
		Utilities::cash_to_string_dp_dynamic(projected_expenses)
	);

	projected_expenses_args[0] = projected_expenses_label.tr("BUDGET_TOTAL_EXPENSE").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(projected_expenses, 3)
	);
	projected_expenses_args[1] = education_budget.generate_expenses_summary_text(projected_expenses_label);
	projected_expenses_args[2] = administration_budget.generate_expenses_summary_text(projected_expenses_label);
	projected_expenses_args[3] = social_budget.generate_expenses_summary_text(projected_expenses_label);
	projected_expenses_args[4] = military_budget.generate_expenses_summary_text(projected_expenses_label);
	projected_expenses_args[5] = projected_expenses_label.tr("BUDGET_INTEREST").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(interest_expenses, 1)
	);
	projected_expenses_args[6] = national_stockpile_budget.generate_expenses_summary_text(projected_expenses_label);
	projected_expenses_args[7] = diplomatic_budget.generate_expenses_summary_text(projected_expenses_label);
	projected_expenses_label.set_tooltip_string(
		projected_expenses_template % projected_expenses_args
	);
}

void BudgetMenu::update_projected_expenses_and_balance() {
	update_projected_expenses();
	update_projected_balance();
}

void BudgetMenu::update_projected_income() {
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	size_t i = 1;

	fixed_point_t projected_income_excluding_tariffs = 0;
	for (StrataTaxBudget const& strata_tax_budget : tax_budgets) {
		const fixed_point_t strata_tax = strata_tax_budget.get_income();
		projected_income_excluding_tariffs += strata_tax;
		projected_income_args[i++] = strata_tax_budget.generate_income_summary_text(projected_income_label);
	}

	projected_income_args[i++] = tariff_budget.generate_income_summary_text(projected_income_label);
	const fixed_point_t stockpile_income = 0; //TODO stockpile sales
	projected_income_excluding_tariffs += stockpile_income;
	projected_income_args[i++] = projected_income_label.tr("BUDGET_EXPORTS").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(stockpile_income, 1)
	);

	const fixed_point_t gold_income = country.get_gold_income();
	projected_income_excluding_tariffs += gold_income;
	projected_income_args[i++] = projected_income_label.tr("BUDGET_GOLD").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(gold_income, 1)
	);
	projected_income_args[i++] = diplomatic_budget.generate_income_summary_text(projected_income_label);

	const fixed_point_t projected_income = projected_income_excluding_tariffs + tariff_budget.get_income();
	projected_income_args[0] = projected_income_label.tr("BUDGET_TOTAL_INCOME").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(projected_income, 3)
	);
	projected_income_label.set_text(
		Utilities::cash_to_string_dp_dynamic(projected_income_excluding_tariffs)
	);
	projected_income_label.set_tooltip_string(
		projected_income_template % projected_income_args
	);

	//TODO stockpile sold to construction projects
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
	diplomatic_budget.F; \
	education_budget.F; \
	military_budget.F; \
	national_stockpile_budget.F; \
	social_budget.F; \
	tariff_budget.F; \
	for (StrataTaxBudget& strata_tax_budget : tax_budgets) { \
		strata_tax_budget.F; \
	}

void BudgetMenu::update() {
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;

	//this will trigger a lot of signals we should ignore
	for (connection& c : connections) {
		c.block();
	};
	DO_FOR_ALL_COMPONENTS(full_update(country))
	for (connection& c : connections) {
		c.unblock();
	};
	update_all_projections();

	const fixed_point_t gold_income = country.get_gold_income();
	gold_income_label.set_text(
		Utilities::format_with_currency(
			Utilities::float_to_string_dp(gold_income, 1)
		)
	);
	gold_income_label.set_tooltip_string(
		gold_income_label.tr("BUDGET_GOLD_INCOME_DESC")
		//TODO add separator & list all province names + income if gold_income > 0
	);
	const fixed_point_t cash_stockpile = country.get_cash_stockpile().get_copy_of_value();
	cash_stockpile_label.set_text(
		Utilities::format_with_currency(
			Utilities::float_to_string_suffixed(cash_stockpile)
		)
	);
}

godot::StringName BudgetMenu::generate_projected_income_template(const size_t tax_budgets_size) {
	static const std::string_view projected_income_template_start = "%s\n--------------";
	static const std::string_view projected_income_template_dynamic_part = "\n%s";

	memory::string projected_income_template;
	projected_income_template.reserve(
		projected_income_template_start.size()
		+ projected_income_template_dynamic_part.size() * (tax_budgets_size + 3)
	);

	projected_income_template.append(projected_income_template_start);
	for (size_t i = 0; i < tax_budgets_size; ++i) {
		projected_income_template.append(projected_income_template_dynamic_part);
	}
	projected_income_template += "\n%s\n%s\n%s%s";
	return godot::StringName(
		projected_income_template.data(),
		false
	);
}

#undef DO_FOR_ALL_COMPONENTS