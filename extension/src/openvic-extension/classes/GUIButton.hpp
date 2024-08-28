#pragma once

#include <godot_cpp/classes/button.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>

#include "openvic-extension/classes/GFXButtonStateTexture.hpp"

namespace OpenVic {
	class GUIButton : public godot::Button {
		GDCLASS(GUIButton, godot::Button)

	protected:
		static void _bind_methods();

		godot::Error set_gfx_button_state_having_texture(godot::Ref<GFXButtonStateHavingTexture> const& texture);

	public:
		godot::Error set_gfx_font(GFX::Font const* gfx_font);
	};
}
