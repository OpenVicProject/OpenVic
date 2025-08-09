#include "DiplomaticBudget.hpp"

#include <godot_cpp/variant/string_name.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

DiplomaticBudget::DiplomaticBudget(GUINode const& parent):
	BudgetIncomeComponent("WARINDEMNITIES_INCOME", decimal_places),
	BudgetExpenseComponent("WARINDEMNITIES_EXPENSE", decimal_places),
	balance_label{*parent.get_gui_label_from_nodepath("./country_budget/diplomatic_balance")}
{
	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/diplomatic_desc",
		"DIPLOMATIC_BALANCE_DESC", "BUDGET_DIPL_DESC"
	);
}

fixed_point_t DiplomaticBudget::get_income() const {
	return reparations_income + war_subsidies_income;
}
fixed_point_t DiplomaticBudget::get_expenses() const {
	return reparations_expenses + war_subsidies_expenses;
}

//no decimal values here just like Victoria 2
#define TOOLTIP_TEXT_PART(value, localisation_key) \
	value > 0 \
		? balance_label.tr(localisation_key).replace( \
			Utilities::get_short_value_placeholder(), \
			godot::String::num_int64(value.to_int32_t_rounded()) \
		) \
		: ""

void DiplomaticBudget::update() {
	CountryInstance* const player_country_ptr = PlayerSingleton::get_singleton()->get_player_country(
		connect_to_mark_dirty<CountryInstance*>()
	);
	if (player_country_ptr == nullptr) {
		return;
	}
	CountryInstance& country = *player_country_ptr;
	const CountryInstance::index_t index = country.get_index();

	//TODO get from sim once sim has it
	reparations_income = index+fixed_point_t::_0_50;
	war_subsidies_income = index+fixed_point_t::_1;
	reparations_expenses = index+fixed_point_t::_1;
	war_subsidies_expenses = index+fixed_point_t::_0_50;
	const fixed_point_t diplomatic_balance = reparations_income
		+ war_subsidies_income
		- reparations_expenses
		- war_subsidies_expenses;
	_balance = diplomatic_balance;

	balance_label.set_text(
		Utilities::format(
			godot::String::utf8("§%s%s§W"),
			Utilities::get_colour_and_sign(diplomatic_balance),
			Utilities::format_with_currency(
				Utilities::float_to_string_dp(diplomatic_balance, decimal_places)
			)
		)
	);

	balance_label.set_tooltip_string(
		Utilities::format(
			"%s%s%s%s", //no new lines in Victoria 2 here
			TOOLTIP_TEXT_PART(war_subsidies_expenses, "WARSUBSIDIES_EXPENSE"),
			TOOLTIP_TEXT_PART(war_subsidies_income, "WARSUBSIDIES_INCOME"),
			TOOLTIP_TEXT_PART(reparations_expenses, expenses_summary_localisation_key),
			TOOLTIP_TEXT_PART(reparations_income, income_summary_localisation_key)
		)
	);
}

#undef TOOLTIP_TEXT_PART

godot::String DiplomaticBudget::generate_income_summary_text(
	const fixed_point_t income, //ignored as this is special
	godot::Object const& translation_object
) const {
	//war_subsidies_income are excluded here in Victoria 2
	return reparations_income > 0
		? "\n"+translation_object.tr(
			income_summary_localisation_key
		).replace(
			Utilities::get_short_value_placeholder(),
			Utilities::float_to_string_dp(reparations_income, income_summary_decimal_places)
		)
		: "";
}

godot::String DiplomaticBudget::generate_expenses_summary_text(
	const fixed_point_t expenses, //ignored as this is special
	godot::Object const& translation_object
) const {
	//war_subsidies_expenses are excluded here in Victoria 2
	return reparations_expenses > 0
		? "\n"+translation_object.tr(
			expenses_summary_localisation_key
		).replace(
			Utilities::get_short_value_placeholder(),
			Utilities::float_to_string_dp(reparations_expenses, expenses_summary_decimal_places)
		)
		: "";
}

godot::String DiplomaticBudget::generate_balance_summary_income_text(godot::Object const& translation_object) const {
	return "\n"+translation_object.tr(
		income_summary_localisation_key
	).replace(
		Utilities::get_short_value_placeholder(),
		"+" + Utilities::float_to_string_dp(war_subsidies_income, income_summary_decimal_places)
	);
}

godot::String DiplomaticBudget::generate_balance_summary_expenses_text(godot::Object const& translation_object) const {
	return "\n"+translation_object.tr(
		expenses_summary_localisation_key
	).replace(
		Utilities::get_short_value_placeholder(),
		Utilities::float_to_string_dp(war_subsidies_expenses, expenses_summary_decimal_places)
	);
}