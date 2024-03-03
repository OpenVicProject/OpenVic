#pragma once

#include <godot_cpp/classes/container.hpp>

#include <openvic-simulation/interface/GUI.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"

namespace OpenVic {
	class GUIListBox : public godot::Container {
		GDCLASS(GUIListBox, godot::Container)

		GUI::ListBox const* PROPERTY(gui_listbox);

		GUIScrollbar* scrollbar;

		struct child_data_t {
			Control* child;
			real_t start_pos, height;
		};

		std::vector<child_data_t> children_data;

		/* The children_data index of the topmost visible child element. */
		int32_t PROPERTY(scroll_index);
		int32_t PROPERTY(max_scroll_index);

		godot::Error _calculate_child_arrangement();
		godot::Error _update_child_positions();

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		GUIListBox();

		godot::Vector2 _get_minimum_size() const override;
		void _gui_input(godot::Ref<godot::InputEvent> const& event) override;

		/* Reset gui_listbox to nullptr, and remove all child elements. */
		void clear();

		/* Remove all child elements except for the scrollbar. */
		void clear_children();

		void set_scroll_index(int32_t new_scroll_index);

		/* Set the GUI::ListBox. This does not affect any existing child elements. */
		godot::Error set_gui_listbox(GUI::ListBox const* new_gui_listbox);

		/* Return the name of the GUI::ListBox, or an empty String if it's null. */
		godot::String get_gui_listbox_name() const;

		GUIScrollbar* get_scrollbar() const;
	};
}
