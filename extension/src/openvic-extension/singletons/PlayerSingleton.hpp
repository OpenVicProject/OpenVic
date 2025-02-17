#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/utility/Getters.hpp>

namespace OpenVic {
	struct CountryInstance;
	struct ProvinceInstance;

	class PlayerSingleton : public godot::Object {
		GDCLASS(PlayerSingleton, godot::Object)

		static inline PlayerSingleton* singleton = nullptr;

		CountryInstance* PROPERTY_PTR(player_country, nullptr);
		ProvinceInstance* PROPERTY_PTR(selected_province, nullptr);

		static godot::StringName const& _signal_province_selected();

	protected:
		static void _bind_methods();

	public:
		static PlayerSingleton* get_singleton();

		PlayerSingleton();
		~PlayerSingleton();

		// Player country
		void set_player_country(CountryInstance* new_player_country);
		void set_player_country_by_province_index(int32_t province_index);
		godot::Vector2 get_player_country_capital_position() const;

		// Selected province
		void set_selected_province(ProvinceInstance* new_selected_province);
		void set_selected_province_by_index(int32_t province_index);
		void unset_selected_province();
		int32_t get_selected_province_index() const;
	};
}
