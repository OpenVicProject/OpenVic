#include "AssetManager.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

void AssetManager::_bind_methods() {
	OV_BIND_METHOD(AssetManager::get_image, { "path", "load_flags" }, DEFVAL(LOAD_FLAG_CACHE_IMAGE));
	OV_BIND_METHOD(AssetManager::get_texture, { "path", "load_flags" }, DEFVAL(LOAD_FLAG_CACHE_TEXTURE));
	OV_BIND_METHOD(AssetManager::get_font, { "name" });
	OV_BIND_METHOD(AssetManager::get_currency_texture, { "height" });
	OV_BIND_METHOD(AssetManager::get_leader_texture, { "name" });

	BIND_BITFIELD_FLAG(LOAD_FLAG_NONE);
	BIND_BITFIELD_FLAG(LOAD_FLAG_CACHE_IMAGE);
	BIND_BITFIELD_FLAG(LOAD_FLAG_CACHE_TEXTURE);
	BIND_BITFIELD_FLAG(LOAD_FLAG_FLIP_Y);
}

AssetManager* AssetManager::get_singleton() {
	return _singleton;
}

AssetManager::AssetManager() {
	ERR_FAIL_COND(_singleton != nullptr);
	_singleton = this;
}

AssetManager::~AssetManager() {
	ERR_FAIL_COND(_singleton != this);
	_singleton = nullptr;
}

Ref<Image> AssetManager::_load_image(String const& path, bool flip_y) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);

	const String lookedup_path = Utilities::std_to_godot_string(
		game_singleton->get_dataloader().lookup_image_file(Utilities::godot_to_std_string(path)).string()
	);
	ERR_FAIL_COND_V_MSG(lookedup_path.is_empty(), nullptr, Utilities::format("Failed to look up image: %s", path));

	const Ref<Image> image = Utilities::load_godot_image(lookedup_path);
	ERR_FAIL_COND_V_MSG(
		image.is_null() || image->is_empty(), nullptr,
		Utilities::format("Failed to load image: %s (looked up: %s)", path, lookedup_path)
	);
	if (image->detect_alpha() != Image::ALPHA_NONE) {
		image->fix_alpha_edges();
	}

	if (flip_y) {
		image->flip_y();
	}

	return image;
}

Ref<Image> AssetManager::get_image(StringName const& path, BitField<LoadFlags> load_flags) {
	/* Check for an existing image entry indicating a previous load attempt, whether successful or not. */
	const image_asset_map_t::const_iterator it = image_assets.find(path);
	if (it != image_assets.end() && it.value().image.is_valid()) {
		return it.value().image;
	}

	/* No load attempt has been made yet, so we try now. */
	const Ref<Image> image = _load_image(path, load_flags & LOAD_FLAG_FLIP_Y);

	if (image.is_valid()) {
		if (load_flags & LOAD_FLAG_CACHE_IMAGE) {
			image_assets[path].image = image;
		}

		return image;
	} else {
		/* Mark both image and texture as failures, regardless of cache flags, in case of future load/creation attempts. */
		image_assets[path] = {};

		ERR_FAIL_V_MSG(nullptr, Utilities::format("Failed to load image: %s", path));
	}
}

Ref<ImageTexture> AssetManager::get_texture(StringName const& path, BitField<LoadFlags> load_flags) {
	/* Check for an existing texture entry indicating a previous creation attempt, whether successful or not. */
	const image_asset_map_t::const_iterator it = image_assets.find(path);
	if (it != image_assets.end() && it.value().texture.is_valid()) {
		return it.value().texture;
	}

	/* No creation attempt has yet been made, so we try now starting by finding the corresponding image. */
	const Ref<Image> image = get_image(path, load_flags);
	ERR_FAIL_NULL_V_MSG(image, nullptr, Utilities::format("Failed to load image for texture: %s", path));

	const Ref<ImageTexture> texture = ImageTexture::create_from_image(image);

	if (texture.is_valid()) {
		if (load_flags & LOAD_FLAG_CACHE_TEXTURE) {
			image_assets[path].texture = texture;
		}

		return texture;
	} else {
		/* Mark texture as a failure, regardless of cache flags, in case of future creation attempts. */
		image_assets[path].texture = Ref<ImageTexture> {};

		ERR_FAIL_V_MSG(nullptr, Utilities::format("Failed to create texture: %s", path));
	}
}

Ref<StyleBoxTexture> AssetManager::make_stylebox_texture(Ref<Texture2D> const& texture, Vector2 const& border) {
	ERR_FAIL_NULL_V(texture, nullptr);

	Ref<StyleBoxTexture> stylebox;
	stylebox.instantiate();
	ERR_FAIL_NULL_V(stylebox, nullptr);

	stylebox->set_texture(texture);

	texture->connect(OV_SNAME(changed), Callable { *stylebox, OV_SNAME(emit_changed) }, Object::CONNECT_PERSIST);

	if (border != Vector2 {}) {
		stylebox->set_texture_margin(SIDE_LEFT, border.x);
		stylebox->set_texture_margin(SIDE_RIGHT, border.x);
		stylebox->set_texture_margin(SIDE_TOP, border.y);
		stylebox->set_texture_margin(SIDE_BOTTOM, border.y);
	}

	return stylebox;
}

Ref<FontFile> AssetManager::get_font(StringName const& name) {
	const font_map_t::const_iterator it = fonts.find(name);
	if (it != fonts.end()) {
		ERR_FAIL_NULL_V_MSG(it->second, nullptr, Utilities::format("Failed to load font previously: %s", name));

		return it->second;
	}

	static const String font_dir = "gfx/fonts/";
	static const String font_ext = ".fnt";
	static const String image_ext = ".tga";

	const StringName image_path = font_dir + name + image_ext;
	const Ref<Image> image = get_image(image_path, LOAD_FLAG_NONE);
	if (image.is_null()) {
		fonts.emplace(name, nullptr);

		ERR_FAIL_V_MSG(nullptr, Utilities::format("Failed to load font image %s for the font named %s", image_path, name));
	}

	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);

	const String font_path = font_dir + name + font_ext;
	const String lookedup_font_path = Utilities::std_to_godot_string(
		game_singleton->get_dataloader().lookup_file(Utilities::godot_to_std_string(font_path)).string()
	);
	if (lookedup_font_path.is_empty()) {
		fonts.emplace(name, nullptr);

		ERR_FAIL_V_MSG(nullptr, Utilities::format("Failed to look up font: %s", font_path));
	}

	const Ref<FontFile> font = Utilities::load_godot_font(lookedup_font_path, image);
	if (font.is_null()) {
		fonts.emplace(name, nullptr);

		ERR_FAIL_V_MSG(
			nullptr,
			Utilities::format("Failed to load font file %s (looked up: %s) for the font named %s", font_path, lookedup_font_path, name)
		);
	}

	fonts.emplace(name, font);
	return font;
}

Error AssetManager::preload_textures() {
	static const String currency_sprite_big = "GFX_tooltip_money_big";
	static const String currency_sprite_medium = "GFX_tooltip_money_small";
	static const String currency_sprite_small = "GFX_tooltip_money";

	static const String missing_leader_sprite = "GFX_leader_generic0";

	constexpr auto load = [](String const& sprite_name, Ref<GFXSpriteTexture>& texture) -> bool {
		GFX::Sprite const* sprite = UITools::get_gfx_sprite(sprite_name);
		ERR_FAIL_NULL_V(sprite, false);

		GFX::IconTextureSprite const* icon_sprite = sprite->cast_to<GFX::IconTextureSprite>();
		ERR_FAIL_NULL_V(icon_sprite, false);

		texture = GFXSpriteTexture::make_gfx_sprite_texture(icon_sprite);
		ERR_FAIL_NULL_V(texture, false);

		return true;
	};

	bool ret = true;

	ret &= load(currency_sprite_big, currency_texture_big);
	ret &= load(currency_sprite_medium, currency_texture_medium);
	ret &= load(currency_sprite_small, currency_texture_small);

	ret &= load(missing_leader_sprite, missing_leader_texture);

	return ERR(ret);
}

Ref<GFXSpriteTexture> AssetManager::get_currency_texture(real_t height) const {
	ERR_FAIL_NULL_V(currency_texture_big, Ref<GFXSpriteTexture>());
	ERR_FAIL_NULL_V(currency_texture_medium, Ref<GFXSpriteTexture>());
	if (height > currency_texture_big->get_height()) {
		return currency_texture_big;
	} else if (height > currency_texture_medium->get_height()) {
		return currency_texture_medium;
	} else {
		return currency_texture_small;
	}
}

Ref<ImageTexture> AssetManager::get_leader_texture_std(std::string_view name) {
	return get_texture(Utilities::std_to_godot_string(CultureManager::make_leader_picture_path(name)));
}

Ref<ImageTexture> AssetManager::get_leader_texture(String const& name) {
	return get_leader_texture_std(Utilities::godot_to_std_string(name));
}
