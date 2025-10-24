#include "EducationBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

EducationBudget::EducationBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		"BUDGET_EXPENSE_SLIDER_EDUCATION",
		EXPENSES,
		"./country_budget/exp_0_slider",
		"./country_budget/exp_val_0"
	),
	BudgetExpenseComponent("BUDGET_EXPENSE_SLIDER_EDUCATION")
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);

	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/education_desc",
		"EDUCATION","EDU_DESC"
	);
}

fixed_point_t EducationBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

bool EducationBudget::was_budget_cut(CountryInstance const& country) const {
	return country.get_was_education_budget_cut_yesterday();
}

fixed_point_t EducationBudget::calculate_budget_and_update_custom(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	if (budget_label != nullptr) {
		budget_label->set_tooltip_string(
			Utilities::format(
				"%s\n--------------\n%s%s",
				budget_label->tr("DIST_EDUCATION"),
				was_budget_cut(country)
					? budget_label->tr("EXPENSE_NO_AFFORD").replace(
						Utilities::get_short_value_placeholder(),
						Utilities::float_to_string_dp_dynamic(
							country.get_actual_education_spending().load()
						)
					) + "\n"
					: "",
				budget_label->tr("EDU_DESC")
			)
		);
	}

	return scaled_value * country.get_projected_education_spending_unscaled_by_slider_untracked();
}

ReadOnlyClampedValue& EducationBudget::get_clamped_value(CountryInstance& country) const {
	return country.get_education_spending_slider_value();
}

void EducationBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_education_spending_slider_value(scaled_value);
}