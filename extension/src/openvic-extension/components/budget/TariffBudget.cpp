#include "TariffBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/defines/CountryDefines.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

TariffBudget::TariffBudget(
	GUINode const& parent,
	CountryDefines const& country_defines
) :
	SliderBudgetComponent(
		parent,
		"TARIFFS_INCOME",
		BALANCE,
		"./country_budget/tariff_slider",
		"./country_budget/tariff_val",
		"./country_budget/tariffs_percent"
	),
	BudgetExpenseComponent(""), //Victoria 2 doesn't display import subsidies
	BudgetIncomeComponent("TARIFFS_INCOME", 1)
{
	slider.set_block_signals(true);
	slider.set_step_count(200);
	slider.set_scale(fixed_point_t::minus_one, 1, 100);
	slider.set_block_signals(false);

	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/tariffs",
		"BUDGET_TARIFFS", "TARIFFS_DESC"
	);
	slider_tooltip_args.resize(4);
	slider_tooltip_args[2] = parent.tr("BASE_TARIFF_EFFICIENCY").replace(
		Utilities::get_percentage_value_placeholder(),
		//not percentage_to_string_dp as localisation text contains the %
		Utilities::fixed_point_to_string_dp(
			100 * country_defines.get_base_tariff_efficiency(),
			1
		)
	);
}

fixed_point_t TariffBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

fixed_point_t TariffBudget::get_income() const {
	return std::max(fixed_point_t::_0, get_balance());
}

bool TariffBudget::was_budget_cut(CountryInstance const& country) const {
	return country.get_was_import_subsidies_budget_cut_yesterday();
}

fixed_point_t TariffBudget::calculate_budget_and_update_custom(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	return scaled_value
		* country.get_yesterdays_import_value_untracked()
		* country.tariff_efficiency.get_untracked();
}

ReadOnlyClampedValue& TariffBudget::get_clamped_value(CountryInstance& country) const {
	return country.get_tariff_rate_slider_value();
}

void TariffBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_tariff_rate_slider_value(scaled_value);
}

void TariffBudget::update_slider_tooltip(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	static const godot::String tooltip_template = "%s\n%s\n--------------\n%s\n%s";
	//Yes Victoria 2 overrides the colour specified in localisation...
	godot::String prefix = scaled_value < 0 ? "R" : "G";

	slider_tooltip_args[0] = slider.tr(slider_tooltip_localisation_key).replace(
		"Y"+Utilities::get_short_value_placeholder(),
		prefix+Utilities::percentage_to_string_dp(
			scaled_value,
			1
		)
	);
	
	slider_tooltip_args[1] = slider.tr("BUDGET_TARIFFS_MODIFIED").replace(
		"Y"+Utilities::get_percentage_value_placeholder(),
		//not percentage_to_string_dp as localisation text contains the %
		prefix+Utilities::fixed_point_to_string_dp(
			100 * scaled_value * country.tariff_efficiency.get_untracked(),
			1
		)
	);

	slider_tooltip_args[3] = slider.tr("BUDGET_ADMIN_EFFICIENCY").replace(
		Utilities::get_percentage_value_placeholder(),
		//not percentage_to_string_dp as localisation text contains the %
		Utilities::fixed_point_to_string_dp(
			100 * country.get_administrative_efficiency_from_administrators_untracked(),
			1
		)
	);
	slider.set_tooltip_string(tooltip_template % slider_tooltip_args);
}