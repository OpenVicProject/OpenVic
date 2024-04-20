#pragma once

#include <godot_cpp/classes/control.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>
#include <openvic-simulation/interface/GUI.hpp>

namespace OpenVic::UITools {
	GFX::Sprite const* get_gfx_sprite(godot::String const& gfx_sprite);
	GUI::Element const* get_gui_element(godot::String const& gui_scene, godot::String const& gui_element);
	GUI::Position const* get_gui_position(godot::String const& gui_scene, godot::String const& gui_position);

	bool generate_gui_element(
		GUI::Element const* element, godot::String const& name, godot::Control*& result
	);
	bool generate_gui_element(
		godot::String const& gui_scene, godot::String const& gui_element, godot::String const& name, godot::Control*& result
	);
}
