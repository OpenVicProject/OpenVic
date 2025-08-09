#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>
#include <openvic-simulation/utility/ForwardableSpan.hpp>

#include "openvic-extension/components/budget/AdministrationBudget.hpp"
#include "openvic-extension/components/budget/EducationBudget.hpp"
#include "openvic-extension/components/budget/MilitaryBudget.hpp"
#include "openvic-extension/components/budget/NationalStockpileBudget.hpp"
#include "openvic-extension/components/budget/SocialBudget.hpp"
#include "openvic-extension/components/ReactiveComponent.hpp"

namespace OpenVic {
	struct CountryInstance;
	struct DiplomaticBudget;
	struct GUINode;
	struct GUILabel;
	struct TariffBudget;

	struct BudgetMenuExpenses : public ReactiveComponent {
	private:
		godot::Array projected_expenses_args;
		GUILabel& projected_expenses_label;
		AdministrationBudget administration_budget;
		DiplomaticBudget& diplomatic_budget;
		EducationBudget education_budget;
		MilitaryBudget military_budget;
		NationalStockpileBudget national_stockpile_budget;
		SocialBudget social_budget;
		TariffBudget& tariff_budget;

		fixed_point_t _projected_expenses;

		void update() override;
	public:
		BudgetMenuExpenses(
			GUINode const& parent,
			CountryDefines const& country_defines,
			DiplomaticBudget& new_diplomatic_budget,
			TariffBudget& new_tariff_budget
		);

		template<typename ConnectTemplateType>
		requires std::invocable<ConnectTemplateType, signal_property<ReactiveComponent>&>
		[[nodiscard]] fixed_point_t get_projected_expenses(ConnectTemplateType&& connect) {
			connect(marked_dirty);
			update_if_dirty();
			return _projected_expenses;
		}
	};
}