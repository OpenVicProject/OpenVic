#include "MenuSingleton.hpp"

using namespace OpenVic;
using namespace godot;

Dictionary MenuSingleton::get_budget_menu_setup_info() const {
	static const StringName _key = "";

	Dictionary dict;

	return dict;
}

Dictionary MenuSingleton::get_budget_menu_info() const {
	static const StringName tax_info_by_strata_key = "tax_info_by_strata";
	// TODO - needs tooltip per pop type, or marker to disable their button and use tooltip: "there are no pops of this type"

	// TODO - gold, total income
	// TODO - national bank, total funds, total debt, interest, loans taken and given

	static const StringName industrial_subsidies_key = "industrial_subsidies";
	static const StringName industrial_subsidies_tooltip_key = "industrial_subsidies_tooltip";

	static const StringName military_costs_key = "military_costs";
	static const StringName military_costs_tooltip_key = "military_costs_tooltip";
	static const StringName overseas_maintenance_key = "overseas_maintenance";
	static const StringName overseas_maintenance_tooltip_key = "overseas_maintenance_tooltip";

	static const StringName national_stockpile_today_key = "national_stockpile_today";
	static const StringName national_stockpile_today_tooltip_key = "national_stockpile_today_tooltip";
	static const StringName national_stockpile_tomorrow_key = "national_stockpile_tomorrow";
	static const StringName national_stockpile_tomorrow_tooltip_key = "national_stockpile_tomorrow_tooltip";
	static const StringName land_spending_slider_key = "land_spending_slider";
	static const StringName land_spending_slider_tooltip_key = "land_spending_slider_tooltip";
	static const StringName naval_spending_slider_key = "naval_spending_slider";
	static const StringName naval_spending_slider_tooltip_key = "naval_spending_slider_tooltip";
	static const StringName construction_spending_slider_key = "construction_spending_slider";
	static const StringName construction_spending_slider_tooltip_key = "construction_spending_slider_tooltip";

	// sliders may need tooltips for when they have custom min/max from modifiers!
	static const StringName education_spending_slider_key = "education_spending_slider";
	static const StringName education_spending_value_key = "education_spending_value";

	static const StringName administrative_efficiency_key = "administrative_efficiency";
	static const StringName administrative_efficiency_tooltip_key = "administrative_efficiency_tooltip";
	static const StringName administration_spending_slider_key = "administration_spending_slider";
	static const StringName administration_spending_value_key = "administration_spending_value";

	static const StringName social_spending_slider_key = "social_spending_slider";
	static const StringName social_spending_value_key = "social_spending_value";
	static const StringName unemployment_subsidies_key = "unemployment_subsidies";
	static const StringName pensions_key = "pensions";

	static const StringName military_spending_slider_key = "military_spending_slider";
	static const StringName military_spending_slider_tooltip_key = "military_spending_slider_tooltip";
	static const StringName military_spending_value_key = "military_spending_value";
	// double check these are hardcoded and not dynamic based on military pop types!
	static const StringName officers_key = "officers";
	static const StringName soldiers_key = "soldiers";

	static const StringName total_expense_key = "total_expense";
	static const StringName total_expense_tooltip_key = "total_expense_tooltip";

	static const StringName tariff_slider_key = "tariff_slider";
	static const StringName tariff_slider_tooltip_key = "tariff_slider_tooltip";
	static const StringName tariff_value_key = "tariff_value";

	// TODO - diplomatic balance, projected daily balance

	// TODO - per strata
	static const StringName tax_slider_pie_chart_key = "tax_slider_pie_chart";
	static const StringName tax_slider_key = "tax_slider";

	// slider data uses Vector3i (min, value, max)

	Dictionary dict;

	return dict;
}
