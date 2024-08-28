#pragma once

#include "openvic-extension/classes/GFXSpriteTexture.hpp"
#include "openvic-extension/classes/GUITextureRect.hpp"

namespace OpenVic {
	class GUIIcon : public GUITextureRect {
		GDCLASS(GUIIcon, GUITextureRect)

		godot::Ref<GFXSpriteTexture> gfx_sprite_texture;

	protected:
		static void _bind_methods();

	public:
		godot::Error set_gfx_texture_sprite(
			GFX::TextureSprite const* gfx_texture_sprite, GFX::frame_t icon = GFX::NO_FRAMES
		);

		godot::Ref<GFXSpriteTexture> get_gfx_sprite_texture() const;

		godot::Error set_gfx_texture_sprite_name(
			godot::String const& gfx_texture_sprite_name, GFX::frame_t icon = GFX::NO_FRAMES
		);

		godot::String get_gfx_texture_sprite_name() const;

		godot::Error set_icon_index(GFX::frame_t icon_index) const;

		GFX::frame_t get_icon_index() const;

		godot::Error set_toggled_icon(bool toggled) const;
	};
}
