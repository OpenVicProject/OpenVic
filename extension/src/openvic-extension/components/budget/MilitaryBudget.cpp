#include "MilitaryBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

MilitaryBudget::MilitaryBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		"BUDGET_SLIDER_MILITARY_SPENDING",
		EXPENSES,
		"./country_budget/exp_3_slider",
		"./country_budget/exp_val_3"
	),
	BudgetExpenseComponent("BUDGET_SLIDER_MILITARY_SPENDING")
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);

	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/mil_spend_desc",
		"MILITARY_SPENDING","DEFENCE_DESC"
	);
}

fixed_point_t MilitaryBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

bool MilitaryBudget::was_budget_cut(CountryInstance const& country) const {
	return country.get_was_military_budget_cut_yesterday();
}

fixed_point_t MilitaryBudget::calculate_budget_and_update_custom(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	if (budget_label != nullptr) {
		budget_label->set_tooltip_string(
			Utilities::format(
				"%s\n--------------\n%s%s",
				budget_label->tr("DIST_DEFENCE"),
				was_budget_cut(country)
					? budget_label->tr("EXPENSE_NO_AFFORD").replace(
						Utilities::get_short_value_placeholder(),
						Utilities::float_to_string_dp_dynamic(
							country.get_actual_military_spending().load()
						)
					) + "\n"
					: "",
				budget_label->tr("DEFENCE_DESC")
			)
		);
	}

	return scaled_value * country.get_projected_military_spending_unscaled_by_slider_untracked();
}

ReadOnlyClampedValue& MilitaryBudget::get_clamped_value(CountryInstance& country) const {
	return country.get_military_spending_slider_value();
}

void MilitaryBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_military_spending_slider_value(scaled_value);
}