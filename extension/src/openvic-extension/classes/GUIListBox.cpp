#include "GUIListBox.hpp"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;
using namespace OpenVic::Utilities::literals;

StringName const& GUIListBox::_signal_scroll_index_changed() {
	return OV_SNAME(scroll_index_changed);
}

Error GUIListBox::_calculate_max_scroll_index(bool signal) {
	if (fixed) {
		if (fixed_item_count <= 0) {
			max_scroll_index = 0;
			fixed_visible_items = 0;
		} else if (fixed_item_height <= 0.0_real) {
			max_scroll_index = fixed_item_count - 1;
			fixed_visible_items = max_scroll_index;
		} else {
			const real_t max_height = get_size().height;

			fixed_visible_items = max_height / fixed_item_height;
			max_scroll_index = std::max(fixed_item_count - std::max(fixed_visible_items, 1), 0);
		}
	} else {
		const int32_t child_count = get_child_count();

		if (child_count <= 0) {
			max_scroll_index = 0;
		} else {
			const real_t max_height = get_size().height;

			real_t height_under_max_scroll_index = 0.0_real;

			max_scroll_index = child_count;

			while (max_scroll_index > 0) {
				max_scroll_index--;
				Control* child = Object::cast_to<Control>(get_child(max_scroll_index));
				if (child != nullptr) {
					height_under_max_scroll_index += child->get_size().height; /* Spacing is ignored */

					if (height_under_max_scroll_index > max_height) {
						if (max_scroll_index + 1 < child_count) {
							max_scroll_index++;
						}
						break;
					}
				}
			}
		}
	}

	ERR_FAIL_NULL_V(scrollbar, FAILED);

	const bool was_blocking_signals = scrollbar->is_blocking_signals();
	scrollbar->set_block_signals(true);
	scrollbar->set_step_count(max_scroll_index);
	scrollbar->set_block_signals(was_blocking_signals);

	scrollbar->set_visible(max_scroll_index > 0);

	set_scroll_index(scrollbar->get_value(), signal);

	return OK;
}

Error GUIListBox::_update_child_positions() {
	const int32_t child_count = get_child_count();
	const real_t max_height = get_size().height;
	const Vector2 offset = gui_listbox != nullptr ? Utilities::to_godot_fvec2(gui_listbox->get_items_offset()) : Vector2 {};

	real_t height = 0.0_real;

	const int32_t child_scroll_index = fixed ? 0 : scroll_index;

	for (int32_t index = 0; index < child_count; ++index) {
		Control* child = Object::cast_to<Control>(get_child(index));

		if (child != nullptr) {
			if (index < child_scroll_index) {
				child->hide();
			} else {
				child->set_position(offset + Vector2 { 0.0_real, height });

				height += child->get_size().height; /* Spacing is ignored */

				child->set_visible(height <= max_height);
			}
		}
	}

	return OK;
}

void GUIListBox::_bind_methods() {
	OV_BIND_METHOD(GUIListBox::clear);
	OV_BIND_METHOD(GUIListBox::clear_children, { "remaining_child_count" }, DEFVAL(0));

	OV_BIND_METHOD(GUIListBox::get_scroll_index);
	OV_BIND_METHOD(GUIListBox::set_scroll_index, { "new_scroll_index", "signal" }, DEFVAL(true));
	OV_BIND_METHOD(GUIListBox::get_max_scroll_index);

	OV_BIND_METHOD(GUIListBox::is_fixed);
	OV_BIND_METHOD(GUIListBox::get_fixed_item_count);
	OV_BIND_METHOD(GUIListBox::get_fixed_visible_items);
	OV_BIND_METHOD(GUIListBox::get_fixed_item_height);
	OV_BIND_METHOD(GUIListBox::set_fixed, { "item_count", "item_height", "signal" }, DEFVAL(true));
	OV_BIND_METHOD(GUIListBox::unset_fixed, { "signal" }, DEFVAL(true));

	OV_BIND_METHOD(GUIListBox::get_gui_listbox_name);
	OV_BIND_METHOD(GUIListBox::get_scrollbar);
	OV_BIND_METHOD(GUIListBox::sort_children, { "callable" });

	ADD_SIGNAL(MethodInfo(_signal_scroll_index_changed(), PropertyInfo(Variant::INT, "value")));
}

void GUIListBox::_notification(int what) {
	switch (what) {
	case NOTIFICATION_SORT_CHILDREN: {
		_calculate_max_scroll_index(!fixed);
	} break;
	}
}

GUIListBox::GUIListBox() : fixed_item_height { 0.0_real } {}

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

void GUIListBox::_gui_input(Ref<InputEvent> const& event) {
	ERR_FAIL_NULL(event);

	if (scrollbar == nullptr) {
		return;
	}

	Ref<InputEventMouseButton> mb = event;

	if (mb.is_valid()) {
		if (mb->is_pressed()) {
			if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_UP) {
				scrollbar->decrement_value();
			} else if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_WHEEL_DOWN) {
				scrollbar->increment_value();
			} else {
				return;
			}
			accept_event();
		}
	}
}

void GUIListBox::clear() {
	gui_listbox = nullptr;
	scroll_index = 0;
	max_scroll_index = 0;

	fixed = false;
	fixed_item_count = 0;
	fixed_visible_items = 0;
	fixed_item_height = 0.0_real;

	clear_children();
	if (scrollbar != nullptr) {
		remove_child(scrollbar);
		scrollbar->queue_free();
		scrollbar = nullptr;
	}
}

void GUIListBox::clear_children(int32_t remaining_child_count) {
	ERR_FAIL_COND(remaining_child_count < 0);

	int32_t child_index = get_child_count();

	while (child_index > remaining_child_count) {
		Node* child = get_child(--child_index);
		remove_child(child);
		child->queue_free();
	}

	// Scrollbar value is left unchanged so we don't jump back to the top of the list when re-generating the same number
	// of entries. In cases where the value is greater than the new maximum scroll level, it will be clamped by
	// set_scroll_index, called via _calculate_max_scroll_index by _notification(NOTIFICATION_SORT_CHILDREN) after all the new
	// children have been added but before the same frame.
}

void GUIListBox::set_scroll_index(int32_t new_scroll_index, bool signal) {
	const int32_t old_scroll_index = scroll_index;

	scroll_index = std::clamp(new_scroll_index, 0, max_scroll_index);

	if (scrollbar != nullptr && scrollbar->get_value() != scroll_index) {
		const bool was_blocking_signals = scrollbar->is_blocking_signals();
		scrollbar->set_block_signals(true);
		scrollbar->set_value(scroll_index);
		scrollbar->set_block_signals(was_blocking_signals);
	}

	if (signal && scroll_index != old_scroll_index) {
		emit_signal(_signal_scroll_index_changed(), scroll_index);
	}

	_update_child_positions();
}

Error GUIListBox::set_fixed(int32_t item_count, real_t item_height, bool signal) {
	fixed = true;

	fixed_item_count = item_count;
	fixed_item_height = item_height;

	return _calculate_max_scroll_index(signal);
}

Error GUIListBox::unset_fixed(bool signal) {
	ERR_FAIL_COND_V(!fixed, FAILED);

	fixed = false;
	fixed_item_count = 0;
	fixed_item_height = 0.0_real;

	return _calculate_max_scroll_index(signal);
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

	const String scrollbar_name = Utilities::std_to_godot_string(gui_listbox->get_scrollbar_name());

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
				add_child(scrollbar, false, INTERNAL_MODE_BACK);

				const Size2 size = Utilities::to_godot_fvec2(gui_listbox->get_size());
				Vector2 position = Utilities::to_godot_fvec2(gui_listbox->get_scrollbar_offset());
				position.x += size.width;
				scrollbar->set_position(position);
				scrollbar->set_length_override(size.height);

				scrollbar->connect(
					GUIScrollbar::signal_value_changed(), Callable { this, OV_SNAME(set_scroll_index) }, CONNECT_PERSIST
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
	return gui_listbox != nullptr ? Utilities::std_to_godot_string(gui_listbox->get_name()) : String {};
}

GUIScrollbar* GUIListBox::get_scrollbar() const {
	return scrollbar;
}

Error GUIListBox::sort_children(Callable const& callable) {
	TypedArray<Node> children;
	ERR_FAIL_COND_V(children.resize(get_child_count()) != OK, FAILED);

	for (int64_t index = children.size() - 1; index >= 0; --index) {
		Node* child = get_child(index);

		children[index] = child;

		remove_child(child);
	}

	children.sort_custom(callable);

	for (int64_t index = 0; index < children.size(); ++index) {
		add_child(Object::cast_to<Node>(children[index]));
	}

	return OK;
}
