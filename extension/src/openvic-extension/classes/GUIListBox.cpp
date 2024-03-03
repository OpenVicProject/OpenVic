#include "GUIListBox.hpp"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

using OpenVic::Utilities::std_view_to_godot_string;

Error GUIListBox::_calculate_child_arrangement() {
	ERR_FAIL_NULL_V(gui_listbox, FAILED);

	const int32_t child_count = get_child_count();
	const real_t max_height = get_size().height;

	real_t height = 0.0f, height_under_max_scroll_index = 0.0f;

	children_data.clear();
	max_scroll_index = 0;

	for (int32_t index = 0; index < child_count; ++index) {
		Control* child = Object::cast_to<Control>(get_child(index));
		if (child != nullptr && child != scrollbar && child->is_visible()) {
			const real_t child_height = child->get_size().height; /* Spacing is ignored */
			children_data.push_back({ child, height, child_height });

			height += child_height;
			height_under_max_scroll_index += child_height;

			while (height_under_max_scroll_index > max_height && max_scroll_index + 1 < children_data.size()) {
				height_under_max_scroll_index -= children_data[max_scroll_index++].height;
			}
		}
	}

	ERR_FAIL_NULL_V(scrollbar, FAILED);

	scrollbar->set_limits(0, max_scroll_index, false);
	scrollbar->emit_value_changed();
	scrollbar->set_visible(max_scroll_index > 0);

	return OK;
}

Error GUIListBox::_update_child_positions() {
	ERR_FAIL_NULL_V(gui_listbox, FAILED);

	if (children_data.empty()) {
		return OK;
	}

	const real_t max_height = get_size().height;
	const real_t scroll_pos = children_data[scroll_index].start_pos;

	for (int32_t index = 0; index < children_data.size(); ++index) {
		child_data_t const& data = children_data[index];
		if (index < scroll_index || data.start_pos + data.height > scroll_pos + max_height) {
			data.child->set_position({ 0.0f, max_height + 10.0f });
		} else {
			data.child->set_position({ 0.0f, data.start_pos - scroll_pos });
		}
	}

	return OK;
}

void GUIListBox::_bind_methods() {
	OV_BIND_METHOD(GUIListBox::clear);
	OV_BIND_METHOD(GUIListBox::clear_children);

	OV_BIND_METHOD(GUIListBox::get_scroll_index);
	OV_BIND_METHOD(GUIListBox::set_scroll_index, { "new_scroll_index" });
	OV_BIND_METHOD(GUIListBox::get_max_scroll_index);

	OV_BIND_METHOD(GUIListBox::get_gui_listbox_name);
	OV_BIND_METHOD(GUIListBox::get_scrollbar);
}

void GUIListBox::_notification(int what) {
	switch (what) {
	case NOTIFICATION_SORT_CHILDREN: {
		_calculate_child_arrangement();
	} break;
	}
}

GUIListBox::GUIListBox()
  : gui_listbox { nullptr }, scrollbar { nullptr }, children_data {}, scroll_index { 0 }, max_scroll_index { 0 } {
	set_clip_contents(true);
}

Vector2 GUIListBox::_get_minimum_size() const {
	if (gui_listbox != nullptr) {
		Size2 size = Utilities::to_godot_fvec2(gui_listbox->get_size());

		if (scrollbar != nullptr) {
			size.width += scrollbar->get_minimum_size().width;
		}

		return size;
	} else {
		return {};
	}
}

void GUIListBox::_gui_input(godot::Ref<godot::InputEvent> const& event) {
	ERR_FAIL_NULL(event);

	Ref<InputEventMouseButton> mb = event;

	if (mb.is_valid()) {
		if (mb->is_pressed()) {
			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_UP) {
				set_scroll_index(scroll_index - 1);
			} else if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_DOWN) {
				set_scroll_index(scroll_index + 1);
			} else {
				return;
			}
			accept_event();
		}
	}
}

void GUIListBox::clear() {
	gui_listbox = nullptr;
	children_data.clear();
	scroll_index = 0;
	max_scroll_index = 0;
	clear_children();
	if (scrollbar != nullptr) {
		remove_child(scrollbar);
		scrollbar = nullptr;
	}
}

void GUIListBox::clear_children() {
	int32_t child_count = get_child_count();
	while (child_count > 0) {
		Node* child = get_child(--child_count);
		if (child != scrollbar) {
			remove_child(child);
		}
	}
	if (scrollbar != nullptr) {
		scrollbar->set_value(0);
	}
}

void GUIListBox::set_scroll_index(int32_t new_scroll_index) {
	scroll_index = std::clamp(new_scroll_index, 0, max_scroll_index);
	if (scrollbar != nullptr && scrollbar->get_value() != scroll_index) {
		scrollbar->set_value(scroll_index, false);
	}
	_update_child_positions();
}

Error GUIListBox::set_gui_listbox(GUI::ListBox const* new_gui_listbox) {
	if (gui_listbox == new_gui_listbox) {
		return OK;
	}

	if (new_gui_listbox == nullptr) {
		clear();
		return OK;
	}

	gui_listbox = new_gui_listbox;

	const String scrollbar_name = std_view_to_godot_string(gui_listbox->get_scrollbar_name());

	Error err = OK;

	if (scrollbar_name.is_empty()) {
		UtilityFunctions::push_error("GUIListBox ", get_name(), " has no scrollbar name!");
		err = FAILED;
		scrollbar = nullptr;
	} else {
		static const String scrollbar_scene = "core";

		Control* scrollbar_control = nullptr;
		if (!UITools::generate_gui_element(scrollbar_scene, scrollbar_name, "", scrollbar_control)) {
			UtilityFunctions::push_error("Error generating scrollbar ", scrollbar_name, " for GUIListBox ", get_name());
			err = FAILED;
		}

		if (scrollbar_control != nullptr) {
			scrollbar = Object::cast_to<GUIScrollbar>(scrollbar_control);
			if (scrollbar != nullptr) {
				add_child(scrollbar);

				const Size2 size = Utilities::to_godot_fvec2(gui_listbox->get_size());
				scrollbar->set_position({ size.width, 0.0f });
				scrollbar->set_length_override(size.height);

				static const StringName set_scroll_index_func_name = "set_scroll_index";

				scrollbar->connect(
					GUIScrollbar::signal_value_changed(), Callable { this, set_scroll_index_func_name }, CONNECT_PERSIST
				);
			} else {
				UtilityFunctions::push_error(
					"Element ", scrollbar_name, " for GUIListBox ", get_name(), " is not a GUIScrollbar"
				);
				err = FAILED;
			}
		} else {
			scrollbar = nullptr;
		}
	}

	return err;
}

String GUIListBox::get_gui_listbox_name() const {
	return gui_listbox != nullptr ? std_view_to_godot_string(gui_listbox->get_name()) : String {};
}

GUIScrollbar* GUIListBox::get_scrollbar() const {
	return scrollbar;
}
