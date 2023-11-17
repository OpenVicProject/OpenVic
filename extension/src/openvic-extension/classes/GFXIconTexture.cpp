#include "GFXIconTexture.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::std_view_to_godot_string;
using OpenVic::Utilities::std_view_to_godot_string_name;

void GFXIconTexture::_bind_methods() {
	OV_BIND_METHOD(GFXIconTexture::clear);

	OV_BIND_METHOD(GFXIconTexture::set_gfx_texture_sprite_name, { "gfx_texture_sprite_name" }, DEFVAL(GFX::NO_FRAMES));
	OV_BIND_METHOD(GFXIconTexture::get_gfx_texture_sprite_name);

	OV_BIND_METHOD(GFXIconTexture::set_icon_index, { "new_icon_index" });
	OV_BIND_METHOD(GFXIconTexture::get_icon_index);
	OV_BIND_METHOD(GFXIconTexture::get_icon_count);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "icon_index"), "set_icon_index", "get_icon_index");
}

GFXIconTexture::GFXIconTexture()
	: gfx_texture_sprite { nullptr }, icon_index { GFX::NO_FRAMES }, icon_count { GFX::NO_FRAMES } {}

Ref<GFXIconTexture> GFXIconTexture::make_gfx_icon_texture(GFX::TextureSprite const* gfx_texture_sprite, GFX::frame_t icon) {
	Ref<GFXIconTexture> icon_texture;
	icon_texture.instantiate();
	ERR_FAIL_NULL_V(icon_texture, icon_texture);
	icon_texture->set_gfx_texture_sprite(gfx_texture_sprite, icon);
	return icon_texture;
}

void GFXIconTexture::clear() {
	gfx_texture_sprite = nullptr;
	set_atlas(nullptr);
	set_region({});
	icon_index = GFX::NO_FRAMES;
	icon_count = GFX::NO_FRAMES;
}

Error GFXIconTexture::set_gfx_texture_sprite(GFX::TextureSprite const* new_gfx_texture_sprite, GFX::frame_t icon) {
	if (gfx_texture_sprite != new_gfx_texture_sprite) {
		if (new_gfx_texture_sprite == nullptr) {
			clear();
			return OK;
		}
		AssetManager* asset_manager = AssetManager::get_singleton();
		ERR_FAIL_NULL_V(asset_manager, FAILED);
		const StringName texture_file = std_view_to_godot_string_name(new_gfx_texture_sprite->get_texture_file());
		const Ref<ImageTexture> texture = asset_manager->get_texture(texture_file);
		ERR_FAIL_NULL_V_MSG(texture, FAILED, "Failed to load texture: " + texture_file);
		gfx_texture_sprite = new_gfx_texture_sprite;
		set_atlas(texture);
		icon_index = GFX::NO_FRAMES;
		icon_count = gfx_texture_sprite->get_no_of_frames();
	}
	return set_icon_index(icon);
}

Error GFXIconTexture::set_gfx_texture_sprite_name(String const& gfx_texture_sprite_name, GFX::frame_t icon) {
	if (gfx_texture_sprite_name.is_empty()) {
		return set_gfx_texture_sprite(nullptr);
	}
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	GFX::Sprite const* sprite = game_singleton->get_gfx_sprite(gfx_texture_sprite_name);
	ERR_FAIL_NULL_V_MSG(sprite, FAILED, "GFX sprite not found: " + gfx_texture_sprite_name);
	GFX::TextureSprite const* new_texture_sprite = sprite->cast_to<GFX::TextureSprite>();
	ERR_FAIL_NULL_V_MSG(
		new_texture_sprite, FAILED, "Invalid type for GFX sprite " + gfx_texture_sprite_name + ": " +
		std_view_to_godot_string(sprite->get_type()) + " (expected " +
		std_view_to_godot_string(GFX::TextureSprite::get_type_static()) + ")"
	);
	return set_gfx_texture_sprite(new_texture_sprite, icon);
}

String GFXIconTexture::get_gfx_texture_sprite_name() const {
	return gfx_texture_sprite != nullptr ? std_view_to_godot_string(gfx_texture_sprite->get_name()) : String {};
}

Error GFXIconTexture::set_icon_index(int32_t new_icon_index) {
	const Ref<Texture2D> atlas_texture = get_atlas();
	ERR_FAIL_NULL_V(atlas_texture, FAILED);
	const Vector2 size = atlas_texture->get_size();
	if (icon_count <= GFX::NO_FRAMES) {
		if (new_icon_index > GFX::NO_FRAMES) {
			UtilityFunctions::push_warning("Invalid icon index ", new_icon_index, " for texture with no frames!");
		}
		icon_index = GFX::NO_FRAMES;
		set_region({ {}, size });
		return OK;
	}
	if (GFX::NO_FRAMES < new_icon_index && new_icon_index <= icon_count) {
		icon_index = new_icon_index;
	} else {
		icon_index = icon_count;
		if (new_icon_index > icon_count) {
			UtilityFunctions::push_warning(
				"Invalid icon index ", new_icon_index, " out of count ", icon_count, " - defaulting to ", icon_index
			);
		}
	}
	set_region({ (icon_index - 1) * size.x / icon_count, 0, size.x / icon_count, size.y });
	return OK;
}
