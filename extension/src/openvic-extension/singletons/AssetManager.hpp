#pragma once

#include <godot_cpp/classes/atlas_texture.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <openvic-simulation/interface/GFX.hpp>

namespace OpenVic {
	class AssetManager : public godot::Object {
		GDCLASS(AssetManager, godot::Object)

		static inline AssetManager* _singleton = nullptr;

		struct image_asset_t {
			godot::Ref<godot::Image> image;
			godot::Ref<godot::ImageTexture> texture;
		};
		using image_asset_map_t = std::map<godot::StringName, image_asset_t>;
		using font_map_t = std::map<godot::StringName, godot::Ref<godot::Font>>;

		image_asset_map_t image_assets;
		font_map_t fonts;

		static godot::Ref<godot::Image> _load_image(godot::StringName path);
		image_asset_map_t::iterator _get_image_asset(godot::StringName path);

	protected:
		static void _bind_methods();

	public:
		static AssetManager* get_singleton();

		AssetManager();
		~AssetManager();

		/* Search for and load an image at the specified path relative to the game defines, first checking the AssetManager's
		 * image cache (if cache is true) in case it has already been loaded, and returning nullptr if image loading fails. */
		godot::Ref<godot::Image> get_image(godot::StringName path, bool cache = true);

		/* Create a texture from an image found at the specified path relative to the game defines, fist checking
		 * AssetManager's texture cache in case it has already been loaded, and returning nullptr if image loading
		 * or texture creation fails. */
		godot::Ref<godot::ImageTexture> get_texture(godot::StringName path);

		/* Extract the specified frame of the texture, which is treated as a single row of frame_count frames. */
		static godot::Ref<godot::AtlasTexture> make_icon(
			godot::Ref<godot::Texture2D> texture, GFX::frame_t frame, GFX::frame_t frame_count
		);

		/* Load a texture as with get_texture, and extract the specified frame as with make_icon. */
		godot::Ref<godot::AtlasTexture> get_icon(godot::StringName path, GFX::frame_t frame, GFX::frame_t frame_count);

		/* Load a texture as with get_texture if frame_count <= 1 otherwise as with get_icon. */
		godot::Ref<godot::Texture2D> get_texture_or_icon(godot::StringName path, GFX::frame_t frame, GFX::frame_t frame_count);

		/* Search for and load a font with the specified name from the game defines' font directory, first checking the
		 * AssetManager's font cache in case it has already been loaded, and returning nullptr if font loading fails. */
		godot::Ref<godot::Font> get_font(godot::StringName name);
	};
}
