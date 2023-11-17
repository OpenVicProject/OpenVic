#pragma once

#include <godot_cpp/classes/control.hpp>

#include <openvic-simulation/interface/GUI.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"

namespace OpenVic::GodotGUIBuilder {
	bool generate_element(GUI::Element const* element, AssetManager& asset_manager, godot::Control*& result);

	bool generate_icon(GUI::Element const& element, AssetManager& asset_manager, godot::Control*& result);
	bool generate_button(GUI::Element const& element, AssetManager& asset_manager, godot::Control*& result);
	bool generate_checkbox(GUI::Element const& element, AssetManager& asset_manager, godot::Control*& result);
	bool generate_text(GUI::Element const& element, AssetManager& asset_manager, godot::Control*& result);
	bool generate_overlapping_elements(GUI::Element const& element, AssetManager& asset_manager, godot::Control*& result);
	bool generate_listbox(GUI::Element const& element, AssetManager& asset_manager, godot::Control*& result);
	bool generate_window(GUI::Element const& element, AssetManager& asset_manager, godot::Control*& result);
}
