#include "AdministrationBudget.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/defines/CountryDefines.hpp>
#include <openvic-simulation/politics/Reform.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"
#include "openvic-simulation/types/fixed_point/FixedPoint.hpp"

using namespace OpenVic;

AdministrationBudget::AdministrationBudget(
	GUINode const& parent,
	CountryDefines const& new_country_defines
):
	SliderBudgetComponent(
		parent,
		"BUDGET_SLIDER_ADMINISTRATION",
		EXPENSES,
		"./country_budget/exp_1_slider",
		"./country_budget/exp_val_1"
	),
	BudgetExpenseComponent("BUDGET_SLIDER_ADMINISTRATION"),
	country_defines{new_country_defines},
	administrative_efficiency_label{*parent.get_gui_label_from_nodepath("./country_budget/admin_efficiency")}
{
	slider.set_block_signals(true);
	slider.set_step_count(100);
	slider.set_scale(0, 1, 100);
	slider.set_block_signals(false);
	
	GUILabel::set_text_and_tooltip(
		parent, "./country_budget/admin_desc",
		"ADMINISTRATION","ADM_DESC"
	);

	administrative_efficiency_tooltip_args.resize(7);
	administrative_efficiency_tooltip_args[1] = administrative_efficiency_label.tr("BUDGET_ADMIN_EFFICIENCY_DESC2");
	administrative_efficiency_tooltip_args[4] = administrative_efficiency_label.tr("ADM_EXPLAIN_DESC");
}

fixed_point_t AdministrationBudget::get_expenses() const {
	return std::max(fixed_point_t::_0, -get_balance());
}

bool AdministrationBudget::was_budget_cut(CountryInstance const& country) const {
	return country.get_was_administration_budget_cut_yesterday();
}

fixed_point_t AdministrationBudget::calculate_budget_and_update_custom(
	CountryInstance& country,
	const fixed_point_t scaled_value
) {
	static const godot::String administrative_efficiency_template = "%s\n%s%s\n--------------\n%s%s%s%s";

	const fixed_point_t administrative_efficiency_from_administrators = country.get_administrative_efficiency_from_administrators_untracked();
	administrative_efficiency_label.set_text(
		Utilities::format(
			"%s%%",
			Utilities::fixed_point_to_string_dp(
				100 * administrative_efficiency_from_administrators,
				1
			)
		)
	);
	
	administrative_efficiency_tooltip_args[0] = administrative_efficiency_label.tr(
		"BUDGET_ADMIN_EFFICIENCY2"
	).replace(
		Utilities::get_percentage_value_placeholder(),
		Utilities::fixed_point_to_string_dp(100 * administrative_efficiency_from_administrators, 1)
	);

	if (scaled_value == 0) {
		administrative_efficiency_tooltip_args[2] = "";
	} else {
		static const godot::String admin_crime_template = godot::String::utf8("\n%s §Y%s%%§! %s");
		const fixed_point_t admin_spending_crime_effect = country_defines.get_admin_spending_crimefight_percent();
		administrative_efficiency_tooltip_args[2] = Utilities::format(
			admin_crime_template,
			administrative_efficiency_label.tr("BUDGET_VIEW_CRIME_FIGHT"),
			Utilities::fixed_point_to_string_dp(100 * admin_spending_crime_effect * scaled_value, 1),
			administrative_efficiency_label.tr(
				"PV_CRIMEFIGHT_MAX"
			).replace(
				Utilities::get_percentage_value_placeholder(),
				Utilities::fixed_point_to_string_dp(100 * admin_spending_crime_effect, 1)
			)
		);
	}

	administrative_efficiency_tooltip_args[3] = administrative_efficiency_label.tr(
		"BUDGET_ADMIN_EFFICIENCY_DESC"
	).replace(
		Utilities::get_percentage_value_placeholder(),
		Utilities::fixed_point_to_string_dp(100 * country.get_administrator_percentage_untracked(), 1)
	).replace(
		"MAX",
		Utilities::fixed_point_to_string_dp(100 * country.desired_administrator_percentage.get_untracked(), 1)
	);
	administrative_efficiency_tooltip_args[5] = administrative_efficiency_label.tr(
		"COMWID_BASE"
	).replace(
		Utilities::get_short_value_placeholder(),
		Utilities::percentage_to_string_dp(country_defines.get_max_bureaucracy_percentage(), 1)
	);

	godot::String reforms_part = "";
	for (auto const& [group, reform] : country.get_reforms()) {
		if (!group.get_is_administrative() || reform == nullptr) {
			continue;
		}

		const fixed_point_t administrative_multiplier = reform->get_administrative_multiplier();
		if (administrative_multiplier == 0) {
			continue;
		}
		
		static const godot::String reform_template = godot::String::utf8("\n%s: %s §Y+%s%%§!");

		const fixed_point_t extra_administrator_percentage = administrative_multiplier * country_defines.get_bureaucracy_percentage_increment();
		reforms_part += Utilities::format(
			reform_template,
			administrative_efficiency_label.tr(
				Utilities::std_to_godot_string(group.get_identifier())
			),
			administrative_efficiency_label.tr(
				Utilities::std_to_godot_string(reform->get_identifier())
			),
			Utilities::fixed_point_to_string_dp(100 * extra_administrator_percentage, 2)
		);
	}
	administrative_efficiency_tooltip_args[6] = reforms_part;

	administrative_efficiency_label.set_tooltip_string(
		administrative_efficiency_template % administrative_efficiency_tooltip_args
	);

	if (budget_label != nullptr) {
		budget_label->set_tooltip_string(
			Utilities::format(
				"%s\n--------------\n%s%s",
				budget_label->tr("DIST_ADMINISTRATION"),
				was_budget_cut(country)
					? budget_label->tr("EXPENSE_NO_AFFORD").replace(
						Utilities::get_short_value_placeholder(),
						Utilities::float_to_string_dp_dynamic(
							static_cast<float>(country.get_actual_administration_spending().load())
						)
					) + "\n"
					: "",
				budget_label->tr("ADM_DESC")
			)
		);
	}

	return scaled_value * country.get_projected_administration_spending_unscaled_by_slider_untracked();
}

ReadOnlyClampedValue& AdministrationBudget::get_clamped_value(CountryInstance& country) const {
	return country.get_administration_spending_slider_value();
}

void AdministrationBudget::on_slider_value_changed(const fixed_point_t scaled_value) {
	PlayerSingleton::get_singleton()->set_administration_spending_slider_value(scaled_value);
}