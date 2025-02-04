#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/utility/Getters.hpp>

namespace OpenVic {
	struct CountryInstance;
	struct ProvinceInstance;

	class PlayerSingleton : public godot::Object {
		GDCLASS(PlayerSingleton, godot::Object)

		static inline PlayerSingleton* singleton = nullptr;

		// TODO - move selected province here!

		CountryInstance const* PROPERTY(player_country, nullptr);
		ProvinceInstance const* PROPERTY(selected_province, nullptr);

		static godot::StringName const& _signal_province_selected();

	protected:
		static void _bind_methods();

	public:
		static PlayerSingleton* get_singleton();

		PlayerSingleton();
		~PlayerSingleton();

		void set_player_country(CountryInstance const* new_player_country);
		void set_player_country_by_province_index(int32_t province_index);
		godot::Vector2 get_player_country_capital_position() const;

		void set_selected_province(ProvinceInstance const* new_selected_province);
		void set_selected_province_by_index(int32_t province_index);
		void unset_selected_province();
		int32_t get_selected_province_index() const;

		void toggle_paused();
		void increase_speed();
		void decrease_speed();

		void set_auto_create_leaders(bool value) const;
		void set_auto_assign_leaders(bool value) const;

		void expand_selected_province_building(int32_t building_index);
	};
}
