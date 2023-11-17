#pragma once

#include <godot_cpp/classes/atlas_texture.hpp>

#include <openvic-simulation/interface/GFX.hpp>

namespace OpenVic {
	class GFXIconTexture : public godot::AtlasTexture {
		GDCLASS(GFXIconTexture, godot::AtlasTexture)

		/* PROPERTY automatically defines getter functions:
		 * - get_gfx_texture_sprite
		 * - get_icon_index
		 * - get_icon_count */
		GFX::TextureSprite const* PROPERTY(gfx_texture_sprite);
		GFX::frame_t PROPERTY(icon_index);
		GFX::frame_t PROPERTY(icon_count);

	protected:
		static void _bind_methods();

	public:
		GFXIconTexture();

		static godot::Ref<GFXIconTexture> make_gfx_icon_texture(
			GFX::TextureSprite const* gfx_texture_sprite, GFX::frame_t icon = GFX::NO_FRAMES
		);

		/* Discard the GFX::TextureSprite, atlas texture and icon index. */
		void clear();

		/* Set the GFX::TextureSprite, load its texture as an atlas and set its displayed icon */
		godot::Error set_gfx_texture_sprite(
			GFX::TextureSprite const* new_gfx_texture_sprite, GFX::frame_t icon = GFX::NO_FRAMES
		);

		/* Search for a GFX::TextureSprite with the specfied name and,
		 * if successful, call set_gfx_texture_sprite to set it and its icon */
		godot::Error set_gfx_texture_sprite_name(
			godot::String const& gfx_texture_sprite_name, GFX::frame_t icon = GFX::NO_FRAMES
		);

		/* Return the name of the GFX::TextureSprite, or an empty String if it's null */
		godot::String get_gfx_texture_sprite_name() const;

		/* Set icon_index to a value between one and icon_count (inclusive), and update the AtlasTexture's region
		 * to display that frame. An index of zero can be used if gfx_texture_sprite has no frames (zero icon_count).
		 * If zero is used but icon_count is non-zero, icon_index defaults to icon_count (the last frame,
		 * not the first frame because it is often empty). */
		godot::Error set_icon_index(GFX::frame_t new_icon_index);
	};
}
