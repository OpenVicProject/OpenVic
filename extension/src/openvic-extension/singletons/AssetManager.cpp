#include "AssetManager.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_to_godot_string;

void AssetManager::_bind_methods() {
	OV_BIND_METHOD(AssetManager::get_image, { "path", "load_flags" }, DEFVAL(LOAD_FLAG_CACHE_IMAGE));
	OV_BIND_METHOD(AssetManager::get_texture, { "path", "load_flags" }, DEFVAL(LOAD_FLAG_CACHE_TEXTURE));
	OV_BIND_METHOD(AssetManager::get_font, { "name" });

	BIND_ENUM_CONSTANT(LOAD_FLAG_NONE);
	BIND_ENUM_CONSTANT(LOAD_FLAG_CACHE_IMAGE);
	BIND_ENUM_CONSTANT(LOAD_FLAG_CACHE_TEXTURE);
	BIND_ENUM_CONSTANT(LOAD_FLAG_FLIP_Y);
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

Ref<Image> AssetManager::_load_image(StringName const& path, bool flip_y) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);

	const String lookedup_path =
		std_to_godot_string(game_singleton->get_dataloader().lookup_image_file(godot_to_std_string(path)).string());
	ERR_FAIL_COND_V_MSG(lookedup_path.is_empty(), nullptr, vformat("Failed to look up image: %s", path));

	const Ref<Image> image = Utilities::load_godot_image(lookedup_path);
	ERR_FAIL_COND_V_MSG(
		image.is_null() || image->is_empty(), nullptr,
		vformat("Failed to load image: %s (looked up: %s)", path, lookedup_path)
	);

	if (flip_y) {
		image->flip_y();
	}

	return image;
}

Ref<Image> AssetManager::get_image(StringName const& path, LoadFlags load_flags) {
	/* Check for an existing image entry indicating a previous load attempt, whether successful or not. */
	const image_asset_map_t::iterator it = image_assets.find(path);
	if (it != image_assets.end()) {
		std::optional<Ref<Image>> const& cached_image = it->second.image;

		if (cached_image.has_value()) {
			ERR_FAIL_NULL_V_MSG(*cached_image, nullptr, vformat("Failed to load image previously: %s", path));

			return *cached_image;
		}
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
		image_assets[path] = { nullptr, nullptr };

		ERR_FAIL_V_MSG(nullptr, vformat("Failed to load image: %s", path));
	}
}

Ref<ImageTexture> AssetManager::get_texture(StringName const& path, LoadFlags load_flags) {
	/* Check for an existing texture entry indicating a previous creation attempt, whether successful or not. */
	const image_asset_map_t::const_iterator it = image_assets.find(path);
	if (it != image_assets.end()) {
		std::optional<Ref<ImageTexture>> const& cached_texture = it->second.texture;

		if (cached_texture.has_value()) {
			ERR_FAIL_NULL_V_MSG(*cached_texture, nullptr, vformat("Failed to create texture previously: %s", path));

			return *cached_texture;
		}
	}

	/* No creation attempt has yet been made, so we try now starting by finding the corresponding image. */
	const Ref<Image> image = get_image(path, load_flags);
	ERR_FAIL_NULL_V_MSG(image, nullptr, vformat("Failed to load image for texture: %s", path));

	const Ref<ImageTexture> texture = ImageTexture::create_from_image(image);

	if (texture.is_valid()) {
		if (load_flags & LOAD_FLAG_CACHE_TEXTURE) {
			image_assets[path].texture = texture;
		}

		return texture;
	} else {
		/* Mark texture as a failure, regardless of cache flags, in case of future creation attempts. */
		image_assets[path].texture = nullptr;

		ERR_FAIL_V_MSG(nullptr, vformat("Failed to create texture: %s", path));
	}
}

Ref<Font> AssetManager::get_font(StringName const& name) {
	const font_map_t::const_iterator it = fonts.find(name);
	if (it != fonts.end()) {
		ERR_FAIL_NULL_V_MSG(it->second, nullptr, vformat("Failed to load font previously: %s", name));

		return it->second;
	}

	static const String font_dir = "gfx/fonts/";
	static const String font_ext = ".fnt";
	static const String image_ext = ".tga";

	const StringName image_path = font_dir + name + image_ext;
	const Ref<Image> image = get_image(image_path, LOAD_FLAG_NONE);
	if (image.is_null()) {
		fonts.emplace(name, nullptr);

		ERR_FAIL_V_MSG(nullptr, vformat("Failed to load font image %s for the font named %s", image_path, name));
	}

	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);

	const String font_path = font_dir + name + font_ext;
	const String lookedup_font_path =
		std_to_godot_string(game_singleton->get_dataloader().lookup_file(godot_to_std_string(font_path)).string());
	if (lookedup_font_path.is_empty()) {
		fonts.emplace(name, nullptr);

		ERR_FAIL_V_MSG(nullptr, vformat("Failed to look up font: %s", font_path));
	}

	const Ref<Font> font = Utilities::load_godot_font(lookedup_font_path, image);
	if (font.is_null()) {
		fonts.emplace(name, nullptr);

		ERR_FAIL_V_MSG(
			nullptr,
			vformat("Failed to load font file %s (looked up: %s) for the font named %s", font_path, lookedup_font_path, name)
		);
	}

	fonts.emplace(name, font);
	return font;
}
