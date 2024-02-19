#pragma once

#include <godot_cpp/classes/atlas_texture.hpp>

#include <openvic-simulation/interface/GFX.hpp>

#include "openvic-extension/classes/GFXButtonStateTexture.hpp"

namespace OpenVic {
	class GFXSpriteTexture : public GFXButtonStateHavingTexture {
		GDCLASS(GFXSpriteTexture, GFXButtonStateHavingTexture)

		/* PROPERTY automatically defines getter functions:
		 * - get_gfx_texture_sprite
		 * - get_icon_index
		 * - get_icon_count
		 * - is_cornered_tile_texture */
		GFX::TextureSprite const* PROPERTY(gfx_texture_sprite);
		GFX::frame_t PROPERTY(icon_index);
		GFX::frame_t PROPERTY(icon_count);
		bool PROPERTY_CUSTOM_PREFIX(cornered_tile_texture, is);
		godot::Vector2i cornered_tile_border_size;

	protected:
		static void _bind_methods();

	public:
		GFXSpriteTexture();

		/* Create a GFXSpriteTexture using the specified GFX::TextureSprite and icon index. Returns nullptr if
		 * set_gfx_texture_sprite fails. */
		static godot::Ref<GFXSpriteTexture> make_gfx_sprite_texture(
			GFX::TextureSprite const* gfx_texture_sprite, GFX::frame_t icon = GFX::NO_FRAMES
		);

		/* Discard the GFX::TextureSprite, atlas texture and icon index. */
		void clear();

		/* Set the GFX::TextureSprite, load its texture as an atlas, check if it is an IconTextureSprite,
		 * and if so set its icon count and the current displayed icon. */
		godot::Error set_gfx_texture_sprite(
			GFX::TextureSprite const* new_gfx_texture_sprite, GFX::frame_t icon = GFX::NO_FRAMES
		);

		/* Search for a GFX::TextureSprite with the specfied name and,
		 * if successful, call set_gfx_texture_sprite to set it and its icon. */
		godot::Error set_gfx_texture_sprite_name(
			godot::String const& gfx_texture_sprite_name, GFX::frame_t icon = GFX::NO_FRAMES
		);

		/* Return the name of the GFX::TextureSprite, or an empty String if it's null. */
		godot::String get_gfx_texture_sprite_name() const;

		/* Set icon_index to a value between one and icon_count (inclusive), and update the AtlasTexture's region
		 * to display that frame. An index of zero can be used if gfx_texture_sprite has no frames (zero icon_count).
		 * If zero is used but icon_count is non-zero, icon_index defaults to icon_count (the last frame,
		 * not the first frame because it is often empty). */
		godot::Error set_icon_index(GFX::frame_t new_icon_index);

		/* Equivalent to draw_rect, but draws a 9 patch texture if this is a cornered tile texture. */
		void draw_rect_cornered(godot::RID const& to_canvas_item, godot::Rect2 const& rect) const;
	};
}