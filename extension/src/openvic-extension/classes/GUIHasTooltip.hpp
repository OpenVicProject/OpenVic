#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include <openvic-simulation/utility/Getters.hpp>

#include "openvic-extension/singletons/MenuSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

/* To add tooltip functionality to a class:
 *  - the class must be derived from Control.
 *  - add GUI_TOOLTIP_DEFINITIONS to the class definition, bearing in mind that it leaves visibility as private.
 *  - add GUI_TOOLTIP_IMPLEMENTATIONS(CLASS) to the class' source file.
 *  - add GUI_TOOLTIP_BIND_METHODS(CLASS) to the class' _bind_methods implementation.
 *  - call _tooltip_notification from the class' _notification method.
 *  - initialise tooltip_active to false in the class' constructor. */

#define GUI_TOOLTIP_DEFINITIONS \
	public: \
		void set_tooltip_string_and_substitution_dict( \
			godot::String const& new_tooltip_string, godot::Dictionary const& new_tooltip_substitution_dict \
		); \
		void set_tooltip_string(godot::String const& new_tooltip_string); \
		void set_tooltip_substitution_dict(godot::Dictionary const& new_tooltip_substitution_dict); \
		void clear_tooltip(); \
	private: \
		godot::String PROPERTY(tooltip_string); \
		godot::Dictionary PROPERTY(tooltip_substitution_dict); \
		bool PROPERTY_CUSTOM_PREFIX(tooltip_active, is); \
		void _tooltip_notification(int what); \
		void _set_tooltip_active(bool new_tooltip_active); \
		void _set_tooltip_visibility(bool visible);

#define GUI_TOOLTIP_IMPLEMENTATIONS(CLASS) \
	void CLASS::set_tooltip_string_and_substitution_dict( \
		String const& new_tooltip_string, Dictionary const& new_tooltip_substitution_dict \
	) { \
		if (get_mouse_filter() == MOUSE_FILTER_IGNORE) { \
			UtilityFunctions::push_error("Tooltips won't work for \"", get_name(), "\" as it has MOUSE_FILTER_IGNORE"); \
		} \
		if (tooltip_string != new_tooltip_string || tooltip_substitution_dict != new_tooltip_substitution_dict) { \
			tooltip_string = new_tooltip_string; \
			tooltip_substitution_dict = new_tooltip_substitution_dict; \
			if (tooltip_active) { \
				_set_tooltip_visibility(!tooltip_string.is_empty()); \
			} \
		} \
	} \
	void CLASS::set_tooltip_string(String const& new_tooltip_string) { \
		if (get_mouse_filter() == MOUSE_FILTER_IGNORE) { \
			UtilityFunctions::push_error("Tooltips won't work for \"", get_name(), "\" as it has MOUSE_FILTER_IGNORE"); \
		} \
		if (tooltip_string != new_tooltip_string) { \
			tooltip_string = new_tooltip_string; \
			if (tooltip_active) { \
				_set_tooltip_visibility(!tooltip_string.is_empty()); \
			} \
		} \
	} \
	void CLASS::set_tooltip_substitution_dict(Dictionary const& new_tooltip_substitution_dict) { \
		if (get_mouse_filter() == MOUSE_FILTER_IGNORE) { \
			UtilityFunctions::push_error("Tooltips won't work for \"", get_name(), "\" as it has MOUSE_FILTER_IGNORE"); \
		} \
		if (tooltip_substitution_dict != new_tooltip_substitution_dict) { \
			tooltip_substitution_dict = new_tooltip_substitution_dict; \
			if (tooltip_active) { \
				_set_tooltip_visibility(!tooltip_string.is_empty()); \
			} \
		} \
	} \
	void CLASS::clear_tooltip() { \
		set_tooltip_string_and_substitution_dict({}, {}); \
	} \
	void CLASS::_tooltip_notification(int what) { \
		if (what == NOTIFICATION_MOUSE_ENTER_SELF) { \
			_set_tooltip_active(true); \
		} else if (what == NOTIFICATION_MOUSE_EXIT_SELF) { \
			_set_tooltip_active(false); \
		} \
	} \
	void CLASS::_set_tooltip_active(bool new_tooltip_active) { \
		if (tooltip_active != new_tooltip_active) { \
			tooltip_active = new_tooltip_active; \
			if (!tooltip_string.is_empty()) { \
				_set_tooltip_visibility(tooltip_active); \
			} \
		} \
	} \
	void CLASS::_set_tooltip_visibility(bool visible) { \
		MenuSingleton* menu_singleton = MenuSingleton::get_singleton(); \
		ERR_FAIL_NULL(menu_singleton); \
		if (visible) { \
			menu_singleton->show_control_tooltip(tooltip_string, tooltip_substitution_dict, this); \
		} else { \
			menu_singleton->hide_tooltip(); \
		} \
	}

#define GUI_TOOLTIP_BIND_METHODS(CLASS) \
	OV_BIND_METHOD(CLASS::get_tooltip_string); \
	OV_BIND_METHOD(CLASS::set_tooltip_string, { "new_tooltip_string" }); \
	OV_BIND_METHOD(CLASS::get_tooltip_substitution_dict); \
	OV_BIND_METHOD(CLASS::set_tooltip_substitution_dict, { "new_tooltip_substitution_dict" }); \
	OV_BIND_METHOD( \
		CLASS::set_tooltip_string_and_substitution_dict, { "new_tooltip_string", "new_tooltip_substitution_dict" } \
	); \
	OV_BIND_METHOD(CLASS::clear_tooltip); \
	OV_BIND_METHOD(CLASS::is_tooltip_active); \
	ADD_PROPERTY( \
		PropertyInfo(Variant::STRING, "tooltip_string", PROPERTY_HINT_MULTILINE_TEXT), \
		"set_tooltip_string", "get_tooltip_string" \
	); \
	ADD_PROPERTY( \
		PropertyInfo(Variant::DICTIONARY, "tooltip_substitution_dict"), \
		"set_tooltip_substitution_dict", "get_tooltip_substitution_dict" \
	); \
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tooltip_active"), "", "is_tooltip_active");
