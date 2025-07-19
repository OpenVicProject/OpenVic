#pragma once

#include <cstdint>

#include <godot_cpp/variant/string_name.hpp>

namespace godot {
	struct Object;
}

namespace OpenVic {
	struct fixed_point_t;

	struct BudgetExpenseComponent {
	protected:
		const godot::StringName expenses_summary_localisation_key;
		const int32_t expenses_summary_decimal_places;
		
		BudgetExpenseComponent(
			godot::StringName&& new_expenses_summary_localisation_key,
			const int32_t new_expenses_summary_decimal_places = -1
		);
	public:
		virtual fixed_point_t get_expenses() const = 0;
		virtual godot::String generate_expenses_summary_text(godot::Object const& translation_object) const;
		virtual godot::String generate_balance_expenses_summary_text(godot::Object const& translation_object) const;
	};
}