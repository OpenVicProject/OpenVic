#include "GUIMaskedFlagButton.hpp"

#include <openvic-extension/utility/ClassBindings.hpp>

using namespace godot;
using namespace OpenVic;

void GUIMaskedFlagButton::_bind_methods() {
	OV_BIND_METHOD(GUIMaskedFlagButton::get_gfx_masked_flag_texture);

	OV_BIND_METHOD(GUIMaskedFlagButton::set_gfx_masked_flag_name, { "gfx_masked_flag_name" });
	OV_BIND_METHOD(GUIMaskedFlagButton::get_gfx_masked_flag_name);

	OV_BIND_METHOD(GUIMaskedFlagButton::set_flag_country_name_and_type, { "flag_country_name", "flag_type" });
	OV_BIND_METHOD(GUIMaskedFlagButton::set_flag_country_name, { "flag_country_name" });
	OV_BIND_METHOD(GUIMaskedFlagButton::get_flag_country_name);
	OV_BIND_METHOD(GUIMaskedFlagButton::get_flag_type);
}

Error GUIMaskedFlagButton::set_gfx_masked_flag(GFX::MaskedFlag const* gfx_masked_flag) {
	const bool needs_setting = gfx_masked_flag_texture.is_null();

	if (needs_setting) {
		gfx_masked_flag_texture.instantiate();
		ERR_FAIL_NULL_V(gfx_masked_flag_texture, FAILED);
	}

	Error err = gfx_masked_flag_texture->set_gfx_masked_flag(gfx_masked_flag);

	if (needs_setting && set_gfx_button_state_having_texture(gfx_masked_flag_texture) != OK) {
		err = FAILED;
	}

	return err;
}

Ref<GFXMaskedFlagTexture> GUIMaskedFlagButton::get_gfx_masked_flag_texture() const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, nullptr);

	return gfx_masked_flag_texture;
}

Error GUIMaskedFlagButton::set_gfx_masked_flag_name(String const& gfx_masked_flag_name) {
	const bool needs_setting = gfx_masked_flag_texture.is_null();

	if (needs_setting) {
		gfx_masked_flag_texture.instantiate();
		ERR_FAIL_NULL_V(gfx_masked_flag_texture, FAILED);
	}

	Error err = gfx_masked_flag_texture->set_gfx_masked_flag_name(gfx_masked_flag_name);

	if (needs_setting && set_gfx_button_state_having_texture(gfx_masked_flag_texture) != OK) {
		err = FAILED;
	}

	return err;
}

String GUIMaskedFlagButton::get_gfx_masked_flag_name() const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, {});

	return gfx_masked_flag_texture->get_gfx_masked_flag_name();
}

Error GUIMaskedFlagButton::set_flag_country_name_and_type(String const& flag_country_name, StringName const& flag_type) const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, FAILED);

	return gfx_masked_flag_texture->set_flag_country_name_and_type(flag_country_name, flag_type);
}

Error GUIMaskedFlagButton::set_flag_country_name(String const& flag_country_name) const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, FAILED);

	return gfx_masked_flag_texture->set_flag_country_name(flag_country_name);
}

String GUIMaskedFlagButton::get_flag_country_name() const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, {});

	return gfx_masked_flag_texture->get_flag_country_name();
}

String GUIMaskedFlagButton::get_flag_type() const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, {});

	return gfx_masked_flag_texture->get_flag_type();
}
