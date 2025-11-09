#pragma once

#include <godot_cpp/classes/atlas_texture.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>

#include "openvic-extension/classes/GFXSpriteTexture.hpp"

namespace OpenVic {
	class AssetManager : public godot::Object {
		GDCLASS(AssetManager, godot::Object)

		static inline AssetManager* _singleton = nullptr;

	public:
		enum LoadFlags {
			LOAD_FLAG_NONE          = 0,
			LOAD_FLAG_CACHE_IMAGE   = 1 << 0,
			LOAD_FLAG_CACHE_TEXTURE = 1 << 1,
			LOAD_FLAG_FLIP_Y        = 1 << 2
		};

	private:
		struct image_asset_t {
			godot::Ref<godot::Image> image;
			godot::Ref<godot::ImageTexture> texture;
		};
		/* deque_ordered_map to avoid the need to reallocate. */
		using image_asset_map_t = deque_ordered_map<godot::StringName, image_asset_t>;
		using font_map_t = deque_ordered_map<godot::StringName, godot::Ref<godot::FontFile>>;

		image_asset_map_t image_assets;
		font_map_t fonts;

		static godot::Ref<godot::Image> _load_image(godot::String const& path, bool flip_y);

	protected:
		static void _bind_methods();

	public:
		static AssetManager* get_singleton();

		AssetManager();
		~AssetManager();

		/* Search for and load an image at the specified path relative to the game defines, first checking the AssetManager's
		 * image cache in case it has already been loaded, and returning nullptr if image loading fails. If the cache image
		 * load flag is set then the loaded image will be stored in the AssetManager's image cache for future access; if the
		 * flip y load flag is set then the image will be flipped vertically before being returned (if the image is already
		 * in the cache then no flipping will occur, regardless of whether it was originally flipped or not). */
		godot::Ref<godot::Image> get_image(godot::StringName const& path, godot::BitField<LoadFlags> load_flags = LOAD_FLAG_CACHE_IMAGE);

		/* Create a texture from an image found at the specified path relative to the game defines, fist checking the
		 * AssetManager's texture cache in case it has already been loaded, and returning nullptr if image loading or texture
		 * creation fails. If the cache image load flag is set then the loaded image will be stored in the AssetManager's
		 * image cache for future access; if the cache texture load flag is set then the created texture will be stored in the
		 * AssetManager's texture cache for future access; if the flip y load flag is set then the image will be flipped
		 * vertically before being used to create the texture (if the image is already in the cache then no flipping will
		 * occur, regardless of whether it was originally flipped or not). */
		godot::Ref<godot::ImageTexture> get_texture(
			godot::StringName const& path, godot::BitField<LoadFlags> load_flags = LOAD_FLAG_CACHE_TEXTURE
		);

		static godot::Ref<godot::StyleBoxTexture> make_stylebox_texture(
			godot::Ref<godot::Texture2D> const& texture, godot::Vector2 const& border = {}
		);

		/* Search for and load a font with the specified name from the game defines' font directory, first checking the
		 * AssetManager's font cache in case it has already been loaded, and returning nullptr if font loading fails. */
		godot::Ref<godot::FontFile> get_font(godot::StringName const& name);

	private:
		godot::Ref<GFXSpriteTexture> PROPERTY(currency_texture_big);    // 32x32
		godot::Ref<GFXSpriteTexture> PROPERTY(currency_texture_medium); // 24x24
		godot::Ref<GFXSpriteTexture> PROPERTY(currency_texture_small);  // 16x16

		godot::Ref<GFXSpriteTexture> PROPERTY(missing_leader_texture);

	public:
		godot::Error preload_textures();

		/* Get the largest currency texture with height less than the specified font height. */
		godot::Ref<GFXSpriteTexture> get_currency_texture(real_t height) const;

		godot::Ref<godot::ImageTexture> get_leader_texture_std(std::string_view name);
		godot::Ref<godot::ImageTexture> get_leader_texture(godot::String const& name);
	};
}

VARIANT_BITFIELD_CAST(OpenVic::AssetManager::LoadFlags);
