#include "StrataTaxBudget.hpp"

#include <godot_cpp/variant/string.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include "openvic-simulation/modifier/ModifierEffectCache.hpp"
#include <openvic-simulation/pop/PopType.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

StrataTaxBudget::StrataTaxBudget(
	GUINode const& parent,
	Strata const& new_strata,
	ModifierEffectCache const& new_modifier_effect_cache
): SliderBudgetComponent(
		parent,
		generate_slider_tooltip_localisation_key(new_strata),
		BALANCE,
		Utilities::format("./country_budget/tax_%d_slider", static_cast<uint64_t>(new_strata.get_index())),
		Utilities::format("./country_budget/tax_%d_inc", static_cast<uint64_t>(new_strata.get_index()))
	),
	BudgetIncomeComponent(generate_summary_localisation_key(new_strata), 1),
	strata{new_strata},
	modifier_effect_cache{new_modifier_effect_cache}
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);

	GUILabel::set_text_and_tooltip(
		parent,
		Utilities::format("./country_budget/tax_%d_desc", static_cast<uint64_t>(new_strata.get_index())),
		slider_tooltip_localisation_key,
		Utilities::format(
			"TAX_%s_DESC",
			Utilities::std_to_godot_string(strata.get_identifier()).to_upper()
		)
	);
}

fixed_point_t StrataTaxBudget::get_income() const {
	return std::max(fixed_point_t::_0, get_balance());
}

fixed_point_t StrataTaxBudget::calculate_budget_and_update_custom(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	return scaled_value
		* country.get_taxable_income_by_strata(strata)
		* country.get_tax_efficiency_untracked();
}

ReadOnlyClampedValue& StrataTaxBudget::get_clamped_value(CountryInstance& country) const {
	return country.get_tax_rate_slider_value_by_strata(strata);
}

void StrataTaxBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_strata_tax_rate_slider_value(
		strata,
		scaled_value
	);
}

godot::StringName StrataTaxBudget::generate_slider_tooltip_localisation_key(Strata const& strata) {
	return "BUDGET_TAX_" + Utilities::std_to_godot_string(strata.get_identifier()).to_upper();
}

godot::StringName StrataTaxBudget::generate_summary_localisation_key(Strata const& strata) {
	return "TAXES_" + Utilities::std_to_godot_string(strata.get_identifier()).to_upper();
}

void StrataTaxBudget::update_slider_tooltip(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	//these use $VALUE$%

	//this uses $VALUE$ only

	const godot::String localised_strata_tax = slider.tr(slider_tooltip_localisation_key);

	const fixed_point_t tax_efficiency = country.get_tax_efficiency_untracked();
	const godot::String tax_efficiency_text = slider.tr(OV_INAME("BUDGET_TAX_EFFICIENCY")).replace(
		Utilities::get_long_value_placeholder(),
		Utilities::float_to_string_dp(100 * static_cast<float>(tax_efficiency), 2)
	);

	const fixed_point_t tax_efficiency_from_tech = country.get_modifier_effect_value(*modifier_effect_cache.get_tax_eff());
	const godot::String tax_efficiency_from_tech_text = slider.tr(OV_INAME("BUDGET_TECH_TAX")).replace(
		Utilities::get_short_value_placeholder(),
		Utilities::format(
			"%s%%",
			Utilities::float_to_string_dp(
				static_cast<float>(tax_efficiency_from_tech), //tax_efficiency_from_tech is already * 100
				2
			)
		)
	);

	const fixed_point_t effective_tax_rate = tax_efficiency * scaled_value;
	const godot::String effective_tax_rate_text = slider.tr(OV_INAME("BUDGET_TAX_EFFECT")).replace(
		Utilities::get_long_value_placeholder(),
		Utilities::float_to_string_dp(100 * static_cast<float>(effective_tax_rate), 2)
	);

	const godot::String tooltip = Utilities::format(
		godot::String::utf8("%s: §Y%s§!\n%s\n%s\n%s"),
		localised_strata_tax,
		Utilities::percentage_to_string_dp(scaled_value, 1),
		tax_efficiency_text,
		tax_efficiency_from_tech_text,
		effective_tax_rate_text
	);
	slider.set_tooltip_string(tooltip);
}