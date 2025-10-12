#include "TariffBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/defines/CountryDefines.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

TariffBudget::TariffBudget(
	GUINode const& parent,
	CountryDefines const& country_defines
) :
	SliderBudgetComponent {
		parent,
		"TARIFFS_INCOME",
		BALANCE,
		"./country_budget/tariff_slider",
		"./country_budget/tariff_val",
		"./country_budget/tariffs_percent"
	},
	BudgetExpenseComponent { "" }, // Victoria 2 doesn't display import subsidies
	BudgetIncomeComponent { "TARIFFS_INCOME", 1 },
	tariff_efficiency_string {
		// Not percentage_to_string_dp as localisation text contains the %
		Utilities::fixed_point_to_string_dp(
			100 * country_defines.get_base_tariff_efficiency(),
			1
		)
	}
{
	slider.set_block_signals(true);
	slider.set_step_count(200);
	slider.set_scale(fixed_point_t::minus_one, 1, 100);
	slider.set_block_signals(false);

	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/tariffs",
		"BUDGET_TARIFFS", "TARIFFS_DESC"
	);
}

fixed_point_t TariffBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

fixed_point_t TariffBudget::get_income() const {
	return std::max(fixed_point_t::_0, get_balance());
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

static const String prefix_replace_key = "PREFIX";
static const String tarrifs_replace_key = "TARIFFS";
static const String admin_replace_key = "ADMIN";

void TariffBudget::update_slider_tooltip(CountryInstance& country, const fixed_point_t scaled_value) {
	if (!slider.has_tooltip()) {
		update_slider_tooltip_localisation();
	}

	static const String green = "G";
	static const String red = "R";

	Dictionary tooltip_substitution_dict;

	// Yes Victoria 2 overrides the colour specified in localisation...
	tooltip_substitution_dict[prefix_replace_key] = scaled_value < 0 ? red : green;
	tooltip_substitution_dict[tarrifs_replace_key] = Utilities::percentage_to_string_dp(scaled_value, 1);
	// Not percentage_to_string_dp as localisation text contains the %
	tooltip_substitution_dict[admin_replace_key] = Utilities::fixed_point_to_string_dp(
		100 * scaled_value * country.tariff_efficiency.get_untracked(), 1
	);
	// Not percentage_to_string_dp as localisation text contains the %
	tooltip_substitution_dict[Utilities::get_percentage_value_placeholder()] = Utilities::fixed_point_to_string_dp(
		100 * country.get_administrative_efficiency_from_administrators_untracked(), 1
	);

	slider.set_tooltip_substitution_dict(
		tooltip_substitution_dict
	);
}

void TariffBudget::update_slider_tooltip_localisation() {
	static const String tooltip_template = "%s\n%s" + MenuSingleton::get_tooltip_separator() + "%s\n%s";
	static const String yellow_value_template_string = "Y" + Utilities::get_short_value_placeholder();
	static const StringName tariffs_localisation_key = "BUDGET_TARIFFS_MODIFIED";
	static const StringName tariff_efficiency_localisation_key = "BASE_TARIFF_EFFICIENCY";
	static const StringName admin_efficiency_localisation_key = "BUDGET_ADMIN_EFFICIENCY";
	static const String tarrifs_replace_text = GUILabel::get_substitution_marker() + prefix_replace_key +
		GUILabel::get_substitution_marker() + GUILabel::get_substitution_marker() + tarrifs_replace_key +
		GUILabel::get_substitution_marker();
	static const String admin_replace_text = GUILabel::get_substitution_marker() + prefix_replace_key +
		GUILabel::get_substitution_marker() + GUILabel::get_substitution_marker() + admin_replace_key +
		GUILabel::get_substitution_marker();

	slider.set_tooltip_string(Utilities::format(
		tooltip_template,
		slider.tr(slider_tooltip_localisation_key).replace(yellow_value_template_string, tarrifs_replace_text),
		slider.tr(tariffs_localisation_key).replace(yellow_value_template_string, admin_replace_text),
		slider.tr(tariff_efficiency_localisation_key).replace(
			Utilities::get_percentage_value_placeholder(), tariff_efficiency_string
		),
		slider.tr(admin_efficiency_localisation_key)
	));
}
