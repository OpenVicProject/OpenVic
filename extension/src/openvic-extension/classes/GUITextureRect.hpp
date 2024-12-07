#pragma once

#include <godot_cpp/classes/texture_rect.hpp>

#include <openvic-extension/classes/GUIHasTooltip.hpp>

namespace OpenVic {
	class GUITextureRect : public godot::TextureRect {
		GDCLASS(GUITextureRect, godot::TextureRect)

		GUI_TOOLTIP_DEFINITIONS

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		GUITextureRect();
	};
}
