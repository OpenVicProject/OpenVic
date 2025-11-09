#include "GUIOverlappingElementsBox.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"
#include "openvic-simulation/types/TextFormat.hpp"

using namespace OpenVic;
using namespace godot;

Error GUIOverlappingElementsBox::_update_child_positions() {
	ERR_FAIL_NULL_V(gui_overlapping_elements_box, FAILED);
	const int32_t child_count = get_child_count();
	if (child_count <= 0) {
		return OK;
	}

	const auto _get_child = [this](int32_t index) -> Control* { return Object::cast_to<Control>(get_child(index)); };

	const float box_width = get_size().x;
	const float child_width = _get_child(0)->get_size().x;

	const float max_spacing = box_width / (child_count + 1);
	const float default_spacing = child_width + static_cast<float>(gui_overlapping_elements_box->get_spacing());

	const float spacing = std::min(max_spacing, default_spacing);

	const float total_width = spacing * (child_count - 1) + child_width;

	float starting_x = 0.0f;
	Error err = OK;
	using enum text_format_t;
	switch (gui_overlapping_elements_box->get_format()) {
		case left:
			break;
		case centre:
			starting_x = (box_width - total_width) / 2;
			break;
		case right:
			starting_x = box_width - total_width;
			break;
		default:
			UtilityFunctions::push_error(
				"Invalid GUIOverlappingElementsBox alignment: ",
				static_cast<int32_t>(gui_overlapping_elements_box->get_format())
			);
			err = FAILED;
	}

	for (int32_t index = 0; index < child_count; ++index) {
		_get_child(index)->set_position({ starting_x + spacing * index, 0.0f });
	}

	return err;
}

void GUIOverlappingElementsBox::_bind_methods() {
	OV_BIND_METHOD(GUIOverlappingElementsBox::clear);
	OV_BIND_METHOD(GUIOverlappingElementsBox::clear_children);
	OV_BIND_METHOD(GUIOverlappingElementsBox::set_child_count, { "new_count" });

	OV_BIND_METHOD(GUIOverlappingElementsBox::get_gui_overlapping_elements_box_name);

	OV_BIND_METHOD(
		GUIOverlappingElementsBox::set_gui_child_element_name, { "gui_child_element_file", "gui_child_element_name" }
	);
	OV_BIND_METHOD(GUIOverlappingElementsBox::get_gui_child_element_name);
}

void GUIOverlappingElementsBox::_notification(int what) {
	if (what == NOTIFICATION_SORT_CHILDREN) {
		_update_child_positions();
	}
}

GUIOverlappingElementsBox::GUIOverlappingElementsBox() {}

void GUIOverlappingElementsBox::clear() {
	clear_children();
	gui_child_element = nullptr;
	gui_overlapping_elements_box = nullptr;
}

void GUIOverlappingElementsBox::clear_children() {
	set_child_count(0);
}

Error GUIOverlappingElementsBox::set_child_count(int32_t new_count) {
	ERR_FAIL_COND_V_MSG(new_count < 0, FAILED, "Child count must be non-negative");
	int32_t child_count = get_child_count();
	if (child_count == new_count) {
		return OK;
	} else if (child_count > new_count) {
		do {
			Node* child = get_child(--child_count);
			remove_child(child);
			child->queue_free();
		} while (child_count > new_count);
		return OK;
	} else {
		ERR_FAIL_NULL_V_MSG(
			gui_child_element, FAILED, Utilities::format(
				"GUIOverlappingElementsBox child element is null (child_count = %d, new_count = %d)", child_count, new_count
			)
		);
		Error err = OK;
		const String gui_child_element_name = Utilities::std_to_godot_string(gui_child_element->get_name()) + "_";
		do {
			Control* child = nullptr;
			const String name = gui_child_element_name + itos(child_count);
			if (!UITools::generate_gui_element(gui_child_element, name, child)) {
				UtilityFunctions::push_error("Error generating GUIOverlappingElementsBox child element ", name);
				err = FAILED;
			}
			ERR_FAIL_NULL_V_MSG(
				child, FAILED, Utilities::format(
					"Failed to generate GUIOverlappingElementsBox child element %s (child_count = %d, new_count = %d)",
					name, child_count, new_count
				)
			);

			/* Move the child element in front of its neighbours when moused-over. */
			child->connect(OV_SNAME(mouse_entered), Callable { child, OV_SNAME(set_z_index) }.bind(1), CONNECT_PERSIST);
			child->connect(OV_SNAME(mouse_exited), Callable { child, OV_SNAME(set_z_index) }.bind(0), CONNECT_PERSIST);

			add_child(child);
			child_count++;
		} while (child_count < new_count);
		return err;
	}
}

Error GUIOverlappingElementsBox::set_gui_overlapping_elements_box(
	GUI::OverlappingElementsBox const* new_gui_overlapping_elements_box
) {
	if (gui_overlapping_elements_box == new_gui_overlapping_elements_box) {
		return OK;
	}
	gui_overlapping_elements_box = new_gui_overlapping_elements_box;
	if (gui_overlapping_elements_box == nullptr) {
		return OK;
	}

	set_custom_minimum_size(Utilities::to_godot_fvec2(gui_overlapping_elements_box->get_size()));
	queue_sort();
	return OK;
}

String GUIOverlappingElementsBox::get_gui_overlapping_elements_box_name() const {
	return gui_overlapping_elements_box != nullptr
		? Utilities::std_to_godot_string(gui_overlapping_elements_box->get_name())
		: String {};
}

Error GUIOverlappingElementsBox::set_gui_child_element(GUI::Element const* new_gui_child_element) {
	clear_children();
	gui_child_element = new_gui_child_element;
	return OK;
}

Error GUIOverlappingElementsBox::set_gui_child_element_name(
	String const& gui_child_element_file, String const& gui_child_element_name
) {
	if (gui_child_element_file.is_empty() && gui_child_element_name.is_empty()) {
		return set_gui_child_element(nullptr);
	}
	ERR_FAIL_COND_V_MSG(
		gui_child_element_file.is_empty(), FAILED,
		Utilities::format("GUI child element file name is empty but element name is not: %s", gui_child_element_name)
	);
	ERR_FAIL_COND_V_MSG(
		gui_child_element_name.is_empty(), FAILED,
		Utilities::format("GUI child element name is empty but file name is not: %s", gui_child_element_file)
	);
	GUI::Element const* const new_gui_child_element = UITools::get_gui_element(gui_child_element_file, gui_child_element_name);
	ERR_FAIL_NULL_V(new_gui_child_element, FAILED);
	return set_gui_child_element(new_gui_child_element);
}

String GUIOverlappingElementsBox::get_gui_child_element_name() const {
	return gui_child_element != nullptr ? Utilities::std_to_godot_string(gui_child_element->get_name()) : String {};
}
