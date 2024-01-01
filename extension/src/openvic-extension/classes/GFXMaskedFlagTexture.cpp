#include "GFXMaskedFlagTexture.hpp"

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_view_to_godot_string;
using OpenVic::Utilities::std_view_to_godot_string_name;

StringName const& GFXMaskedFlagTexture::_signal_image_updated() {
	static const StringName signal_image_updated = "image_updated";
	return signal_image_updated;
}

Error GFXMaskedFlagTexture::_generate_combined_image() {
	ERR_FAIL_NULL_V(overlay_image, FAILED);
	/* Whether we've already set the ImageTexture to an image of the right dimensions and format,
	 * and so can update it without creating and setting a new image, or not. */
	const bool can_update = combined_image.is_valid() && combined_image->get_size() == overlay_image->get_size()
		&& combined_image->get_format() == overlay_image->get_format();
	if (!can_update) {
		combined_image = Image::create(
			overlay_image->get_width(), overlay_image->get_height(), false, overlay_image->get_format()
		);
		ERR_FAIL_NULL_V(combined_image, FAILED);
	}

	if (mask_image.is_valid() && flag_image.is_valid()) {
		const Vector2i centre_translation = (mask_image->get_size() - combined_image->get_size()) / 2;
		for (Vector2i combined_image_point { 0, 0 }; combined_image_point.y < combined_image->get_height(); ++combined_image_point.y) {
			for (combined_image_point.x = 0; combined_image_point.x < combined_image->get_width(); ++combined_image_point.x) {
				const Color overlay_image_colour = overlay_image->get_pixelv(combined_image_point);
				// Translate to mask_image coordinates, keeping the centres of each image aligned.
				const Vector2i mask_image_point = combined_image_point + centre_translation;
				if (
					0 <= mask_image_point.x && mask_image_point.x < mask_image->get_width() &&
					0 <= mask_image_point.y && mask_image_point.y < mask_image->get_height()
				) {
					const Color mask_image_colour = mask_image->get_pixelv(mask_image_point);
					// Rescale from mask_image to flag_image coordinates.
					const Vector2i flag_image_point = mask_image_point * flag_image->get_size() / mask_image->get_size();
					Color flag_image_colour = flag_image->get_pixelv(flag_image_point);
					flag_image_colour.a = mask_image_colour.a;
					combined_image->set_pixelv(combined_image_point, flag_image_colour.blend(overlay_image_colour));
				} else {
					combined_image->set_pixelv(combined_image_point, overlay_image_colour);
				}
			}
		}
	} else {
		combined_image->blit_rect(overlay_image, overlay_image->get_used_rect(), {});
	}

	if (can_update) {
		update(combined_image);
	} else {
		set_image(combined_image);
	}
	emit_signal(_signal_image_updated(), combined_image);
	return OK;
}

void GFXMaskedFlagTexture::_bind_methods() {
	OV_BIND_METHOD(GFXMaskedFlagTexture::clear);

	OV_BIND_METHOD(GFXMaskedFlagTexture::set_gfx_masked_flag_name, { "gfx_masked_flag_name" });
	OV_BIND_METHOD(GFXMaskedFlagTexture::get_gfx_masked_flag_name);

	OV_BIND_METHOD(GFXMaskedFlagTexture::set_flag_country_name_and_type, { "new_flag_country_name", "new_flag_type" });
	OV_BIND_METHOD(GFXMaskedFlagTexture::set_flag_country_name, { "new_flag_country_name" });
	OV_BIND_METHOD(GFXMaskedFlagTexture::get_flag_country_name);
	OV_BIND_METHOD(GFXMaskedFlagTexture::get_flag_type);

	ADD_SIGNAL(
		MethodInfo(_signal_image_updated(), PropertyInfo(Variant::OBJECT, "source_image", PROPERTY_HINT_RESOURCE_TYPE, "Image"))
	);
}

GFXMaskedFlagTexture::GFXMaskedFlagTexture() : gfx_masked_flag { nullptr }, flag_country { nullptr } {}

Ref<GFXMaskedFlagTexture> GFXMaskedFlagTexture::make_gfx_masked_flag_texture(
	GFX::MaskedFlag const* gfx_masked_flag, std::vector<Ref<GFXButtonStateTexture>> const& button_state_textures
) {
	Ref<GFXMaskedFlagTexture> masked_flag_texture;
	masked_flag_texture.instantiate();
	ERR_FAIL_NULL_V(masked_flag_texture, nullptr);

	for (Ref<GFXButtonStateTexture> const& button_state_texture : button_state_textures) {
		masked_flag_texture->connect(
			_signal_image_updated(),
			Callable { *button_state_texture, GFXButtonStateTexture::get_generate_state_image_func_name() },
			CONNECT_PERSIST
		);
	}

	ERR_FAIL_COND_V(masked_flag_texture->set_gfx_masked_flag(gfx_masked_flag) != OK, nullptr);
	return masked_flag_texture;
}

void GFXMaskedFlagTexture::clear() {
	gfx_masked_flag = nullptr;
	flag_country = nullptr;
	flag_type = String {};

	overlay_image.unref();
	mask_image.unref();
	flag_image.unref();
}

Error GFXMaskedFlagTexture::set_gfx_masked_flag(GFX::MaskedFlag const* new_gfx_masked_flag) {
	if (gfx_masked_flag == new_gfx_masked_flag) {
		return OK;
	}
	if (new_gfx_masked_flag == nullptr) {
		clear();
		return OK;
	}
	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);

	const StringName overlay_file = std_view_to_godot_string_name(new_gfx_masked_flag->get_overlay_file());
	const Ref<Image> new_overlay_image = asset_manager->get_image(overlay_file);
	ERR_FAIL_NULL_V_MSG(new_overlay_image, FAILED, vformat("Failed to load flag overlay image: %s", overlay_file));

	const StringName mask_file = std_view_to_godot_string_name(new_gfx_masked_flag->get_mask_file());
	const Ref<Image> new_mask_image = asset_manager->get_image(mask_file);
	ERR_FAIL_NULL_V_MSG(new_mask_image, FAILED, vformat("Failed to load flag mask image: %s", mask_file));

	gfx_masked_flag = new_gfx_masked_flag;
	overlay_image = new_overlay_image;
	mask_image = new_mask_image;

	return _generate_combined_image();
}

Error GFXMaskedFlagTexture::set_gfx_masked_flag_name(String const& gfx_masked_flag_name) {
	if (gfx_masked_flag_name.is_empty()) {
		return set_gfx_masked_flag(nullptr);
	}
	GFX::Sprite const* sprite = UITools::get_gfx_sprite(gfx_masked_flag_name);
	ERR_FAIL_NULL_V(sprite, FAILED);
	GFX::MaskedFlag const* new_masked_flag = sprite->cast_to<GFX::MaskedFlag>();
	ERR_FAIL_NULL_V_MSG(
		new_masked_flag, FAILED, vformat(
			"Invalid type for GFX sprite %s: %s (expected %s)", gfx_masked_flag_name,
			std_view_to_godot_string(sprite->get_type()), std_view_to_godot_string(GFX::MaskedFlag::get_type_static())
		)
	);
	return set_gfx_masked_flag(new_masked_flag);
}

String GFXMaskedFlagTexture::get_gfx_masked_flag_name() const {
	return gfx_masked_flag != nullptr ? std_view_to_godot_string(gfx_masked_flag->get_name()) : String {};
}

Error GFXMaskedFlagTexture::set_flag_country_and_type(Country const* new_flag_country, StringName const& new_flag_type) {
	if (flag_country == new_flag_country && flag_type == new_flag_type) {
		return OK;
	}
	if (new_flag_country != nullptr) {
		GameSingleton* game_singleton = GameSingleton::get_singleton();
		ERR_FAIL_NULL_V(game_singleton, FAILED);

		const Ref<Image> new_flag_image = game_singleton->get_flag_image(new_flag_country, new_flag_type);
		ERR_FAIL_NULL_V(new_flag_image, FAILED);

		flag_country = new_flag_country;
		flag_type = new_flag_type;
		flag_image = new_flag_image;
	} else {
		// TODO - use REB flag as default/error flag
		flag_country = nullptr;
		flag_type = String {};
		flag_image.unref();
	}
	return _generate_combined_image();
}

Error GFXMaskedFlagTexture::set_flag_country_name_and_type(String const& new_flag_country_name, StringName const& new_flag_type) {
	if (new_flag_country_name.is_empty()) {
		return set_flag_country_and_type(nullptr, {});
	}
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	Country const* new_flag_country = game_singleton->get_game_manager().get_country_manager().get_country_by_identifier(
		godot_to_std_string(new_flag_country_name)
	);
	ERR_FAIL_NULL_V_MSG(new_flag_country, FAILED, vformat("Country not found: %s", new_flag_country_name));
	return set_flag_country_and_type(new_flag_country, new_flag_type);
}

Error GFXMaskedFlagTexture::set_flag_country(Country const* new_flag_country) {
	// TODO - get country's current flag type from the game state
	return set_flag_country_and_type( new_flag_country, {});
}

Error GFXMaskedFlagTexture::set_flag_country_name(String const& new_flag_country_name) {
	if (new_flag_country_name.is_empty()) {
		return set_flag_country(nullptr);
	}
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	Country const* new_flag_country = game_singleton->get_game_manager().get_country_manager().get_country_by_identifier(
		godot_to_std_string(new_flag_country_name)
	);
	ERR_FAIL_NULL_V_MSG(new_flag_country, FAILED, vformat("Country not found: %s", new_flag_country_name));
	return set_flag_country(new_flag_country);
}

String GFXMaskedFlagTexture::get_flag_country_name() const {
	return flag_country != nullptr ? std_view_to_godot_string(flag_country->get_identifier()) : String {};
}
