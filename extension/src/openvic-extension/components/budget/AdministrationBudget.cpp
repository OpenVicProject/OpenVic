#include "AdministrationBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"

using namespace OpenVic;

AdministrationBudget::AdministrationBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		"BUDGET_SLIDER_ADMINISTRATION",
		EXPENSES,
		"./country_budget/exp_1_slider",
		"./country_budget/exp_val_1"
	),
	BudgetExpenseComponent("BUDGET_SLIDER_ADMINISTRATION")
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);
	
	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/admin_desc",
		"ADMINISTRATION","ADM_DESC"
	);

	if (budget_label != nullptr) {
		budget_label->set_tooltip_string(
			godot::vformat(
				"%s\n--------------\n%s",
				budget_label->tr("DIST_ADMINISTRATION"),
				budget_label->tr("ADM_DESC")
			)
		);
	}
}

fixed_point_t AdministrationBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

fixed_point_t AdministrationBudget::calculate_budget_and_update_custom(
	CountryInstance const& country,
	const fixed_point_t scaled_value
) {
	return scaled_value * country.get_projected_administration_spending_unscaled_by_slider();
}

SliderValue const& AdministrationBudget::get_slider_value(CountryInstance const& country) const {
	return country.get_administration_spending_slider_value();
}

void AdministrationBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_administration_spending_slider_value(scaled_value);
}