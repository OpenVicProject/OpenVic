#pragma once

#include <godot_cpp/classes/texture_rect.hpp>

namespace OpenVic {
	class GUITextureRect : public godot::TextureRect {
		GDCLASS(GUITextureRect, godot::TextureRect)

	protected:
		static void _bind_methods();
	};
}
