#pragma once

#include <godot_cpp/classes/container.hpp>

#include <openvic-simulation/interface/GUI.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"

namespace OpenVic {
	class GUIListBox : public godot::Container {
		GDCLASS(GUIListBox, godot::Container)

		GUI::ListBox const* PROPERTY(gui_listbox, nullptr);

		GUIScrollbar* scrollbar = nullptr;

		/* The children_data index of the topmost visible child element. */
		int32_t PROPERTY(scroll_index, 0);
		int32_t PROPERTY(max_scroll_index, 0);

		bool PROPERTY_CUSTOM_PREFIX(fixed, is, false);
		int32_t PROPERTY(fixed_item_count, 0);
		int32_t PROPERTY(fixed_visible_items, 0);
		real_t PROPERTY(fixed_item_height, 0.0);

		static godot::StringName const& _signal_scroll_index_changed();

		godot::Error _calculate_max_scroll_index(bool signal);
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

		/* Remove child elements until there are remaining_child_count left excluding the scrollbar. */
		void clear_children(int32_t remaining_child_count = 0);

		void set_scroll_index(int32_t new_scroll_index, bool signal = true);

		godot::Error set_fixed(int32_t item_count, real_t item_height, bool signal = true);
		godot::Error unset_fixed(bool signal = true);

		/* Set the GUI::ListBox. This does not affect any existing child elements. */
		godot::Error set_gui_listbox(GUI::ListBox const* new_gui_listbox);

		/* Return the name of the GUI::ListBox, or an empty String if it's null. */
		godot::String get_gui_listbox_name() const;

		GUIScrollbar* get_scrollbar() const;
	};
}
