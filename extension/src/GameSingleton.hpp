#pragma once

#include <functional>

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/texture2d_array.hpp>

#include "openvic2/GameManager.hpp"

namespace OpenVic2 {
	class GameSingleton : public godot::Object {
		GDCLASS(GameSingleton, godot::Object)

		static GameSingleton* singleton;

		GameManager game_manager;

		godot::Vector2i image_subdivisions;
		godot::Ref<godot::Texture2DArray> province_shape_texture;
		godot::Ref<godot::Image> province_colour_image;
		godot::Ref<godot::ImageTexture> province_colour_texture;
		Mapmode::index_t mapmode_index = 0;

		godot::Error _parse_province_identifier_entry(godot::String const& identifier, godot::Variant const& entry);
		godot::Error _parse_region_entry(godot::String const& identifier, godot::Variant const& entry);
		void _tick();
	protected:
		static void _bind_methods();

	public:
		static GameSingleton* get_singleton();

		GameSingleton();
		~GameSingleton();

		godot::Error load_province_identifier_file(godot::String const& file_path);
		godot::Error load_water_province_file(godot::String const& file_path);
		godot::Error load_region_file(godot::String const& file_path);
		godot::Error load_province_shape_file(godot::String const& file_path);
		godot::Error setup();

		int32_t get_province_index_from_uv_coords(godot::Vector2 const& coords) const;
		godot::Dictionary get_province_info_from_index(int32_t index) const;
		int32_t get_width() const;
		int32_t get_height() const;
		float get_aspect_ratio() const;
		godot::Vector2i get_province_shape_image_subdivisions() const;
		godot::Ref<godot::Texture> get_province_shape_texture() const;
		godot::Ref<godot::Texture> get_province_colour_texture() const;

		godot::Error update_colour_image();
		int32_t get_mapmode_count() const;
		godot::String get_mapmode_identifier(int32_t index) const;
		godot::Error set_mapmode(godot::String const& identifier);

		godot::Error expand_building(int32_t province_index, godot::String const& building_type_identifier);

		void set_paused(bool paused);
		void toggle_paused();
		bool is_paused() const;
		void increase_speed();
		void decrease_speed();
		bool can_increase_speed() const;
		bool can_decrease_speed() const;
		godot::String get_longform_date() const;
		void try_tick();
	};
}
