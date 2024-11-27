#pragma once

#include <openvic-extension/classes/GFXMaskedFlagTexture.hpp>
#include <openvic-extension/classes/GUIButton.hpp>

namespace OpenVic {
	class GUIMaskedFlagButton : public GUIButton {
		GDCLASS(GUIMaskedFlagButton, GUIButton)

		godot::Ref<GFXMaskedFlagTexture> gfx_masked_flag_texture;

	protected:
		static void _bind_methods();

	public:
		godot::Error set_gfx_masked_flag(GFX::MaskedFlag const* gfx_masked_flag);

		godot::Ref<GFXMaskedFlagTexture> get_gfx_masked_flag_texture() const;

		godot::Error set_gfx_masked_flag_name(godot::String const& gfx_masked_flag_name);

		godot::String get_gfx_masked_flag_name() const;

		godot::Error set_flag_country_name_and_type( //
			godot::String const& flag_country_name, godot::StringName const& flag_type
		) const;

		godot::Error set_flag_country_name(godot::String const& flag_country_name) const;

		godot::String get_flag_country_name() const;

		godot::String get_flag_type() const;
	};
}
