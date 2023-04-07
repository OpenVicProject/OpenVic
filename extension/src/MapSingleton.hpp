#pragma once

#include <godot_cpp/classes/image.hpp>

#include "openvic2/Map.hpp"

namespace OpenVic2 {
	class MapSingleton : public godot::Object {

		GDCLASS(MapSingleton, godot::Object)

		static MapSingleton* singleton;

		godot::Ref<godot::Image> province_shape_image, province_index_image, province_colour_image;
		int32_t width = 0, height = 0;
		Map map;

		godot::Error _parse_province_identifier_entry(godot::String const& identifier, godot::Variant const& entry);
		godot::Error _parse_region_entry(godot::String const& identifier, godot::Variant const& entry);
	protected:
		static void _bind_methods();

	public:
		static MapSingleton* get_singleton();

		MapSingleton();
		~MapSingleton();

		godot::Error load_province_identifier_file(godot::String const& file_path);
		godot::Error load_region_file(godot::String const& file_path);
		godot::Error load_province_shape_file(godot::String const& file_path);
		godot::String get_province_identifier_from_pixel_coords(godot::Vector2i const& coords) const;
		int32_t get_width() const;
		int32_t get_height() const;
		godot::Ref<godot::Image> get_province_index_image() const;
		godot::Ref<godot::Image> get_province_colour_image() const;
	};
}
