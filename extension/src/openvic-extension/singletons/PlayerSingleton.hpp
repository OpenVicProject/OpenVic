#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/utility/Getters.hpp>

namespace OpenVic {
	struct CountryInstance;
	struct ProvinceInstance;
	struct GUIScrollbar;

	class PlayerSingleton : public godot::Object {
		GDCLASS(PlayerSingleton, godot::Object)

		static inline PlayerSingleton* singleton = nullptr;

		CountryInstance const* PROPERTY(player_country, nullptr);
		ProvinceInstance const* PROPERTY(selected_province, nullptr);

		static godot::StringName const& _signal_province_selected();

	protected:
		static void _bind_methods();

	public:
		static PlayerSingleton* get_singleton();

		PlayerSingleton();
		~PlayerSingleton();

		// Player country
		void set_player_country(CountryInstance const* new_player_country);
		void set_player_country_by_province_index(int32_t province_index);
		godot::Vector2 get_player_country_capital_position() const;

		// Selected province
		void set_selected_province(ProvinceInstance const* new_selected_province);
		void set_selected_province_by_index(int32_t province_index);
		void unset_selected_province();
		int32_t get_selected_province_index() const;

		// Core
		void toggle_paused() const;
		void increase_speed() const;
		void decrease_speed() const;

		// Production
		void expand_selected_province_building(int32_t building_index) const;

		// Budget
		void set_strata_tax_slider_value(int32_t strata, GUIScrollbar const* tax_slider);
		void set_land_spending_slider_value(GUIScrollbar const* land_spending_slider);
		void set_naval_spending_slider_value(GUIScrollbar const* naval_spending_slider);
		void set_construction_spending_slider_value(GUIScrollbar const* construction_spending_slider);
		void set_education_spending_slider_value(GUIScrollbar const* education_spending_slider);
		void set_administration_spending_slider_value(GUIScrollbar const* administration_spending_slider);
		void set_social_spending_slider_value(GUIScrollbar const* social_spending_slider);
		void set_military_spending_slider_value(GUIScrollbar const* military_spending_slider);
		void set_tariff_rate_slider_value(GUIScrollbar const* tariff_rate_slider);

		// Technology

		// Politics

		// Population

		// Trade
		void set_good_automated(int32_t good_index, bool is_automated) const;
		void set_good_trade_order(int32_t good_index, bool is_selling, GUIScrollbar const* amount_slider) const;

		// Diplomacy

		// Military
		// Argument true for general, false for admiral
		void create_leader(bool is_general) const;
		void set_can_use_leader(uint64_t leader_id, bool can_use) const;
		void set_auto_create_leaders(bool value) const;
		void set_auto_assign_leaders(bool value) const;
		void set_mobilise(bool value) const;
	};
}
