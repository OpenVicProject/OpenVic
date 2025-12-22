#include "GUIIcon.hpp"

#include "openvic-extension/core/Bind.hpp"

using namespace godot;
using namespace OpenVic;

void GUIIcon::_bind_methods() {
	OV_BIND_METHOD(GUIIcon::get_gfx_sprite_texture);

	OV_BIND_METHOD(GUIIcon::set_gfx_texture_sprite_name, { "gfx_texture_sprite_name", "icon" }, DEFVAL(GFX::NO_FRAMES));
	OV_BIND_METHOD(GUIIcon::get_gfx_texture_sprite_name);

	OV_BIND_METHOD(GUIIcon::set_icon_index, { "icon_index" });
	OV_BIND_METHOD(GUIIcon::get_icon_index);

	OV_BIND_METHOD(GUIIcon::set_toggled_icon, { "toggled" });
}

Error GUIIcon::set_gfx_texture_sprite(GFX::TextureSprite const* gfx_texture_sprite, GFX::frame_t icon) {
	const bool needs_setting = gfx_sprite_texture.is_null();

	if (needs_setting) {
		gfx_sprite_texture.instantiate();
		ERR_FAIL_NULL_V(gfx_sprite_texture, FAILED);
	}

	const Error err = gfx_sprite_texture->set_gfx_texture_sprite(gfx_texture_sprite, icon);

	if (needs_setting) {
		set_texture(gfx_sprite_texture);
	}

	return err;
}

Ref<GFXSpriteTexture> GUIIcon::get_gfx_sprite_texture() const {
	ERR_FAIL_NULL_V(gfx_sprite_texture, nullptr);

	return gfx_sprite_texture;
}

Error GUIIcon::set_gfx_texture_sprite_name(String const& gfx_texture_sprite_name, GFX::frame_t icon) {
	const bool needs_setting = gfx_sprite_texture.is_null();

	if (needs_setting) {
		gfx_sprite_texture.instantiate();
		ERR_FAIL_NULL_V(gfx_sprite_texture, FAILED);
	}

	const Error err = gfx_sprite_texture->set_gfx_texture_sprite_name(gfx_texture_sprite_name, icon);

	if (needs_setting) {
		set_texture(gfx_sprite_texture);
	}

	return err;
}

String GUIIcon::get_gfx_texture_sprite_name() const {
	ERR_FAIL_NULL_V(gfx_sprite_texture, {});

	return gfx_sprite_texture->get_gfx_texture_sprite_name();
}

Error GUIIcon::set_icon_index(GFX::frame_t icon_index) const {
	ERR_FAIL_NULL_V(gfx_sprite_texture, FAILED);

	return gfx_sprite_texture->set_icon_index(icon_index);
}

GFX::frame_t GUIIcon::get_icon_index() const {
	ERR_FAIL_NULL_V(gfx_sprite_texture, FAILED);

	return gfx_sprite_texture->get_icon_index();
}

Error GUIIcon::set_toggled_icon(bool toggled) const {
	ERR_FAIL_NULL_V(gfx_sprite_texture, FAILED);

	return gfx_sprite_texture->set_toggled_icon(toggled);
}
