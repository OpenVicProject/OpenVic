#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <openvic-extension/singletons/GameSingleton.hpp>
#include <openvic-simulation/dataloader/Dataloader.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>
#include <openvic-simulation/types/IdentifierRegistry.hpp>

namespace OpenVic {

	class CursorSingleton : public godot::Object {
		
		GDCLASS(CursorSingleton, godot::Object);
		static inline CursorSingleton* _singleton = nullptr;

	public:
		//An intermediate data type to help with loading cursors.
		//The size of images/hotspots arrays corresponds to resolutions_per_cursor.
		struct image_hotspot_pair_asset_t {
			std::vector<godot::Vector2i> hotspots;
			std::vector<godot::Ref<godot::ImageTexture>> images;
		};

	private:
		//.cur files use all but the last 2 properties, rest are for .ani
		struct cursor_asset_t {
			std::vector<godot::PackedVector2Array> hotspots;
			std::vector<godot::TypedArray<godot::ImageTexture>> images;
			godot::PackedVector2Array resolutions;
			int32_t animation_length; //1 for static cursors
			godot::PackedFloat32Array display_rates;
			godot::PackedInt32Array sequence;
		};

		//map of "subfolder/fileName.cur/.ani" -> cursor_asset. Subfolder comes after gfx/cursor
		using cursor_map_t = godot::HashMap<godot::StringName, cursor_asset_t>;
		cursor_map_t cursors;

		godot::TypedArray<godot::StringName> cursor_names;
		

	public:
		CursorSingleton();
		~CursorSingleton();
		static CursorSingleton* get_singleton();
	
	protected:
		static void _bind_methods();

	private:
		bool _load_cursor_ani(godot::StringName const& name, godot::String const& path);
		bool _load_cursor_cur(godot::StringName const& name, godot::String const& path);

	public:
		godot::Error load_cursors();
		godot::TypedArray<godot::StringName> get_cursor_names() const;
		
		godot::TypedArray<godot::ImageTexture> get_frames(godot::StringName const& name, int32_t res_index = 0) const;
		godot::PackedVector2Array get_hotspots(godot::StringName const& name, int32_t res_index = 0) const;
		int32_t get_animation_length(godot::StringName const& name) const;
		godot::PackedVector2Array get_resolutions(godot::StringName const& name) const;
		godot::PackedFloat32Array get_display_rates(godot::StringName const& name) const;
		godot::PackedInt32Array get_sequence(godot::StringName const& name) const;

		void generate_resolution(godot::StringName const& name, int32_t base_res_index, godot::Vector2 target_res);
	};

}