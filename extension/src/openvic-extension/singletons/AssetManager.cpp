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
	OV_BIND_METHOD(AssetManager::get_image, { "path" });
	OV_BIND_METHOD(AssetManager::get_texture, { "path" });
	OV_BIND_METHOD(AssetManager::get_font, { "name" });
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

Ref<Image> AssetManager::_load_image(StringName const& path) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	const String lookedup_path =
		std_to_godot_string(game_singleton->get_dataloader().lookup_image_file(godot_to_std_string(path)).string());
	ERR_FAIL_COND_V_MSG(lookedup_path.is_empty(), nullptr, vformat("Failed to look up image: %s", path));
	const Ref<Image> image = Utilities::load_godot_image(lookedup_path);
	ERR_FAIL_COND_V_MSG(
		image.is_null() || image->is_empty(), nullptr, vformat("Failed to load image: %s (looked up: %s)", path, lookedup_path)
	);
	return image;
}

AssetManager::image_asset_map_t::iterator AssetManager::_get_image_asset(StringName const& path) {
	const image_asset_map_t::iterator it = image_assets.find(path);
	if (it != image_assets.end()) {
		return it;
	}
	const Ref<Image> image = _load_image(path);
	ERR_FAIL_NULL_V(image, image_assets.end());
	return image_assets.emplace(std::move(path), AssetManager::image_asset_t { image, nullptr }).first;
}

Ref<Image> AssetManager::get_image(StringName const& path, bool cache) {
	if (cache) {
		const image_asset_map_t::const_iterator it = _get_image_asset(path);
		ERR_FAIL_COND_V(it == image_assets.end(), nullptr);
		return it->second.image;
	} else {
		return _load_image(path);
	}
}

Ref<ImageTexture> AssetManager::get_texture(StringName const& path) {
	const image_asset_map_t::iterator it = _get_image_asset(path);
	ERR_FAIL_COND_V(it == image_assets.end(), nullptr);
	if (it->second.texture.is_null()) {
		it->second.texture = ImageTexture::create_from_image(it->second.image);
		ERR_FAIL_NULL_V_MSG(it->second.texture, nullptr, vformat("Failed to turn image into texture: %s", path));
	}
	return it->second.texture;
}

Ref<Font> AssetManager::get_font(StringName const& name) {
	const font_map_t::const_iterator it = fonts.find(name);
	if (it != fonts.end()) {
		return it->second;
	}

	static const String font_dir = "gfx/fonts/";
	static const String font_ext = ".fnt";
	static const String image_ext = ".tga";

	const StringName image_path = font_dir + name + image_ext;
	const Ref<Image> image = get_image(image_path);
	ERR_FAIL_NULL_V_MSG(image, nullptr, vformat("Failed to load font image %s for the font named %s", image_path, name));
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	const String font_path = font_dir + name + font_ext;
	const String lookedup_font_path =
		std_to_godot_string(game_singleton->get_dataloader().lookup_file(godot_to_std_string(font_path)).string());
	const Ref<Font> font = Utilities::load_godot_font(lookedup_font_path, image);
	ERR_FAIL_NULL_V_MSG(
		font, nullptr,
		vformat("Failed to load font file %s (looked up: %s) for the font named %s", font_path, lookedup_font_path, name)
	);
	fonts.emplace(std::move(name), font);
	return font;
}
