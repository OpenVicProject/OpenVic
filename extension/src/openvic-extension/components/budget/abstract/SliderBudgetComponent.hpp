#pragma once

#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/string.hpp>

#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/components/ReactiveComponent.hpp"

namespace OpenVic {
	struct CountryInstance;
	struct GUILabel;
	struct GUINode;
	struct GUIScrollbar;
	struct ReadOnlyClampedValue;

	enum BudgetType {
		BALANCE,
		EXPENSES
	};

	struct SliderBudgetComponent : public ReactiveComponent {
	private:
		const BudgetType budget_type; //multiplies balance by -1 for balance_label
		GUILabel* const percent_label;
		fixed_point_t _balance;

		scoped_connection player_country_connection;
		CountryInstance* player_country_cached;
		void _on_player_country_changed(CountryInstance* const player_country);

		scoped_connection slider_scaled_value_connection;
		fixed_point_t slider_scaled_value_cached;
		void _on_slider_scaled_value_changed(const fixed_point_t scaled_value);
	protected:
		const godot::String slider_tooltip_localisation_key;
		GUIScrollbar& slider;
		GUILabel* const budget_label;

		SliderBudgetComponent(
			GUINode const& parent,
			godot::String&& new_slider_tooltip_localisation_key,
			const BudgetType new_budget_type,
			godot::NodePath const& slider_path,
			godot::NodePath const& budget_label_path = {},
			godot::NodePath const& percent_label_path = {}
		);

		void initialise() override;
		virtual void update_slider_tooltip(
			CountryInstance& country,
			const fixed_point_t scaled_value
		);		

		virtual fixed_point_t calculate_budget_and_update_custom(
			CountryInstance& country,
			const fixed_point_t scaled_value
		) = 0;
		virtual ReadOnlyClampedValue& get_clamped_value(CountryInstance& country) const = 0;
		virtual void on_slider_scaled_value_changed(const fixed_point_t scaled_value) = 0;
		void update() override;
	public:
		template<typename ConnectTemplateType>
		requires std::invocable<ConnectTemplateType, signal_property<ReactiveComponent>&>
		[[nodiscard]] fixed_point_t get_balance(ConnectTemplateType&& connect) {
			connect(marked_dirty);
			update_if_dirty();
			return _balance;
		}
	};
}