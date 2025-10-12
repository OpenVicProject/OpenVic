#include "BudgetMenu.hpp"

#include <cstddef>

#include <godot_cpp/core/error_macros.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/pop/PopType.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

static const NodePath cash_stockpile_path = "./country_budget/total_funds_val";
static const NodePath gold_income_path = "./country_budget/gold_inc";
static const NodePath projected_balance_path = "./country_budget/balance";
static const NodePath projected_expenses_path = "./country_budget/total_exp";
static const NodePath projected_income_path =  "./country_budget/total_inc";
static const NodePath gold_description_path =  "./country_budget/gold_desc";
static const NodePath industrial_subsidies_description_path =  "./country_budget/ind_sub_desc";

BudgetMenu::BudgetMenu(
	GUINode const& parent,
	decltype(tax_budgets)::keys_span_type strata_keys,
	ModifierEffectCache const& modifier_effect_cache,
	CountryDefines const& country_defines
) : cash_stockpile_label { *parent.get_gui_label_from_nodepath(cash_stockpile_path) },
	gold_income_label { *parent.get_gui_label_from_nodepath(gold_income_path) },
	projected_balance_label { *parent.get_gui_label_from_nodepath(projected_balance_path) },
	projected_expenses_label { *parent.get_gui_label_from_nodepath(projected_expenses_path) },
	projected_income_label { *parent.get_gui_label_from_nodepath(projected_income_path) },
	administration_budget { parent, country_defines },
	diplomatic_budget { parent },
	education_budget { parent },
	military_budget { parent },
	national_stockpile_budget { parent },
	social_budget { parent },
	tariff_budget { parent, country_defines },
	tax_budgets {
		strata_keys,
		[&parent, &modifier_effect_cache](Strata const& strata) -> StrataTaxBudget {
			return StrataTaxBudget { parent, strata, modifier_effect_cache };
		}
	}
	{
		connections.reserve(7 + strata_keys.size());
		connections.push_back(
			administration_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this)
		);
		connections.push_back(diplomatic_budget.balance_changed.connect(&BudgetMenu::update_all_projections, this));
		connections.push_back(
			education_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this)
		);
		connections.push_back(
			military_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this)
		);
		connections.push_back(
			national_stockpile_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this)
		);
		connections.push_back(social_budget.balance_changed.connect(&BudgetMenu::update_projected_expenses_and_balance, this));
		connections.push_back(tariff_budget.balance_changed.connect(&BudgetMenu::update_all_projections, this));
		for (StrataTaxBudget& strata_tax_budget : tax_budgets.get_values()) {
			connections.push_back(
				strata_tax_budget.balance_changed.connect(&BudgetMenu::update_projected_income_and_balance, this)
			);
		}

		static const StringName gold_text_localisation_key = "GOLD";
		static const StringName gold_tooltip_localisation_key = "precious_metal_desc";
		static const StringName industrial_subsidies_text_localisation_key = "BUDGET_INDUSTRIAL_SUBSIDIES";
		static const StringName industrial_subsidies_tooltip_localisation_key = "IND_SUP_DESC";

		GUILabel::set_text_and_tooltip(
			parent, gold_description_path, gold_text_localisation_key, gold_tooltip_localisation_key
		);
		GUILabel::set_text_and_tooltip(
			parent, industrial_subsidies_description_path, industrial_subsidies_text_localisation_key,
			industrial_subsidies_tooltip_localisation_key
		);
	}

void BudgetMenu::update_projected_balance() {
	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;
	fixed_point_t projected_balance = country.get_gold_income_untracked()
		+ administration_budget.get_balance()
		+ diplomatic_budget.get_balance()
		+ education_budget.get_balance()
		+ military_budget.get_balance()
		+ national_stockpile_budget.get_balance()
		+ social_budget.get_balance()
		+ tariff_budget.get_balance();

	for (StrataTaxBudget const& strata_tax_budget : tax_budgets.get_values()) {
		projected_balance += strata_tax_budget.get_balance();
	}

	static const String projected_balance_template_string =
		GUILabel::get_colour_marker() + String { "%s%s" } + GUILabel::get_colour_marker() + "W";

	projected_balance_label.set_text(
		Utilities::format(
			projected_balance_template_string,
			Utilities::get_colour_and_sign(projected_balance),
			Utilities::cash_to_string_dp_dynamic(projected_balance)
		)
	);
}

static StringName const& get_projected_expenses_replace_key() {
	static const StringName projected_expenses_replace_key = "PROJ";
	return projected_expenses_replace_key;
}
static StringName const& get_education_replace_key() {
	static const StringName education_replace_key = "EDU";
	return education_replace_key;
}
static StringName const& get_administration_replace_key() {
	static const StringName administration_replace_key = "ADM";
	return administration_replace_key;
}
static StringName const& get_social_replace_key() {
	static const StringName social_replace_key = "SOC";
	return social_replace_key;
}
static StringName const& get_military_replace_key() {
	static const StringName military_replace_key = "MIL";
	return military_replace_key;
}
static StringName const& get_interest_replace_key() {
	static const StringName interest_replace_key = "INT";
	return interest_replace_key;
}
static StringName const& get_national_stockpile_replace_key() {
	static const StringName national_stockpile_replace_key = "NAT";
	return national_stockpile_replace_key;
}
static StringName const& get_diplomatic_replace_key() {
	static const StringName diplomatic_replace_key = "DIP";
	return diplomatic_replace_key;
}

void BudgetMenu::update_projected_expenses_tooltip_localisation() {
	static const String projected_expenses_template_template = "%s" + MenuSingleton::get_tooltip_separator() +
		GUILabel::get_substitution_marker() + get_education_replace_key() + GUILabel::get_substitution_marker() + "\n" +
		GUILabel::get_substitution_marker() + get_administration_replace_key() + GUILabel::get_substitution_marker() + "\n" +
		GUILabel::get_substitution_marker() + get_social_replace_key() + GUILabel::get_substitution_marker() + "\n" +
		GUILabel::get_substitution_marker() + get_military_replace_key() + GUILabel::get_substitution_marker() + "\n%s\n" +
		GUILabel::get_substitution_marker() + get_national_stockpile_replace_key() + GUILabel::get_substitution_marker() +
		GUILabel::get_substitution_marker() + get_diplomatic_replace_key() + GUILabel::get_substitution_marker();
	static const StringName projected_expenses_localisation_key = "BUDGET_TOTAL_EXPENSE";
	static const String projected_expenses_replace_string = GUILabel::get_substitution_marker() +
		get_projected_expenses_replace_key() + GUILabel::get_substitution_marker();
	static const StringName interest_expenses_localisation_key = "BUDGET_INTEREST";
	static const String interest_replace_string = GUILabel::get_substitution_marker() + get_interest_replace_key() +
		GUILabel::get_substitution_marker();

	projected_expenses_label.set_tooltip_string(
		Utilities::format(
			projected_expenses_template_template,
			projected_expenses_label.tr(projected_expenses_localisation_key).replace(
				Utilities::get_short_value_placeholder(), projected_expenses_replace_string
			),
			projected_expenses_label.tr(interest_expenses_localisation_key).replace(
				Utilities::get_short_value_placeholder(), interest_replace_string
			)
		)
	);
}

void BudgetMenu::update_projected_expenses() {
	if (!projected_expenses_label.has_tooltip()) {
		update_projected_expenses_tooltip_localisation();
	}

	const fixed_point_t projected_expenses =  administration_budget.get_expenses()
		+ diplomatic_budget.get_expenses()
		+ education_budget.get_expenses()
		+ military_budget.get_expenses()
		+ national_stockpile_budget.get_expenses()
		+ social_budget.get_expenses()
		+ tariff_budget.get_expenses();
		// TODO: + import subsidies?
		// TODO: + factory subsidies
		// TODO: + interest
	const fixed_point_t interest_expenses = 0;

	projected_expenses_label.set_text(
		Utilities::cash_to_string_dp_dynamic(projected_expenses)
	);

	projected_expenses_label.add_substitution(
		get_projected_expenses_replace_key(), Utilities::float_to_string_dp(projected_expenses, 3)
	);
	projected_expenses_label.add_substitution(
		get_education_replace_key(), education_budget.generate_expenses_summary_text(projected_expenses_label)
	);
	projected_expenses_label.add_substitution(
		get_administration_replace_key(), administration_budget.generate_expenses_summary_text(projected_expenses_label)
	);
	projected_expenses_label.add_substitution(
		get_social_replace_key(), social_budget.generate_expenses_summary_text(projected_expenses_label)
	);
	projected_expenses_label.add_substitution(
		get_military_replace_key(), military_budget.generate_expenses_summary_text(projected_expenses_label)
	);
	projected_expenses_label.add_substitution(
		get_interest_replace_key(), Utilities::float_to_string_dp(interest_expenses, 1)
	);
	projected_expenses_label.add_substitution(
		get_national_stockpile_replace_key(),
		national_stockpile_budget.generate_expenses_summary_text(projected_expenses_label)
	);
	projected_expenses_label.add_substitution(
		get_diplomatic_replace_key(), diplomatic_budget.generate_expenses_summary_text(projected_expenses_label)
	);
}

void BudgetMenu::update_projected_expenses_and_balance() {
	update_projected_expenses();
	update_projected_balance();
}

static StringName const& get_projected_income_replace_key() {
	static const StringName projected_income_replace_key = "PROJ";
	return projected_income_replace_key;
}
static StringName const& get_income_summary_replace_key() {
	static const StringName income_summary_replace_key = "PROJ";
	return income_summary_replace_key;
}
static StringName const& get_exports_replace_key() {
	static const StringName exports_replace_key = "PROJ";
	return exports_replace_key;
}
static StringName const& get_gold_replace_key() {
	static const StringName gold_replace_key = "PROJ";
	return gold_replace_key;
}
static StringName const& get_diplomatic_replace_key() {
	static const StringName diplomatic_replace_key = "PROJ";
	return diplomatic_replace_key;
}

// tmp-changes

void BudgetMenu::update_projected_income_localisation() {
	static const String projected_income_template_start = GUILabel::get_substitution_marker() +
		get_projected_income_replace_key() + GUILabel::get_substitution_marker() + MenuSingleton::get_tooltip_separator();
	static const String projected_income_template_dynamic_part = "\n" + GUILabel::get_substitution_marker() + "%d" +
		GUILabel::get_substitution_marker();
	static const String projected_income_template_end = GUILabel::get_substitution_marker() +
		get_income_summary_replace_key() + GUILabel::get_substitution_marker() + "\n" + GUILabel::get_substitution_marker() +
		"%s" + GUILabel::get_substitution_marker() + "\n" + GUILabel::get_substitution_marker() + "%s" +
		GUILabel::get_substitution_marker() + GUILabel::get_substitution_marker() + "%s" + GUILabel::get_substitution_marker();

	String projected_income_template = projected_income_template_start;
	for (size_t strata_index = 0; strata_index < tax_budgets.get_count(); ++strata_index) {
		projected_income_template += Utilities::format(projected_income_template_dynamic_part, strata_index);
	}
	projected_income_template += projected_income_template_end;

	projected_income_label.set_tooltip_string(projected_income_template);
}

void BudgetMenu::update_projected_income() {
	if (!projected_income_label.has_tooltip()) {
		update_projected_income_localisation();
	}

	CountryInstance const* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance const& country = *country_ptr;

	fixed_point_t projected_income_excluding_tariffs = 0;
	for (StrataTaxBudget const& strata_tax_budget : tax_budgets.get_values()) {
		const fixed_point_t strata_tax = strata_tax_budget.get_income();
		projected_income_excluding_tariffs += strata_tax;
		projected_income_label.add_substitution(
			String::num_uint64(strata_tax_budget.get_strata().get_index()),
			strata_tax_budget.generate_income_summary_text(projected_income_label)
		);
	}

	static const StringName exports_localisation_key = "BUDGET_EXPORTS";
	static const StringName gold_localisation_key = "BUDGET_GOLD";
	static const StringName total_income_localisation_key = "BUDGET_TOTAL_INCOME";

	projected_income_args[i++] = tariff_budget.generate_income_summary_text(projected_income_label);
	const fixed_point_t stockpile_income = 0; // TODO - stockpile sales
	projected_income_excluding_tariffs += stockpile_income;
	projected_income_args[i++] = projected_income_label.tr(exports_localisation_key).replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(stockpile_income, 1)
	);

	const fixed_point_t gold_income = country.get_gold_income_untracked();
	projected_income_excluding_tariffs += gold_income;
	projected_income_args[i++] = projected_income_label.tr(gold_localisation_key).replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(gold_income, 1)
	);
	projected_income_args[i++] = diplomatic_budget.generate_income_summary_text(projected_income_label);

	const fixed_point_t projected_income = projected_income_excluding_tariffs + tariff_budget.get_income();
	projected_income_label.add_substitution(
		get_projected_income_replace_key(),
		projected_income_label.tr(total_income_localisation_key).replace(
			Utilities::get_short_value_placeholder(),
			Utilities::float_to_string_dp(projected_income, 3)
		)
	);
	projected_income_label.set_text(
		Utilities::cash_to_string_dp_dynamic(projected_income_excluding_tariffs)
	);

	// TODO - stockpile sold to construction projects
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

void BudgetMenu::update_gold_income_description_localisation() {
	static const StringName gold_income_description_localisation_key = "BUDGET_GOLD_INCOME_DESC";
	gold_income_label.set_tooltip_string(gold_income_label.tr(gold_income_description_localisation_key));
}

void BudgetMenu::update() {
	CountryInstance* const country_ptr = PlayerSingleton::get_singleton()->get_player_country();
	ERR_FAIL_NULL(country_ptr);
	CountryInstance& country = *country_ptr;

	// This will trigger a lot of signals we should ignore
	for (connection& c : connections) {
		c.block();
	};
	DO_FOR_ALL_COMPONENTS(full_update(country))
	for (connection& c : connections) {
		c.unblock();
	};
	update_all_projections();

	const fixed_point_t gold_income = country.get_gold_income_untracked();
	gold_income_label.set_text(
		Utilities::format_with_currency(
			Utilities::float_to_string_dp(gold_income, 1)
		)
	);
	if (!gold_income_label.has_tooltip()) {
		localise_gold_income_description();
	}
	// TODO - gold_income_label - add separator & list all province names + income if gold_income > 0
	const fixed_point_t cash_stockpile = country.get_cash_stockpile();
	cash_stockpile_label.set_text(
		Utilities::format_with_currency(
			Utilities::float_to_string_suffixed(cash_stockpile)
		)
	);
}

#undef DO_FOR_ALL_COMPONENTS

void BudgetMenu::update_localisation() {
	tariff_budget.update_slider_tooltip_localisation();
	update_projected_expenses_tooltip_localisation();
	update_gold_income_description_localisation();
	update_projected_income_localisation();
}
