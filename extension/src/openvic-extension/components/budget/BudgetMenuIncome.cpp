#include "BudgetMenuIncome.hpp"

#include <tuple>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/modifier/ModifierEffectCache.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

BudgetMenuIncome::BudgetMenuIncome(
	GUINode const& parent,
	utility::forwardable_span<const Strata> strata_keys,
	ModifierEffectCache const& modifier_effect_cache,
	DiplomaticBudget& new_diplomatic_budget,
	TariffBudget& new_tariff_budget
) : gold_income_label{*parent.get_gui_label_from_nodepath("./country_budget/gold_inc")},
	projected_income_label{*parent.get_gui_label_from_nodepath("./country_budget/total_inc")},
	diplomatic_budget{new_diplomatic_budget},
	tariff_budget{new_tariff_budget},
	projected_income_template{generate_projected_income_template(strata_keys.size())},
	tax_budgets(strata_keys.size(), [
		strata_keys,
		parent_ptr=&parent,
		modifier_effect_cache_ptr=&modifier_effect_cache
	](const size_t i)->auto {
		return std::make_tuple(parent_ptr, &strata_keys[i], modifier_effect_cache_ptr);
	})
	{
		GUILabel::set_text_and_tooltip(
			parent, "./country_budget/gold_desc",
			"GOLD","precious_metal_desc"
		);

		projected_income_args.resize(5 + strata_keys.size());
	}

void BudgetMenuIncome::update() {
	CountryInstance* const player_country_ptr = PlayerSingleton::get_singleton()->get_player_country(
		connect_to_mark_dirty<CountryInstance*>()
	);
	if (player_country_ptr == nullptr) {
		return;
	}
	CountryInstance& country = *player_country_ptr;

	const fixed_point_t gold_income = country.get_gold_income(connect_to_mark_dirty<fixed_point_t>());
	gold_income_label.set_text(
		Utilities::format_with_currency(
			Utilities::float_to_string_dp(gold_income, 1)
		)
	);
	gold_income_label.set_tooltip_string(
		gold_income_label.tr("BUDGET_GOLD_INCOME_DESC")
		//TODO add separator & list all province names + income if gold_income > 0
	);

	size_t i = 1;

	fixed_point_t projected_income_excluding_tariffs = 0;
	for (StrataTaxBudget& strata_tax_budget : tax_budgets) {
		const fixed_point_t strata_tax = strata_tax_budget.get_balance(connect_property_to_mark_dirty());
		projected_income_excluding_tariffs += strata_tax;
		projected_income_args[i++] = strata_tax_budget.generate_income_summary_text(
			strata_tax,
			projected_income_label
		);
	}
	
	const fixed_point_t tariff_income = std::max(
		fixed_point_t::_0,
		tariff_budget.get_balance(connect_property_to_mark_dirty())
	);

	projected_income_args[i++] = tariff_budget.generate_income_summary_text(
		tariff_income,
		projected_income_label
	);
	const fixed_point_t stockpile_income = 0; //TODO stockpile sales
	projected_income_excluding_tariffs += stockpile_income;
	projected_income_args[i++] = projected_income_label.tr("BUDGET_EXPORTS").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(stockpile_income, 1)
	);

	projected_income_excluding_tariffs += gold_income;
	projected_income_args[i++] = projected_income_label.tr("BUDGET_GOLD").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(gold_income, 1)
	);

	const fixed_point_t diplomatic_income = std::max( //not counted in projected_income
		fixed_point_t::_0,
		diplomatic_budget.get_balance(connect_property_to_mark_dirty())
	);
	projected_income_args[i++] = diplomatic_budget.generate_income_summary_text(
		diplomatic_income,
		projected_income_label
	);

	const fixed_point_t projected_income_including_tariffs = projected_income_excluding_tariffs + tariff_income;
	projected_income_args[0] = projected_income_label.tr("BUDGET_TOTAL_INCOME").replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(projected_income_including_tariffs, 3)
	);
	projected_income_label.set_text(
		Utilities::cash_to_string_dp_dynamic(projected_income_excluding_tariffs)
	);
	projected_income_label.set_tooltip_string(
		projected_income_template % projected_income_args
	);

	//TODO stockpile sold to construction projects

	_projected_income = projected_income_including_tariffs;
}

godot::StringName BudgetMenuIncome::generate_projected_income_template(const size_t tax_budgets_size) {
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