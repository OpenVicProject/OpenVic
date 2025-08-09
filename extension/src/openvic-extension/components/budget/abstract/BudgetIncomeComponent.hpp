#pragma once

#include <cstdint>

#include <godot_cpp/variant/string_name.hpp>

namespace godot {
	struct Object;
}

namespace OpenVic {
	struct fixed_point_t;

	struct BudgetIncomeComponent {
	protected:
		const godot::StringName income_summary_localisation_key;
		int32_t income_summary_decimal_places;
		
		BudgetIncomeComponent(
			godot::StringName&& new_income_summary_localisation_key,
			const int32_t new_income_summary_decimal_places = -1
		);
	public:
		virtual godot::String generate_income_summary_text(
			const fixed_point_t income,
			godot::Object const& translation_object
		) const;
		virtual godot::String generate_balance_income_summary_text(
			const fixed_point_t income,
			godot::Object const& translation_object
		) const;
	};
}