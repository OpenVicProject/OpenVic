#pragma once

#include <godot_cpp/classes/texture_progress_bar.hpp>

#include "openvic-simulation/interface/GFXSprite.hpp"

namespace OpenVic {
	class GUIProgressBar : public godot::TextureProgressBar {
		GDCLASS(GUIProgressBar, godot::TextureProgressBar)

	protected:
		static void _bind_methods();

	public:
		godot::Error set_gfx_progress_bar(GFX::ProgressBar const* progress_bar);
	};
}
