#include "BudgetMenu.hpp"

#include <godot_cpp/core/error_macros.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/components/budget/BudgetMenuExpenses.hpp"
#include "openvic-extension/components/budget/BudgetMenuIncome.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

BudgetMenu::BudgetMenu(
	GUINode const& parent,
	utility::forwardable_span<const Strata> strata_keys,
	ModifierEffectCache const& modifier_effect_cache,
	CountryDefines const& country_defines
) : cash_stockpile_label{*parent.get_gui_label_from_nodepath("./country_budget/total_funds_val")},
	projected_balance_label{*parent.get_gui_label_from_nodepath("./country_budget/balance")},
	diplomatic_budget{parent},
	tariff_budget{parent,country_defines},
	expenses_component{parent, country_defines, diplomatic_budget, tariff_budget},
	income_component{parent, strata_keys, modifier_effect_cache, diplomatic_budget, tariff_budget}
{
	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/ind_sub_desc",
		"BUDGET_INDUSTRIAL_SUBSIDIES","IND_SUP_DESC"
	);
}

void BudgetMenu::update() {
	CountryInstance* const player_country_ptr = PlayerSingleton::get_singleton()->get_player_country(
		connect_to_mark_dirty<CountryInstance*>()
	);
	if (player_country_ptr == nullptr) {
		return;
	}
	CountryInstance& country = *player_country_ptr;

	const fixed_point_t projected_balance = income_component.get_projected_income(connect_property_to_mark_dirty())
		- expenses_component.get_projected_expenses(connect_property_to_mark_dirty());

	projected_balance_label.set_text(
		Utilities::format(
			godot::String::utf8("§%s%s§W"),
			Utilities::get_colour_and_sign(projected_balance),
			Utilities::cash_to_string_dp_dynamic(projected_balance)
		)
	);

	add_connection(GameSingleton::get_singleton()->gamestate_updated.connect(&BudgetMenu::mark_dirty, this)); //for cash stockpile
	const fixed_point_t cash_stockpile = country.get_cash_stockpile();
	cash_stockpile_label.set_text(
		Utilities::format_with_currency(
			Utilities::float_to_string_suffixed(cash_stockpile)
		)
	);
}