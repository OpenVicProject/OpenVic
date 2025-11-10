#include "GFXMaskedFlagTexture.hpp"

#include "openvic-extension/core/Convert.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

Error GFXMaskedFlagTexture::_generate_combined_image() {
	ERR_FAIL_NULL_V(overlay_image, FAILED);
	/* Whether we've already set the ImageTexture to an image of the right dimensions and format,
	 * and so can update it without creating and setting a new image, or not. */
	bool can_update = button_image.is_valid() && button_image->get_size() == overlay_image->get_size()
		&& button_image->get_format() == overlay_image->get_format();
	if (!can_update) {
		button_image = Image::create(
			overlay_image->get_width(), overlay_image->get_height(), false, overlay_image->get_format()
		);
		ERR_FAIL_NULL_V(button_image, FAILED);
	}

	if (combined_texture.is_null()) {
		can_update = false;
		combined_texture.instantiate();
		ERR_FAIL_NULL_V(combined_texture, FAILED);
		set_atlas(combined_texture);
		set_region({ {}, button_image->get_size() });
	}

	if (mask_image.is_valid() && flag_image.is_valid() && flag_image_rect.has_area()) {
		const Vector2i centre_translation = (mask_image->get_size() - button_image->get_size()) / 2;

		for (
			Vector2i combined_image_point { 0, 0 };
			combined_image_point.y < button_image->get_height();
			++combined_image_point.y
		) {

			for (combined_image_point.x = 0; combined_image_point.x < button_image->get_width(); ++combined_image_point.x) {

				const Color overlay_image_colour = overlay_image->get_pixelv(combined_image_point);

				// Translate to mask_image coordinates, keeping the centres of each image aligned.
				const Vector2i mask_image_point = combined_image_point + centre_translation;

				if (
					0 <= mask_image_point.x && mask_image_point.x < mask_image->get_width() &&
					0 <= mask_image_point.y && mask_image_point.y < mask_image->get_height()
				) {
					const Color mask_image_colour = mask_image->get_pixelv(mask_image_point);

					// Rescale from mask_image to flag_image coordinates.
					const Vector2i flag_image_point =
						flag_image_rect.position + mask_image_point * flag_image_rect.size / mask_image->get_size();

					Color flag_image_colour = flag_image->get_pixelv(flag_image_point);
					flag_image_colour.a = mask_image_colour.a;

					button_image->set_pixelv(combined_image_point, flag_image_colour.blend(overlay_image_colour));
				} else {
					button_image->set_pixelv(combined_image_point, overlay_image_colour);
				}
			}
		}
	} else {
		button_image->blit_rect(overlay_image, overlay_image->get_used_rect(), {});
	}

	if (can_update) {
		combined_texture->update(button_image);
	} else {
		combined_texture->set_image(button_image);
	}
	_update_button_states();
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
}

GFXMaskedFlagTexture::GFXMaskedFlagTexture() {}

Ref<GFXMaskedFlagTexture> GFXMaskedFlagTexture::make_gfx_masked_flag_texture(GFX::MaskedFlag const* gfx_masked_flag) {
	Ref<GFXMaskedFlagTexture> masked_flag_texture;
	masked_flag_texture.instantiate();
	ERR_FAIL_NULL_V(masked_flag_texture, nullptr);

	ERR_FAIL_COND_V(masked_flag_texture->set_gfx_masked_flag(gfx_masked_flag) != OK, nullptr);

	return masked_flag_texture;
}

void GFXMaskedFlagTexture::clear() {
	gfx_masked_flag = nullptr;
	flag_country = nullptr;
	flag_type = String {};

	_clear_button_states();

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

	const StringName overlay_file = convert_to<String>(new_gfx_masked_flag->get_overlay_file());
	const Ref<Image> new_overlay_image = asset_manager->get_image(overlay_file);
	ERR_FAIL_NULL_V_MSG(new_overlay_image, FAILED, Utilities::format("Failed to load flag overlay image: %s", overlay_file));

	const StringName mask_file = convert_to<String>(new_gfx_masked_flag->get_mask_file());
	const Ref<Image> new_mask_image = asset_manager->get_image(mask_file);
	ERR_FAIL_NULL_V_MSG(new_mask_image, FAILED, Utilities::format("Failed to load flag mask image: %s", mask_file));

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
		new_masked_flag, FAILED, Utilities::format(
			"Invalid type for GFX sprite %s: %s (expected %s)", gfx_masked_flag_name,
			convert_to<String>(sprite->get_type()),
			convert_to<String>(GFX::MaskedFlag::get_type_static())
		)
	);

	return set_gfx_masked_flag(new_masked_flag);
}

String GFXMaskedFlagTexture::get_gfx_masked_flag_name() const {
	return gfx_masked_flag != nullptr ? convert_to<String>(gfx_masked_flag->get_name()) : String {};
}

Error GFXMaskedFlagTexture::set_flag_country_and_type(
	CountryDefinition const* new_flag_country, StringName const& new_flag_type
) {
	if (flag_country == new_flag_country && flag_type == new_flag_type) {
		return OK;
	}

	if (new_flag_country != nullptr) {
		GameSingleton const* game_singleton = GameSingleton::get_singleton();
		ERR_FAIL_NULL_V(game_singleton, FAILED);

		flag_image_rect = game_singleton->get_flag_sheet_rect(new_flag_country->get_index(), new_flag_type);
		ERR_FAIL_COND_V(!flag_image_rect.has_area(), FAILED);

		flag_country = new_flag_country;
		flag_type = new_flag_type;
		flag_image = game_singleton->get_flag_sheet_image();
	} else {
		// TODO - use REB flag as default/error flag
		flag_country = nullptr;
		flag_type = String {};
		flag_image.unref();
	}

	return _generate_combined_image();
}

Error GFXMaskedFlagTexture::set_flag_country_name_and_type(
	String const& new_flag_country_name, StringName const& new_flag_type
) {
	if (new_flag_country_name.is_empty()) {
		return set_flag_country_and_type(nullptr, {});
	}

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);

	CountryDefinition const* new_flag_country = game_singleton->get_definition_manager()
		.get_country_definition_manager()
		.get_country_definition_by_identifier(
			convert_to<std::string>(new_flag_country_name)
		);
	ERR_FAIL_NULL_V_MSG(new_flag_country, FAILED, Utilities::format("Country not found: %s", new_flag_country_name));

	return set_flag_country_and_type(new_flag_country, new_flag_type);
}

Error GFXMaskedFlagTexture::set_flag_country(CountryInstance* new_flag_country) {
	if (new_flag_country == nullptr) {
		return set_flag_country_and_type(nullptr, {});
	}

	GovernmentType const* government_type = new_flag_country->flag_government_type.get_untracked();

	const StringName new_flag_type = government_type != nullptr
		? StringName { convert_to<String>(government_type->get_flag_type()) }
		: StringName {};

	return set_flag_country_and_type(&new_flag_country->get_country_definition(), new_flag_type);
}

Error GFXMaskedFlagTexture::set_flag_country_name(String const& new_flag_country_name) {
	if (new_flag_country_name.is_empty()) {
		return set_flag_country(nullptr);
	}

	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);

	InstanceManager* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, FAILED);

	CountryInstance* new_flag_country = instance_manager->get_country_instance_manager()
		.get_country_instance_by_identifier(
			convert_to<std::string>(new_flag_country_name)
		);
	ERR_FAIL_NULL_V_MSG(new_flag_country, FAILED, Utilities::format("Country not found: %s", new_flag_country_name));

	return set_flag_country(new_flag_country);
}

String GFXMaskedFlagTexture::get_flag_country_name() const {
	return flag_country != nullptr ? convert_to<String>(flag_country->get_identifier()) : String {};
}
