#pragma once

#include <godot_cpp/classes/atlas_texture.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>

namespace OpenVic {
	class AssetManager : public godot::Object {
		GDCLASS(AssetManager, godot::Object)

		static inline AssetManager* _singleton = nullptr;

		struct image_asset_t {
			godot::Ref<godot::Image> image;
			godot::Ref<godot::ImageTexture> texture;
		};
		/* deque_ordered_map to avoid the need to reallocate. */
		using image_asset_map_t = deque_ordered_map<godot::StringName, image_asset_t>;
		using font_map_t = deque_ordered_map<godot::StringName, godot::Ref<godot::Font>>;

		image_asset_map_t image_assets;
		font_map_t fonts;

		static godot::Ref<godot::Image> _load_image(godot::StringName const& path);
		image_asset_t* _get_image_asset(godot::StringName const& path, bool flip_y);

	protected:
		static void _bind_methods();

	public:
		static AssetManager* get_singleton();

		AssetManager();
		~AssetManager();

		/* Search for and load an image at the specified path relative to the game defines, first checking the AssetManager's
		 * image cache (if cache is true) in case it has already been loaded, and returning nullptr if image loading fails. */
		godot::Ref<godot::Image> get_image(godot::StringName const& path, bool cache = true, bool flip_y = false);

		/* Create a texture from an image found at the specified path relative to the game defines, fist checking
		 * AssetManager's texture cache in case it has already been loaded, and returning nullptr if image loading
		 * or texture creation fails. */
		godot::Ref<godot::ImageTexture> get_texture(godot::StringName const& path, bool flip_y = false);

		/* Search for and load a font with the specified name from the game defines' font directory, first checking the
		 * AssetManager's font cache in case it has already been loaded, and returning nullptr if font loading fails. */
		godot::Ref<godot::Font> get_font(godot::StringName const& name);
	};
}
