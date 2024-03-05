#pragma once

#include <godot_cpp/classes/image.hpp>

#include <openvic-simulation/types/OrderedContainers.hpp>

namespace OpenVic {
	struct GameManager;

	class MenuSingleton : public godot::Object {
		GDCLASS(MenuSingleton, godot::Object)

		static inline MenuSingleton* singleton = nullptr;

		GameManager* game_manager;

	protected:
		static void _bind_methods();

	public:
		static MenuSingleton* get_singleton();

		/* This should only be called AFTER GameSingleton has been initialised! */
		MenuSingleton();
		~MenuSingleton();

		/* PROVINCE OVERVIEW PANEL */
		/* Get info to display in Province Overview Panel, packaged in a Dictionary using StringName constants as keys. */
		godot::Dictionary get_province_info_from_index(int32_t index) const;
		int32_t get_province_building_count() const;
		godot::String get_province_building_identifier(int32_t building_index) const;
		godot::Error expand_selected_province_building(int32_t building_index);
		int32_t get_slave_pop_icon_index() const;
		int32_t get_administrative_pop_icon_index() const;
		int32_t get_rgo_owner_pop_icon_index() const;

		/* TIME/SPEED CONTROL PANEL */
		void set_paused(bool paused);
		void toggle_paused();
		bool is_paused() const;
		void increase_speed();
		void decrease_speed();
		int32_t get_speed() const;
		bool can_increase_speed() const;
		bool can_decrease_speed() const;
		godot::String get_longform_date() const;
	};
}
