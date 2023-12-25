#pragma once

#include <godot_cpp/classes/control.hpp>

#include <openvic-simulation/interface/GFX.hpp>
#include <openvic-simulation/interface/GUI.hpp>

namespace OpenVic::UITools {
	GFX::Sprite const* get_gfx_sprite(godot::String const& gfx_sprite);
	GUI::Element const* get_gui_element(godot::String const& gui_file, godot::String const& gui_element);

	bool generate_gui_element(
		GUI::Element const* element, godot::String const& name, godot::Control*& result
	);
	bool generate_gui_element(
		godot::String const& gui_file, godot::String const& gui_element, godot::String const& name, godot::Control*& result
	);
}
