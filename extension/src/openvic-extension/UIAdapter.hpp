#pragma once

#include <godot_cpp/classes/control.hpp>

#include <openvic-simulation/interface/GUI.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"

namespace OpenVic::GodotGUIBuilder {

	bool generate_element(
		GUI::Element const* element, godot::String const& name, AssetManager& asset_manager, godot::Control*& result
	);

#define GEN_GUI_ARGS \
	GUI::Element const& element, godot::String const& name, AssetManager& asset_manager, godot::Control*& result

	bool generate_icon(GEN_GUI_ARGS);
	bool generate_button(GEN_GUI_ARGS);
	bool generate_checkbox(GEN_GUI_ARGS);
	bool generate_text(GEN_GUI_ARGS);
	bool generate_overlapping_elements(GEN_GUI_ARGS);
	bool generate_listbox(GEN_GUI_ARGS);
	bool generate_window(GEN_GUI_ARGS);

#undef GEN_GUI_ARGS

}
