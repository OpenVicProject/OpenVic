#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/image.hpp>
#include "openvic2/Map.hpp"

namespace OpenVic2 {
	class MapSingleton : public godot::Object {

		GDCLASS(MapSingleton, godot::Object)

		static MapSingleton* singleton;

		godot::Ref<godot::Image> province_shape_image;
		Map map;

	protected:
		static void _bind_methods();

	public:
		static MapSingleton* get_singleton();

		MapSingleton();
		~MapSingleton();

		godot::Error load_province_identifier_file(godot::String const& file_path);
		godot::Error load_province_shape_file(godot::String const& file_path);
		godot::Ref<godot::Image> get_province_shape_image() const;
	};
}
