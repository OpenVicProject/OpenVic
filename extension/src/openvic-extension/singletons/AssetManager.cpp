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

Ref<Image> AssetManager::_load_image(StringName path) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	const String lookedup_path =
		std_to_godot_string(game_singleton->get_dataloader().lookup_image_file(godot_to_std_string(path)).string());
	if (lookedup_path.is_empty()) {
		UtilityFunctions::push_error("Failed to look up image: ", path);
		return nullptr;
	}
	const Ref<Image> image = Utilities::load_godot_image(lookedup_path);
	if (image.is_null() || image->is_empty()) {
		UtilityFunctions::push_error("Failed to load image: ", lookedup_path, " (looked up from ", path, ")");
		return nullptr;
	} else {
		return image;
	}
}

AssetManager::image_asset_map_t::iterator AssetManager::_get_image_asset(StringName path) {
	const image_asset_map_t::iterator it = image_assets.find(path);
	if (it != image_assets.end()) {
		return it;
	}
	const Ref<Image> image = _load_image(path);
	if (image.is_valid()) {
		return image_assets.emplace(std::move(path), AssetManager::image_asset_t { image, nullptr }).first;
	} else {
		return image_assets.end();
	}
}

Ref<Image> AssetManager::get_image(StringName path, bool cache) {
	if (cache) {
		const image_asset_map_t::const_iterator it = _get_image_asset(path);
		if (it != image_assets.end()) {
			return it->second.image;
		} else {
			return nullptr;
		}
	} else {
		return _load_image(path);
	}
}

Ref<ImageTexture> AssetManager::get_texture(StringName path) {
	const image_asset_map_t::iterator it = _get_image_asset(path);
	if (it != image_assets.end()) {
		if (it->second.texture.is_null()) {
			it->second.texture = ImageTexture::create_from_image(it->second.image);
			if (it->second.texture.is_null()) {
				UtilityFunctions::push_error("Failed to turn image into texture: ", path);
			}
		}
		return it->second.texture;
	} else {
		return nullptr;
	}
}

Ref<Font> AssetManager::get_font(StringName name) {
	const font_map_t::const_iterator it = fonts.find(name);
	if (it != fonts.end()) {
		return it->second;
	}

	static const String font_dir = "gfx/fonts/";
	static const String font_ext = ".fnt";
	static const String image_ext = ".tga";

	const String image_path = font_dir + name + image_ext;
	const Ref<Image> image = get_image(image_path);
	if (image.is_null()) {
		UtilityFunctions::push_error("Failed to load font image: ", image_path, " for the font named ", name);
		return nullptr;
	}
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	const String lookedup_font_path =
		std_to_godot_string(game_singleton->get_dataloader().lookup_file(godot_to_std_string(font_dir + name + font_ext)).string());
	const Ref<Font> font = Utilities::load_godot_font(lookedup_font_path, image);
	if (font.is_null()) {
		UtilityFunctions::push_error("Failed to load font file ", lookedup_font_path, " for the font named ", name);
		return nullptr;
	}
	fonts.emplace(std::move(name), font);
	return font;
}
