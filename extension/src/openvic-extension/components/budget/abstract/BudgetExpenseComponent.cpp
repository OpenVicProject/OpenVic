#include "BudgetExpenseComponent.hpp"

#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;

BudgetExpenseComponent::BudgetExpenseComponent(
	godot::StringName&& new_expenses_summary_localisation_key,
	const int32_t new_expenses_summary_decimal_places
) : expenses_summary_localisation_key{new_expenses_summary_localisation_key},
	expenses_summary_decimal_places{new_expenses_summary_decimal_places} {}

godot::String BudgetExpenseComponent::generate_expenses_summary_text(
	godot::Object const& translation_object
) const {
	const float expenses = static_cast<float>(get_expenses());
	return translation_object.tr(expenses_summary_localisation_key).replace(
		Utilities::get_short_value_placeholder(),
		expenses_summary_decimal_places < 0
			? Utilities::float_to_string_dp_dynamic(expenses)
			: Utilities::float_to_string_dp(expenses, expenses_summary_decimal_places)
	);
}

godot::String BudgetExpenseComponent::generate_balance_expenses_summary_text(godot::Object const& translation_object) const {
	static const godot::StringName red_minus = "R-";
	const float expenses = static_cast<float>(get_expenses());//TODO use yesterdays value
	return translation_object.tr(expenses_summary_localisation_key).replace(
		"Y$VAL$",
		red_minus + Utilities::float_to_string_dp(expenses, expenses_summary_decimal_places)
	);
}