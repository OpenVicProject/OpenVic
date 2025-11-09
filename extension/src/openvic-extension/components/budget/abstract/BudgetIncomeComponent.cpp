#include "BudgetIncomeComponent.hpp"

#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

BudgetIncomeComponent::BudgetIncomeComponent(
	godot::StringName&& new_income_summary_localisation_key,
	const int32_t new_income_summary_decimal_places
) : income_summary_localisation_key{new_income_summary_localisation_key},
	income_summary_decimal_places{new_income_summary_decimal_places} {}

godot::String BudgetIncomeComponent::generate_income_summary_text(
	godot::Object const& translation_object
) const {
	const float income = static_cast<float>(get_income());
	return translation_object.tr(income_summary_localisation_key).replace(
		Utilities::get_short_value_placeholder(),
		income_summary_decimal_places < 0
			? Utilities::float_to_string_dp_dynamic(income)
			: Utilities::float_to_string_dp(income, income_summary_decimal_places)
	);
}

godot::String BudgetIncomeComponent::generate_balance_income_summary_text(godot::Object const& translation_object) const {
	const float income = static_cast<float>(get_income()); //TODO use yesterdays value
	return translation_object.tr(income_summary_localisation_key).replace(
		"Y$VAL$",
		"G+" + Utilities::float_to_string_dp(income, income_summary_decimal_places)
	);
}