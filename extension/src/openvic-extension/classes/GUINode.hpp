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
#include "openvic-extension/classes/GUIOverlappingElementsBox.hpp"

namespace OpenVic {
	class GUINode : public godot::Control {
		GDCLASS(GUINode, godot::Control)

	protected:
		static void _bind_methods();

	public:
		GUINode();

		static godot::Control* generate_gui_element(
			godot::String const& gui_scene, godot::String const& gui_element, godot::String const& name = ""
		);

		godot::Error add_gui_element(
			godot::String const& gui_scene, godot::String const& gui_element, godot::String const& name = ""
		);

		static godot::Vector2 get_gui_position(godot::String const& gui_scene, godot::String const& gui_position);

		static godot::Button* get_button_from_node(godot::Node* node);
		static godot::CheckBox* get_check_box_from_node(godot::Node* node);
		static godot::Label* get_label_from_node(godot::Node* node);
		static godot::Panel* get_panel_from_node(godot::Node* node);
		static godot::TextureProgressBar* get_progress_bar_from_node(godot::Node* node);
		static godot::TextureRect* get_texture_rect_from_node(godot::Node* node);
		static GUIOverlappingElementsBox* get_gui_overlapping_elements_box_from_node(godot::Node* node);

		godot::Button* get_button_from_nodepath(godot::NodePath const& path) const;
		godot::CheckBox* get_check_box_from_nodepath(godot::NodePath const& path) const;
		godot::Label* get_label_from_nodepath(godot::NodePath const& path) const;
		godot::Panel* get_panel_from_nodepath(godot::NodePath const& path) const;
		godot::TextureProgressBar* get_progress_bar_from_nodepath(godot::NodePath const& path) const;
		godot::TextureRect* get_texture_rect_from_nodepath(godot::NodePath const& path) const;
		GUIOverlappingElementsBox* get_gui_overlapping_elements_box_from_nodepath(godot::NodePath const& path) const;

		/* Helper functions to get textures from TextureRects and Buttons. */
		static godot::Ref<godot::Texture2D> get_texture_from_node(godot::Node* node);
		static godot::Ref<GFXIconTexture> get_gfx_icon_texture_from_node(godot::Node* node);
		static godot::Ref<GFXMaskedFlagTexture> get_gfx_masked_flag_texture_from_node(godot::Node* node);
		static godot::Ref<GFXPieChartTexture> get_gfx_pie_chart_texture_from_node(godot::Node* node);

		godot::Ref<godot::Texture2D> get_texture_from_nodepath(godot::NodePath const& path) const;
		godot::Ref<GFXIconTexture> get_gfx_icon_texture_from_nodepath(godot::NodePath const& path) const;
		godot::Ref<GFXMaskedFlagTexture> get_gfx_masked_flag_texture_from_nodepath(godot::NodePath const& path) const;
		godot::Ref<GFXPieChartTexture> get_gfx_pie_chart_texture_from_nodepath(godot::NodePath const& path) const;

		godot::Error hide_node(godot::NodePath const& path) const;
		godot::Error hide_nodes(godot::TypedArray<godot::NodePath> const& paths) const;
	};
}
