#pragma once

#include <functional>

#include <godot_cpp/classes/image.hpp>

#include "openvic2/Map.hpp"

namespace OpenVic2 {
	class MapSingleton : public godot::Object {
		GDCLASS(MapSingleton, godot::Object)

		static MapSingleton* singleton;

		godot::Ref<godot::Image> province_index_image, province_colour_image;
		Map map;
		Mapmode::index_t mapmode_index = 0;

		godot::Error _parse_province_identifier_entry(godot::String const& identifier, godot::Variant const& entry);
		godot::Error _parse_region_entry(godot::String const& identifier, godot::Variant const& entry);
	protected:
		static void _bind_methods();

	public:
		static MapSingleton* get_singleton();

		MapSingleton();
		~MapSingleton();

		godot::Error load_province_identifier_file(godot::String const& file_path);
		godot::Error load_water_province_file(godot::String const& file_path);
		godot::Error load_region_file(godot::String const& file_path);
		godot::Error load_province_shape_file(godot::String const& file_path);

		Province* get_province_from_uv_coords(godot::Vector2 const& coords);
		Province const* get_province_from_uv_coords(godot::Vector2 const& coords) const;
		int32_t get_province_index_from_uv_coords(godot::Vector2 const& coords) const;
		godot::String get_province_identifier_from_uv_coords(godot::Vector2 const& coords) const;
		godot::String get_region_identifier_from_province_identifier(godot::String const& identifier) const;
		int32_t get_width() const;
		int32_t get_height() const;
		godot::Ref<godot::Image> get_province_index_image() const;
		godot::Ref<godot::Image> get_province_colour_image() const;

		godot::Error update_colour_image();
		int32_t get_mapmode_count() const;
		godot::String get_mapmode_identifier(int32_t index) const;
		godot::Error set_mapmode(godot::String const& identifier);
	};
}
