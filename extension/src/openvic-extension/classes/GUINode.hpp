#pragma once

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/texture_progress_bar.hpp>
#include <godot_cpp/classes/texture_rect.hpp>

#include <openvic-simulation/interface/GUI.hpp>

#include "openvic-extension/classes/GFXIconTexture.hpp"
#include "openvic-extension/classes/GFXMaskedFlagTexture.hpp"
#include "openvic-extension/classes/GFXPieChartTexture.hpp"

namespace OpenVic {
	class GUINode : public godot::Control {
		GDCLASS(GUINode, godot::Control)

		template<std::derived_from<godot::Node> T>
		T* _get_cast_node(godot::NodePath const& path) const;

		template<std::derived_from<godot::Texture2D> T>
		godot::Ref<T> _get_cast_texture_from_node(godot::NodePath const& path) const;

	protected:
		static void _bind_methods();

	public:
		GUINode() = default;

		godot::Error _add_gui_element(GUI::Element const* element, godot::String const& name);
		godot::Error add_gui_element(
			godot::String const& gui_file, godot::String const& gui_element, godot::String const& name = ""
		);

		godot::Button* get_button_node(godot::NodePath const& path) const;
		godot::CheckBox* get_check_box_node(godot::NodePath const& path) const;
		godot::Label* get_label_node(godot::NodePath const& path) const;
		godot::Panel* get_panel_node(godot::NodePath const& path) const;
		godot::TextureProgressBar* get_progress_bar_node(godot::NodePath const& path) const;
		godot::TextureRect* get_texture_rect_node(godot::NodePath const& path) const;

		/* Helper functions to get textures from TextureRects and Buttons. */
		godot::Ref<godot::Texture2D> get_texture_from_node(godot::NodePath const& path) const;
		godot::Ref<GFXIconTexture> get_gfx_icon_texture_from_node(godot::NodePath const& path) const;
		godot::Ref<GFXMaskedFlagTexture> get_gfx_masked_flag_texture_from_node(godot::NodePath const& path) const;
		godot::Ref<GFXPieChartTexture> get_gfx_pie_chart_texture_from_node(godot::NodePath const& path) const;

		godot::Error hide_node(godot::NodePath const& path) const;
		godot::Error hide_nodes(godot::Array const& paths) const;
	};
}
