#pragma once

#include <godot_cpp/classes/texture_progress_bar.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>

#include "openvic-extension/classes/GUIHasTooltip.hpp"

namespace OpenVic {
	class GUIProgressBar : public godot::TextureProgressBar {
		GDCLASS(GUIProgressBar, godot::TextureProgressBar);

		GUI_TOOLTIP_DEFINITIONS

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		GUIProgressBar();

		godot::Error set_gfx_progress_bar(GFX::ProgressBar const* progress_bar);
	};
}
