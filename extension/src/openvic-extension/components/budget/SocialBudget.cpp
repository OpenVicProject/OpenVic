#include "SocialBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

SocialBudget::SocialBudget(GUINode const& parent):
	SliderBudgetComponent(
		parent,
		"BUDGET_SLIDER_SOCIAL_SPENDING",
		EXPENSES,
		"./country_budget/exp_2_slider",
		"./country_budget/exp_val_2"
	),
	BudgetExpenseComponent("BUDGET_SLIDER_SOCIAL_SPENDING"),
	pensions_label{*parent.get_gui_label_from_nodepath("./country_budget/exp_val_2")}, //TODO figure out how to get their path
	unemployment_subsidies_label{*parent.get_gui_label_from_nodepath("./country_budget/exp_val_2")}
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);

	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/soc_stand_desc",
		"SOCIAL_SPENDING","SOCIAL_DESC2"
	);
}

fixed_point_t SocialBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

bool SocialBudget::was_budget_cut(CountryInstance const& country) const {
	return country.get_was_social_budget_cut_yesterday();
}

fixed_point_t SocialBudget::calculate_budget_and_update_custom(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	if (budget_label != nullptr) {
		budget_label->set_tooltip_string(
			Utilities::format(
				"%s\n--------------\n%s%s",
				budget_label->tr("DIST_SOCIAL"),
				was_budget_cut(country)
					? budget_label->tr("EXPENSE_NO_AFFORD").replace(
						Utilities::get_short_value_placeholder(),
						Utilities::float_to_string_dp_dynamic(
							static_cast<float>(country.get_actual_pensions_spending().load()
							+ country.get_actual_unemployment_subsidies_spending().load())
						)
					) + "\n"
					: "",
				budget_label->tr("SOCIAL_DESC2")
			)
		);
	}

	const fixed_point_t pensions = scaled_value * country.get_projected_pensions_spending_unscaled_by_slider_untracked();
	pensions_label.set_text(
		Utilities::cash_to_string_dp_dynamic(pensions)
	);
	const fixed_point_t unemployment_subsidies = scaled_value * country.get_projected_unemployment_subsidies_spending_unscaled_by_slider_untracked();
	unemployment_subsidies_label.set_text(
		Utilities::cash_to_string_dp_dynamic(unemployment_subsidies)
	);
	return pensions + unemployment_subsidies;
}

ReadOnlyClampedValue& SocialBudget::get_clamped_value(CountryInstance& country) const {
	return country.get_social_spending_slider_value();
}

void SocialBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_social_spending_slider_value(scaled_value);
}