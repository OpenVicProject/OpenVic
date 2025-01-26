#pragma once

#include <godot_cpp/classes/bit_map.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "openvic-extension/classes/GUIIcon.hpp"
#include "openvic-extension/classes/GUIIconButton.hpp"
#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUILineChart.hpp"
#include "openvic-extension/classes/GUIListBox.hpp"
#include "openvic-extension/classes/GUIMaskedFlag.hpp"
#include "openvic-extension/classes/GUIMaskedFlagButton.hpp"
#include "openvic-extension/classes/GUIOverlappingElementsBox.hpp"
#include "openvic-extension/classes/GUIPieChart.hpp"
#include "openvic-extension/classes/GUIProgressBar.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"

namespace OpenVic {
	class GUINode : public godot::Control {
		GDCLASS(GUINode, godot::Control)

		godot::Ref<godot::BitMap> _click_mask;
		godot::Vector<Control*> _mask_controls;
		godot::Rect2 _texture_region;
		godot::Rect2 _position_rect;

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

		static GUIIconButton* get_gui_icon_button_from_node(godot::Node* node);
		static GUIMaskedFlagButton* get_gui_masked_flag_button_from_node(godot::Node* node);
		static GUILabel* get_gui_label_from_node(godot::Node* node);
		static godot::Panel* get_panel_from_node(godot::Node* node);
		static GUIProgressBar* get_gui_progress_bar_from_node(godot::Node* node);
		static GUIIcon* get_gui_icon_from_node(godot::Node* node);
		static GUIMaskedFlag* get_gui_masked_flag_from_node(godot::Node* node);
		static GUIPieChart* get_gui_pie_chart_from_node(godot::Node* node);
		static GUIOverlappingElementsBox* get_gui_overlapping_elements_box_from_node(godot::Node* node);
		static GUIScrollbar* get_gui_scrollbar_from_node(godot::Node* node);
		static GUIListBox* get_gui_listbox_from_node(godot::Node* node);
		static godot::LineEdit* get_line_edit_from_node(godot::Node* node);
		static GUILineChart* get_gui_line_chart_from_node(godot::Node* node);

		GUIIconButton* get_gui_icon_button_from_nodepath(godot::NodePath const& path) const;
		GUIMaskedFlagButton* get_gui_masked_flag_button_from_nodepath(godot::NodePath const& path) const;
		GUILabel* get_gui_label_from_nodepath(godot::NodePath const& path) const;
		godot::Panel* get_panel_from_nodepath(godot::NodePath const& path) const;
		GUIProgressBar* get_gui_progress_bar_from_nodepath(godot::NodePath const& path) const;
		GUIIcon* get_gui_icon_from_nodepath(godot::NodePath const& path) const;
		GUIMaskedFlag* get_gui_masked_flag_from_nodepath(godot::NodePath const& path) const;
		GUIPieChart* get_gui_pie_chart_from_nodepath(godot::NodePath const& path) const;
		GUIOverlappingElementsBox* get_gui_overlapping_elements_box_from_nodepath(godot::NodePath const& path) const;
		GUIScrollbar* get_gui_scrollbar_from_nodepath(godot::NodePath const& path) const;
		GUIListBox* get_gui_listbox_from_nodepath(godot::NodePath const& path) const;
		godot::LineEdit* get_line_edit_from_nodepath(godot::NodePath const& path) const;
		GUILineChart* get_gui_line_chart_from_nodepath(godot::NodePath const& path) const;

		/* Helper functions to get textures from TextureRects and GUIButtons. */
		static godot::Ref<godot::Texture2D> get_texture_from_node(godot::Node* node);
		godot::Ref<godot::Texture2D> get_texture_from_nodepath(godot::NodePath const& path) const;

		godot::Error hide_node(godot::NodePath const& path) const;
		godot::Error hide_nodes(godot::TypedArray<godot::NodePath> const& paths) const;

		godot::Error remove_node(godot::NodePath const& path) const;
		godot::Error remove_nodes(godot::TypedArray<godot::NodePath> const& paths) const;

		static godot::String int_to_string_suffixed(int64_t val);
		static godot::String int_to_string_commas(int64_t val);
		static godot::String float_to_string_suffixed(float val);
		static godot::String float_to_string_dp(float val, int32_t decimal_places);
		// 3dp if abs(val) < 2 else 2dp if abs(val) < 10 else 1dp
		static godot::String float_to_string_dp_dynamic(float val);
		// The "ignore_empty" argument refers to what this function produces when given an empty string - if the argument
		// is false then empty inputs are replaced with "NO PROVINCE", otherwise they return the empty string unchanged.
		static godot::String format_province_name(godot::String const& province_identifier, bool ignore_empty = false);

		godot::Ref<godot::BitMap> get_click_mask() const;
		void set_click_mask(godot::Ref<godot::BitMap> const& mask);

		void set_click_mask_from_nodepaths(godot::TypedArray<godot::NodePath> const& paths);
		bool _update_click_mask_for(godot::Ref<godot::Image> const& img, int index);
		void update_click_mask();

		bool _has_point(godot::Vector2 const& point) const override;
	};
}
