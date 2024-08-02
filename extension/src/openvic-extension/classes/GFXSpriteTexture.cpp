#include "GFXSpriteTexture.hpp"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

void GFXSpriteTexture::_bind_methods() {
	OV_BIND_METHOD(GFXSpriteTexture::clear);

	OV_BIND_METHOD(
		GFXSpriteTexture::set_gfx_texture_sprite_name, { "gfx_texture_sprite_name", "icon" }, DEFVAL(GFX::NO_FRAMES)
	);
	OV_BIND_METHOD(GFXSpriteTexture::get_gfx_texture_sprite_name);

	OV_BIND_METHOD(GFXSpriteTexture::set_icon_index, { "new_icon_index" });
	OV_BIND_METHOD(GFXSpriteTexture::set_toggled_icon, { "toggle" });
	OV_BIND_METHOD(GFXSpriteTexture::get_icon_index);
	OV_BIND_METHOD(GFXSpriteTexture::get_icon_count);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "icon_index"), "set_icon_index", "get_icon_index");
}

GFXSpriteTexture::GFXSpriteTexture()
  : gfx_texture_sprite { nullptr }, icon_index { GFX::NO_FRAMES }, icon_count { GFX::NO_FRAMES } {}

Ref<GFXSpriteTexture> GFXSpriteTexture::make_gfx_sprite_texture(
	GFX::TextureSprite const* gfx_texture_sprite, GFX::frame_t icon
) {
	Ref<GFXSpriteTexture> texture;
	texture.instantiate();
	ERR_FAIL_NULL_V(texture, nullptr);
	ERR_FAIL_COND_V(texture->set_gfx_texture_sprite(gfx_texture_sprite, icon) != OK, nullptr);
	return texture;
}

void GFXSpriteTexture::clear() {
	gfx_texture_sprite = nullptr;
	_clear_button_states();
	icon_index = GFX::NO_FRAMES;
	icon_count = GFX::NO_FRAMES;
	cornered_tile_border_size = {};
}

Error GFXSpriteTexture::set_gfx_texture_sprite(GFX::TextureSprite const* new_gfx_texture_sprite, GFX::frame_t icon) {
	if (gfx_texture_sprite != new_gfx_texture_sprite) {
		if (new_gfx_texture_sprite == nullptr) {
			clear();
			return OK;
		}
		AssetManager* asset_manager = AssetManager::get_singleton();
		ERR_FAIL_NULL_V(asset_manager, FAILED);

		const StringName texture_file = Utilities::std_to_godot_string(new_gfx_texture_sprite->get_texture_file());

		/* Needed for GFXButtonStateTexture, AssetManager::get_texture will re-use this image from its internal cache. */
		const Ref<Image> image = asset_manager->get_image(texture_file);
		ERR_FAIL_NULL_V_MSG(image, FAILED, vformat("Failed to load image: %s", texture_file));

		const Ref<ImageTexture> texture = asset_manager->get_texture(texture_file);
		ERR_FAIL_NULL_V_MSG(texture, FAILED, vformat("Failed to load texture: %s", texture_file));

		button_image = image;
		gfx_texture_sprite = new_gfx_texture_sprite;
		set_atlas(texture);
		icon_index = GFX::NO_FRAMES;

		GFX::IconTextureSprite const* const icon_texture_sprite = gfx_texture_sprite->cast_to<GFX::IconTextureSprite>();
		if (icon_texture_sprite != nullptr) {
			icon_count = icon_texture_sprite->get_no_of_frames();
		} else {
			icon_count = GFX::NO_FRAMES;
		}

		GFX::CorneredTileTextureSprite const* const cornered_tile_texture_sprite =
			gfx_texture_sprite->cast_to<GFX::CorneredTileTextureSprite>();
		if (cornered_tile_texture_sprite != nullptr) {
			cornered_tile_border_size = Utilities::to_godot_ivec2(cornered_tile_texture_sprite->get_border_size());
		} else {
			cornered_tile_border_size = {};
		}
	}
	return set_icon_index(icon);
}

Error GFXSpriteTexture::set_gfx_texture_sprite_name(String const& gfx_texture_sprite_name, GFX::frame_t icon) {
	if (gfx_texture_sprite_name.is_empty()) {
		return set_gfx_texture_sprite(nullptr);
	}
	GFX::Sprite const* sprite = UITools::get_gfx_sprite(gfx_texture_sprite_name);
	ERR_FAIL_NULL_V(sprite, FAILED);
	GFX::TextureSprite const* new_texture_sprite = sprite->cast_to<GFX::TextureSprite>();
	ERR_FAIL_NULL_V_MSG(
		new_texture_sprite, FAILED, vformat(
			"Invalid type for GFX sprite %s: %s (expected %s)", gfx_texture_sprite_name,
			Utilities::std_to_godot_string(sprite->get_type()),
			Utilities::std_to_godot_string(GFX::TextureSprite::get_type_static())
		)
	);
	return set_gfx_texture_sprite(new_texture_sprite, icon);
}

String GFXSpriteTexture::get_gfx_texture_sprite_name() const {
	return gfx_texture_sprite != nullptr ? Utilities::std_to_godot_string(gfx_texture_sprite->get_name()) : String {};
}

Error GFXSpriteTexture::set_icon_index(int32_t new_icon_index) {
	const Ref<Texture2D> atlas_texture = get_atlas();
	ERR_FAIL_NULL_V(atlas_texture, FAILED);
	const Vector2 size = atlas_texture->get_size();
	if (icon_count <= GFX::NO_FRAMES) {
		if (new_icon_index > GFX::NO_FRAMES) {
			UtilityFunctions::push_warning("Invalid icon index ", new_icon_index, " for texture with no frames!");
		}
		icon_index = GFX::NO_FRAMES;
		set_region({ {}, size });
	} else {
		if (GFX::NO_FRAMES < new_icon_index && new_icon_index <= icon_count) {
			icon_index = new_icon_index;
		} else {
			icon_index = 1;
			if (new_icon_index > icon_count) {
				UtilityFunctions::push_warning(
					"Invalid icon index ", new_icon_index, " out of count ", icon_count, " - defaulting to ", icon_index
				);
			}
		}
		set_region({ (icon_index - 1) * size.x / icon_count, 0, size.x / icon_count, size.y });
	}
	_update_button_states();
	return OK;
}

Error GFXSpriteTexture::set_toggled_icon(bool toggled) {
	return set_icon_index(toggled ? 2 : 1);
}
