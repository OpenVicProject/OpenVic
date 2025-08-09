#pragma once

#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>
#include <openvic-simulation/types/FixedVector.hpp>
#include <openvic-simulation/utility/ForwardableSpan.hpp>

#include "openvic-extension/components/budget/StrataTaxBudget.hpp"
#include "openvic-extension/components/ReactiveComponent.hpp"

namespace OpenVic {
	struct CountryInstance;
	struct DiplomaticBudget;
	struct GUINode;
	struct GUILabel;
	struct TariffBudget;

	struct BudgetMenuIncome : public ReactiveComponent {
	private:
		static godot::StringName generate_projected_income_template(const size_t tax_budgets_size);

		const godot::StringName projected_income_template;
		godot::Array projected_income_args;
		GUILabel& gold_income_label;
		GUILabel& projected_income_label;
		DiplomaticBudget& diplomatic_budget;
		TariffBudget& tariff_budget;
		memory::FixedVector<StrataTaxBudget> tax_budgets;

		fixed_point_t _projected_income;

		void update() override;
	public:
		BudgetMenuIncome(
			GUINode const& parent,
			utility::forwardable_span<const Strata> strata_keys,
			ModifierEffectCache const& modifier_effect_cache,
			DiplomaticBudget& new_diplomatic_budget,
			TariffBudget& new_tariff_budget
		);

		template<typename ConnectTemplateType>
		requires std::invocable<ConnectTemplateType, signal_property<ReactiveComponent>&>
		[[nodiscard]] fixed_point_t get_projected_income(ConnectTemplateType&& connect) {
			connect(marked_dirty);
			update_if_dirty();
			return _projected_income;
		}
	};
}