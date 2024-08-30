#include "GUIMaskedFlag.hpp"

#include "openvic-extension/utility/ClassBindings.hpp"

using namespace godot;
using namespace OpenVic;

void GUIMaskedFlag::_bind_methods() {
	OV_BIND_METHOD(GUIMaskedFlag::get_gfx_masked_flag_texture);

	OV_BIND_METHOD(GUIMaskedFlag::set_gfx_masked_flag_name, { "gfx_masked_flag_name" });
	OV_BIND_METHOD(GUIMaskedFlag::get_gfx_masked_flag_name);

	OV_BIND_METHOD(GUIMaskedFlag::set_flag_country_name_and_type, { "flag_country_name", "flag_type" });
	OV_BIND_METHOD(GUIMaskedFlag::set_flag_country_name, { "flag_country_name" });
	OV_BIND_METHOD(GUIMaskedFlag::get_flag_country_name);
	OV_BIND_METHOD(GUIMaskedFlag::get_flag_type);
}

Error GUIMaskedFlag::set_gfx_masked_flag(GFX::MaskedFlag const* gfx_masked_flag) {
	const bool needs_setting = gfx_masked_flag_texture.is_null();

	if (needs_setting) {
		gfx_masked_flag_texture.instantiate();
		ERR_FAIL_NULL_V(gfx_masked_flag_texture, FAILED);
	}

	const Error err = gfx_masked_flag_texture->set_gfx_masked_flag(gfx_masked_flag);

	if (needs_setting) {
		set_texture(gfx_masked_flag_texture);
	}

	return err;
}

Ref<GFXMaskedFlagTexture> GUIMaskedFlag::get_gfx_masked_flag_texture() const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, nullptr);

	return gfx_masked_flag_texture;
}

Error GUIMaskedFlag::set_gfx_masked_flag_name(String const& gfx_masked_flag_name) {
	const bool needs_setting = gfx_masked_flag_texture.is_null();

	if (needs_setting) {
		gfx_masked_flag_texture.instantiate();
		ERR_FAIL_NULL_V(gfx_masked_flag_texture, FAILED);
	}

	const Error err = gfx_masked_flag_texture->set_gfx_masked_flag_name(gfx_masked_flag_name);

	if (needs_setting) {
		set_texture(gfx_masked_flag_texture);
	}

	return err;
}

String GUIMaskedFlag::get_gfx_masked_flag_name() const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, {});

	return gfx_masked_flag_texture->get_gfx_masked_flag_name();
}

Error GUIMaskedFlag::set_flag_country_name_and_type(String const& flag_country_name, StringName const& flag_type) const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, FAILED);

	return gfx_masked_flag_texture->set_flag_country_name_and_type(flag_country_name, flag_type);
}

Error GUIMaskedFlag::set_flag_country_name(String const& flag_country_name) const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, FAILED);

	return gfx_masked_flag_texture->set_flag_country_name(flag_country_name);
}

String GUIMaskedFlag::get_flag_country_name() const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, {});

	return gfx_masked_flag_texture->get_flag_country_name();
}

String GUIMaskedFlag::get_flag_type() const {
	ERR_FAIL_NULL_V(gfx_masked_flag_texture, {});

	return gfx_masked_flag_texture->get_flag_type();
}
