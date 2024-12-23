#pragma once

#include <godot_cpp/classes/container.hpp>

#include <openvic-simulation/interface/GUI.hpp>

namespace OpenVic {
	class GUIOverlappingElementsBox : public godot::Container {
		GDCLASS(GUIOverlappingElementsBox, godot::Container)

		GUI::OverlappingElementsBox const* PROPERTY(gui_overlapping_elements_box, nullptr);
		GUI::Element const* PROPERTY(gui_child_element, nullptr);

		godot::Error _update_child_positions();

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		GUIOverlappingElementsBox();

		/* Reset gui_overlapping_elements_box and gui_child_element to nullptr, and remove all child elements. */
		void clear();

		/* Remove all child elements. */
		void clear_children();

		/* Add or remove child elements to reach the specified count. */
		godot::Error set_child_count(int32_t new_count);

		/* Set the GUI::OverlappingElementsBox. This does not affect any existing child elements. */
		godot::Error set_gui_overlapping_elements_box(GUI::OverlappingElementsBox const* new_gui_overlapping_elements_box);

		/* Return the name of the GUI::OverlappingElementsBox, or an empty String if it's null. */
		godot::String get_gui_overlapping_elements_box_name() const;

		/* Set the child GUI::Element, removing all previous child elements (even if the child GUI::Element doesn't change). */
		godot::Error set_gui_child_element(GUI::Element const* new_gui_child_element);

		/* Search for a GUI::Element with the specified name and, if successful,
		 * set the child element to it using set_gui_child_element. */
		godot::Error set_gui_child_element_name(
			godot::String const& gui_child_element_file, godot::String const& gui_child_element_name
		);

		/* Return the name of the child GUI::Element, or an empty String if it's null. */
		godot::String get_gui_child_element_name() const;
	};
}
