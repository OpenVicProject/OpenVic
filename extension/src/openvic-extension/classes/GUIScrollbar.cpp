#include "GUIScrollbar.hpp"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/types/SliderValue.hpp>

#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

/* StringNames cannot be constructed until Godot has called StringName::setup(),
 * so we must use wrapper functions to delay their initialisation. */
StringName const& GUIScrollbar::signal_value_changed() {
	static const StringName signal_value_changed = "value_changed";
	return signal_value_changed;
}

GUI_TOOLTIP_IMPLEMENTATIONS(GUIScrollbar)

void GUIScrollbar::_bind_methods() {
	GUI_TOOLTIP_BIND_METHODS(GUIScrollbar)

	OV_BIND_METHOD(GUIScrollbar::emit_value_changed);
	OV_BIND_METHOD(GUIScrollbar::reset);
	OV_BIND_METHOD(GUIScrollbar::clear);

	OV_BIND_METHOD(GUIScrollbar::set_gui_scrollbar_name, { "gui_scene", "gui_scrollbar_name" });
	OV_BIND_METHOD(GUIScrollbar::get_gui_scrollbar_name);

	OV_BIND_METHOD(GUIScrollbar::get_orientation);

	OV_BIND_METHOD(GUIScrollbar::get_value);
	OV_BIND_METHOD(GUIScrollbar::get_value_as_ratio);
	OV_BIND_METHOD(GUIScrollbar::get_min_value);
	OV_BIND_METHOD(GUIScrollbar::get_max_value);
	OV_BIND_METHOD(GUIScrollbar::set_value, { "new_value", "signal" }, DEFVAL(true));
	OV_BIND_METHOD(GUIScrollbar::increment_value, { "signal" }, DEFVAL(true));
	OV_BIND_METHOD(GUIScrollbar::decrement_value, { "signal" }, DEFVAL(true));
	OV_BIND_METHOD(GUIScrollbar::set_value_as_ratio, { "new_ratio", "signal" }, DEFVAL(true));
	OV_BIND_METHOD(GUIScrollbar::get_value_scaled);

	OV_BIND_METHOD(GUIScrollbar::is_range_limited);
	OV_BIND_METHOD(GUIScrollbar::get_range_limit_min);
	OV_BIND_METHOD(GUIScrollbar::get_range_limit_max);
	OV_BIND_METHOD(GUIScrollbar::set_range_limits, { "new_range_limit_min", "new_range_limit_max", "signal" }, DEFVAL(true));
	OV_BIND_METHOD(GUIScrollbar::set_range_limits_and_value, {
		"new_range_limit_min", "new_range_limit_max", "new_value", "signal"
	}, DEFVAL(true));
	OV_BIND_METHOD(GUIScrollbar::set_limits, { "new_min_value", "new_max_value", "signal" }, DEFVAL(true));

	OV_BIND_METHOD(GUIScrollbar::get_length_override);
	OV_BIND_METHOD(GUIScrollbar::set_length_override, { "new_length_override" });

	ADD_SIGNAL(MethodInfo(signal_value_changed(), PropertyInfo(Variant::INT, "value")));
}

GUIScrollbar::GUIScrollbar() {
	/* Anything which the constructor might not have default initialised will be set by clear(). */
	clear();
}

void GUIScrollbar::_start_button_change(bool shift_pressed, bool control_pressed) {
	if (shift_pressed) {
		if (control_pressed) {
			button_change_value_base = max_value - min_value;
		} else {
			button_change_value_base = 10;
		}
	} else {
		if (control_pressed) {
			button_change_value_base = 100;
		} else {
			button_change_value_base = 1;
		}
	}
	if (orientation == HORIZONTAL) {
		button_change_value_base = -button_change_value_base;
	}
	button_change_value = 0;
	button_change_accelerate_timer = BUTTON_CHANGE_ACCELERATE_DELAY;
	button_change_timer = BUTTON_CHANGE_DELAY;
	set_physics_process_internal(true);
}

void GUIScrollbar::_stop_button_change() {
	/* This ensures value always changes by at least 1 if less/more is pressed. */
	if (button_change_value == 0) {
		button_change_value = button_change_value_base;
		_update_button_change();
	}
	set_physics_process_internal(false);
}

bool GUIScrollbar::_update_button_change() {
	if (pressed_less) {
		set_value(value + button_change_value);
	} else if (pressed_more) {
		set_value(value - button_change_value);
	} else {
		return false;
	}
	return true;
}

float GUIScrollbar::_value_to_ratio(int32_t val) const {
	return min_value != max_value
		? static_cast<float>(val - min_value) / (max_value - min_value)
		: 0.0f;
}

void GUIScrollbar::_calculate_rects() {
	update_minimum_size();

	const Size2 size = _get_minimum_size();

	if (orientation == HORIZONTAL) {
		slider_distance = size.width;

		if (less_texture.is_valid()) {
			less_rect = { {}, less_texture->get_size() };

			slider_distance -= less_rect.size.width;
		} else {
			less_rect = {};
		}

		if (more_texture.is_valid()) {
			const Size2 more_size = more_texture->get_size();

			more_rect = { { size.width - more_size.width, 0.0f }, more_size };

			slider_distance -= more_rect.size.width;
		} else {
			more_rect = {};
		}

		slider_start = less_rect.size.width;

		if (track_texture.is_valid()) {
			const real_t track_height = track_texture->get_height();

			track_rect = { { slider_start, 0.0f }, { slider_distance, track_height } };
		} else {
			track_rect = {};
		}

		if (slider_texture.is_valid()) {
			slider_rect = { {}, slider_texture->get_size() };

			slider_distance -= slider_rect.size.width;
		} else {
			slider_rect = {};
		}
	} else { /* VERTICAL */
		slider_distance = size.height;

		if (less_texture.is_valid()) {
			const Size2 less_size = less_texture->get_size();

			less_rect = { { 0.0f, size.height - less_size.height }, less_size };

			slider_distance -= less_rect.size.height;
		} else {
			less_rect = {};
		}

		if (more_texture.is_valid()) {
			more_rect = { {}, more_texture->get_size() };

			slider_distance -= more_rect.size.height;
		} else {
			more_rect = {};
		}

		slider_start = more_rect.size.height;
		const real_t average_button_width = (more_rect.size.width + less_rect.size.width) / 2.0f;

		if (track_texture.is_valid()) {
			const real_t track_width = track_texture->get_width();

			/* For some reason vertical scrollbar track textures overlap with their more buttons by a single pixel.
			 * They have a row of transparent pixels at the top to account for this, so we must also draw them
			 * one pixel taller to avoid having a gap between the track and the more button. */
			track_rect = {
				{ (average_button_width - track_width) / 2.0f, slider_start - 1.0f },
				{ track_width, slider_distance + 1.0f }
			};
		} else {
			track_rect = {};
		}

		if (slider_texture.is_valid()) {
			const Size2 slider_size = slider_texture->get_size();

			slider_rect = {
				{ (average_button_width - slider_size.width) / 2.0f, 0.0f },
				slider_size
			};

			slider_distance -= slider_rect.size.height;
		} else {
			slider_rect = {};
		}
	}

	if (range_limit_min_texture.is_valid()) {
		range_limit_min_rect = { {}, range_limit_min_texture->get_size() };
	} else {
		range_limit_min_rect = {};
	}

	if (range_limit_max_texture.is_valid()) {
		range_limit_max_rect = { {}, range_limit_max_texture->get_size() };
	} else {
		range_limit_max_rect = {};
	}
}

void GUIScrollbar::_constrain_value() {
	/* Clamp using range limits, as even when range limiting is disabled the limits will be set to min/max values. */
	value = std::clamp(value, range_limit_min, range_limit_max);

	slider_rect.position[orientation == HORIZONTAL ? 0 : 1] = slider_start + slider_distance * get_value_as_ratio();

	queue_redraw();
}

/* _constrain_value() should be called sometime after this. */
Error GUIScrollbar::_constrain_range_limits() {
	if (range_limited) {
		range_limit_min = std::clamp(range_limit_min, min_value, max_value);
		range_limit_max = std::clamp(range_limit_max, min_value, max_value);

		Error err = OK;
		if (range_limit_min > range_limit_max) {
			UtilityFunctions::push_error(
				"GUIScrollbar range max ", range_limit_max, " is less than range min ", range_limit_min, " - swapping values."
			);
			std::swap(range_limit_min, range_limit_max);
			err = FAILED;
		}

		const int axis = orientation == HORIZONTAL ? 0 : 1;
		range_limit_min_rect.position[axis] = slider_start + slider_distance * _value_to_ratio(range_limit_min);
		range_limit_max_rect.position[axis] = slider_start + slider_distance * _value_to_ratio(range_limit_max)
			+ slider_rect.size[axis] / 2.0f;

		return err;
	} else {
		range_limit_min = min_value;
		range_limit_max = max_value;
		return OK;
	}
}

/* _constrain_range_limits() should be called sometime after this. */
Error GUIScrollbar::_constrain_limits() {
	if (min_value <= max_value) {
		return OK;
	} else {
		UtilityFunctions::push_error(
			"GUIScrollbar max value ", max_value, " is less than min value ", min_value, " - swapping values."
		);
		std::swap(min_value, max_value);
		return FAILED;
	}
}

Vector2 GUIScrollbar::_get_minimum_size() const {
	if (gui_scrollbar != nullptr) {
		Size2 size = Utilities::to_godot_fvec2(gui_scrollbar->get_size());

		const int axis = orientation == HORIZONTAL ? 1 : 0;
		if (less_texture.is_valid()) {
			size[axis] = std::max(size[axis], less_texture->get_size()[axis]);
		}
		if (more_texture.is_valid()) {
			size[axis] = std::max(size[axis], more_texture->get_size()[axis]);
		}

		if (length_override > 0.0f) {
			size[1 - axis] = length_override;
		}

		return size;
	} else {
		return {};
	}
}

void GUIScrollbar::emit_value_changed() {
	emit_signal(signal_value_changed(), value);
}

Error GUIScrollbar::reset() {
	set_physics_process_internal(false);
	button_change_accelerate_timer = 0.0;
	button_change_timer = 0.0;
	button_change_value_base = 0;
	button_change_value = 0;

	hover_slider = false;
	hover_track = false;
	hover_less = false;
	hover_more = false;
	pressed_slider = false;
	pressed_track = false;
	pressed_less = false;
	pressed_more = false;

	value = min_value;
	range_limit_min = min_value;
	range_limit_max = max_value;

	const Error err = _constrain_range_limits();
	_constrain_value();
	emit_value_changed();
	return err;
}

void GUIScrollbar::clear() {
	gui_scrollbar = nullptr;

	slider_texture.unref();
	track_texture.unref();
	less_texture.unref();
	more_texture.unref();

	slider_rect = {};
	track_rect = {};
	less_rect = {};
	more_rect = {};

	range_limit_min_texture.unref();
	range_limit_max_texture.unref();

	range_limit_min_rect = {};
	range_limit_max_rect = {};

	orientation = HORIZONTAL;
	length_override = 0.0f;
	min_value = 0;
	max_value = 100;
	range_limited = false;

	_calculate_rects();

	_constrain_limits();
	reset();
}

Error GUIScrollbar::set_gui_scrollbar(GUI::Scrollbar const* new_gui_scrollbar) {
	if (gui_scrollbar == new_gui_scrollbar) {
		return OK;
	}
	if (new_gui_scrollbar == nullptr) {
		clear();
		return OK;
	}

	bool ret = true;

	gui_scrollbar = new_gui_scrollbar;

	const String gui_scrollbar_name = Utilities::std_to_godot_string(gui_scrollbar->get_name());

	orientation = gui_scrollbar->is_horizontal() ? HORIZONTAL : VERTICAL;
	length_override = 0.0f;
	range_limited = gui_scrollbar->is_range_limited();

	/* _Element is either GUI::Button or GUI::Icon, both of which have their own
	 * separately defined get_sprite(), hence the need for a template. */
	const auto set_texture = [&gui_scrollbar_name]<typename _Element>(
		String const& target, _Element const* element, Ref<GFXSpriteTexture>& texture
	) -> bool {
		ERR_FAIL_NULL_V_MSG(element, false, vformat(
			"Invalid %s element for GUIScrollbar %s - null!", target, gui_scrollbar_name
		));
		const String element_name = Utilities::std_to_godot_string(element->get_name());

		/* Get Sprite, convert to TextureSprite, use to make a GFXSpriteTexture. */
		GFX::Sprite const* sprite = element->get_sprite();
		ERR_FAIL_NULL_V_MSG(sprite, false, vformat(
			"Invalid %s element %s for GUIScrollbar %s - sprite is null!", target, element_name, gui_scrollbar_name
		));
		GFX::TextureSprite const* texture_sprite = sprite->cast_to<GFX::TextureSprite>();
		ERR_FAIL_NULL_V_MSG(texture_sprite, false, vformat(
			"Invalid %s element %s for GUIScrollbar %s - sprite type is %s with base type %s, expected base %s!", target,
			element_name, gui_scrollbar_name, Utilities::std_to_godot_string(sprite->get_type()),
			Utilities::std_to_godot_string(sprite->get_base_type()),
			Utilities::std_to_godot_string(GFX::TextureSprite::get_type_static())
		));
		texture = GFXSpriteTexture::make_gfx_sprite_texture(texture_sprite);
		ERR_FAIL_NULL_V_MSG(texture, false, vformat(
			"Failed to make GFXSpriteTexture from %s element %s for GUIScrollbar %s!", target, element_name, gui_scrollbar_name
		));
		if constexpr (std::is_same_v<_Element, GUI::Button>) {
			using enum GFXButtonStateTexture::ButtonState;
			for (GFXButtonStateTexture::ButtonState state : { HOVER, PRESSED }) {
				ERR_FAIL_NULL_V_MSG(texture->get_button_state_texture(state), false, vformat(
					"Failed to generate %s texture for %s element %s for GUIScrollbar %s!",
					GFXButtonStateTexture::button_state_to_name(state), target, element_name, gui_scrollbar_name
				));
			}
		}
		return true;
	};

	static const String SLIDER_NAME = "slider";
	static const String TRACK_NAME = "track";
	static const String LESS_NAME = "less";
	static const String MORE_NAME = "more";
	static const String RANGE_LIMIT_MIN_NAME = "range limit min";
	static const String RANGE_LIMIT_MAX_NAME = "range limit max";

	ret &= set_texture(SLIDER_NAME, gui_scrollbar->get_slider_button(), slider_texture);
	ret &= set_texture(TRACK_NAME, gui_scrollbar->get_track_button(), track_texture);
	ret &= set_texture(LESS_NAME, gui_scrollbar->get_less_button(), less_texture);
	ret &= set_texture(MORE_NAME, gui_scrollbar->get_more_button(), more_texture);
	if (range_limited) {
		ret &= set_texture(RANGE_LIMIT_MIN_NAME, gui_scrollbar->get_range_limit_min_icon(), range_limit_min_texture);
		ret &= set_texture(RANGE_LIMIT_MAX_NAME, gui_scrollbar->get_range_limit_max_icon(), range_limit_max_texture);
	} else {
		range_limit_min_texture.unref();
		range_limit_max_texture.unref();
	}

	_calculate_rects();

	ret &= set_step_size_and_limits_fp(
		gui_scrollbar->get_step_size(), gui_scrollbar->get_min_value(), gui_scrollbar->get_max_value()
	) == OK;

	return ERR(ret);
}

Error GUIScrollbar::set_gui_scrollbar_name(String const& gui_scene, String const& gui_scrollbar_name) {
	if (gui_scene.is_empty() && gui_scrollbar_name.is_empty()) {
		return set_gui_scrollbar(nullptr);
	}
	ERR_FAIL_COND_V_MSG(
		gui_scene.is_empty() || gui_scrollbar_name.is_empty(), FAILED, "GUI scene or scrollbar name is empty!"
	);

	GUI::Element const* gui_element = UITools::get_gui_element(gui_scene, gui_scrollbar_name);
	ERR_FAIL_NULL_V(gui_element, FAILED);
	GUI::Scrollbar const* new_gui_scrollbar = gui_element->cast_to<GUI::Scrollbar>();
	ERR_FAIL_NULL_V(new_gui_scrollbar, FAILED);
	return set_gui_scrollbar(new_gui_scrollbar);
}

String GUIScrollbar::get_gui_scrollbar_name() const {
	return gui_scrollbar != nullptr ? Utilities::std_to_godot_string(gui_scrollbar->get_name()) : String {};
}

void GUIScrollbar::set_value(int32_t new_value, bool signal) {
	const int32_t old_value = value;
	value = new_value;
	_constrain_value();
	if (signal && value != old_value) {
		emit_value_changed();
	}
}

void GUIScrollbar::set_value_fp(fixed_point_t new_value, bool signal) {
	return set_value(new_value / step_size, signal);
}

void GUIScrollbar::set_value_from_slider_value(SliderValue const& slider_value, int32_t scale, bool signal) {
	set_value_fp(slider_value.get_value() * scale, signal);
}

void GUIScrollbar::increment_value(bool signal) {
	set_value(value + 1, signal);
}

void GUIScrollbar::decrement_value(bool signal) {
	set_value(value - 1, signal);
}

float GUIScrollbar::get_value_as_ratio() const {
	return _value_to_ratio(value);
}

void GUIScrollbar::set_value_as_ratio(float new_ratio, bool signal) {
	set_value(min_value + (max_value - min_value) * new_ratio, signal);
}

fixed_point_t GUIScrollbar::get_value_scaled_fp() const {
	return value * step_size;
}

float GUIScrollbar::get_value_scaled() const {
	return get_value_scaled_fp().to_float();
}

Error GUIScrollbar::set_range_limits(int32_t new_range_limit_min, int32_t new_range_limit_max, bool signal) {
	return set_range_limits_and_value(new_range_limit_min, new_range_limit_max, value, signal);
}

Error GUIScrollbar::set_range_limits_and_value(
	int32_t new_range_limit_min, int32_t new_range_limit_max, int32_t new_value, bool signal
) {
	ERR_FAIL_COND_V_MSG(!range_limited, FAILED, "Cannot set range limits of non-range-limited GUIScrollbar!");
	range_limit_min = new_range_limit_min;
	range_limit_max = new_range_limit_max;
	const Error err = _constrain_range_limits();
	set_value(new_value, signal);
	return err;
}

Error GUIScrollbar::set_limits(int32_t new_min_value, int32_t new_max_value, bool signal) {
	min_value = new_min_value;
	max_value = new_max_value;
	bool ret = _constrain_limits() == OK;
	ret &= _constrain_range_limits() == OK;
	set_value(value, signal);
	return ERR(ret);
}

Error GUIScrollbar::set_range_limits_and_value_fp(
	fixed_point_t new_range_limit_min, fixed_point_t new_range_limit_max, fixed_point_t new_value, bool signal
) {
	return set_range_limits_and_value(
		new_range_limit_min / step_size,
		new_range_limit_max / step_size,
		new_value / step_size,
		signal
	);
}

Error GUIScrollbar::set_range_limits_and_value_from_slider_value(
	SliderValue const& slider_value, int32_t scale, bool signal
) {
	return set_range_limits_and_value_fp(
		slider_value.get_min() * scale,
		slider_value.get_max() * scale,
		slider_value.get_value() * scale,
		signal
	);
}

Error GUIScrollbar::set_step_size_and_limits_fp(fixed_point_t new_step_size, int32_t new_min_value, int32_t new_max_value) {
	bool ret = true;

	step_size = new_step_size;
	if (step_size <= 0) {
		UtilityFunctions::push_error(
			"Invalid step size ", Utilities::fixed_point_to_string_dp(step_size, -1), " for GUIScrollbar ",
			get_name(), " - not positive! Defaulting to 1."
		);
		step_size = 1;
		ret = false;
	}

	min_value = new_min_value / step_size;
	max_value = new_max_value / step_size;

	ret &= _constrain_limits() == OK;
	ret &= reset() == OK;

	return ERR(ret);
}

void GUIScrollbar::set_length_override(real_t new_length_override) {
	ERR_FAIL_COND_MSG(
		length_override < 0, vformat("Invalid GUIScrollbar length override: %f - cannot be negative!", length_override)
	);

	length_override = new_length_override;

	_calculate_rects();
	_constrain_limits();
	_constrain_range_limits();
	_constrain_value();
}

void GUIScrollbar::_gui_input(Ref<InputEvent> const& event) {
	ERR_FAIL_NULL(event);

	Ref<InputEventMouseButton> mb = event;

	if (mb.is_valid()) {
		if (mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
			if (mb->is_pressed()) {
				if (less_rect.has_point(mb->get_position())) {
					pressed_less = true;
					_start_button_change(mb->is_shift_pressed(), mb->is_ctrl_pressed());
				} else if (more_rect.has_point(mb->get_position())) {
					pressed_more = true;
					_start_button_change(mb->is_shift_pressed(), mb->is_ctrl_pressed());
				} else if (slider_rect.has_point(mb->get_position())) {
					pressed_slider = true;
				} else if (track_rect.has_point(mb->get_position())) {
					pressed_track = true;
					const real_t click_pos = mb->get_position()[orientation == HORIZONTAL ? 0 : 1];
					set_value_as_ratio((click_pos - slider_start) / slider_distance);
				} else {
					return;
				}
			} else {
				if (pressed_less) {
					_stop_button_change();
					pressed_less = false;
				} else if (pressed_more) {
					_stop_button_change();
					pressed_more = false;
				} else if (pressed_slider) {
					pressed_slider = false;
				} else if (pressed_track) {
					pressed_track = false;
				} else {
					return;
				}
			}
			queue_redraw();
		}
		return;
	}

	Ref<InputEventMouseMotion> mm = event;

	if (mm.is_valid()) {
		if (pressed_track) {
			/* Switch to moving the slider if a track click is held and moved. */
			pressed_track = false;
			pressed_slider = true;
		}
		if (pressed_slider) {
			const real_t click_pos = mm->get_position()[orientation == HORIZONTAL ? 0 : 1];
			set_value_as_ratio((click_pos - slider_start) / slider_distance);
		}

		if (hover_slider != slider_rect.has_point(mm->get_position())) {
			hover_slider = !hover_slider;
			queue_redraw();
		}
		if (hover_track != track_rect.has_point(mm->get_position())) {
			hover_track = !hover_track;
			queue_redraw();
		}
		if (hover_less != less_rect.has_point(mm->get_position())) {
			hover_less = !hover_less;
			queue_redraw();
		}
		if (hover_more != more_rect.has_point(mm->get_position())) {
			hover_more = !hover_more;
			queue_redraw();
		}

		_set_tooltip_active(hover_slider || hover_track || hover_less || hover_more);

		return;
	}
}

void GUIScrollbar::_notification(int what) {
	// GUIScrollbar doesn't use _tooltip_notification, as we don't want to show tooltips when hovering over transparent parts.

	switch (what) {
	case NOTIFICATION_VISIBILITY_CHANGED:
	case NOTIFICATION_MOUSE_EXIT: {
		hover_slider = false;
		hover_track = false;
		hover_less = false;
		hover_more = false;
		queue_redraw();
	} break;

	case NOTIFICATION_MOUSE_EXIT_SELF: {
		_set_tooltip_active(false);
	} break;

	/* Pressing (and holding) less and more buttons. */
	case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
		const double delta = get_physics_process_delta_time();

		button_change_accelerate_timer -= delta;
		while (button_change_accelerate_timer <= 0.0) {
			button_change_accelerate_timer += BUTTON_CHANGE_ACCELERATE_DELAY;
			button_change_value += button_change_value_base;
		}

		button_change_timer -= delta;
		while (button_change_timer <= 0.0) {
			button_change_timer += BUTTON_CHANGE_DELAY;
			if (!_update_button_change()) {
				set_physics_process_internal(false);
			}
		}
	} break;

	case NOTIFICATION_DRAW: {
		const RID ci = get_canvas_item();

		using enum GFXButtonStateTexture::ButtonState;

		if (less_texture.is_valid()) {
			Ref<Texture2D> less_state_texture;
			if (pressed_less) {
				less_state_texture = less_texture->get_button_state_texture(PRESSED);
			} else if (hover_less) {
				less_state_texture = less_texture->get_button_state_texture(HOVER);
			}
			if (less_state_texture.is_null()) {
				less_state_texture = less_texture;
			}
			less_state_texture->draw_rect(ci, less_rect, false);
		}

		if (more_texture.is_valid()) {
			Ref<Texture2D> more_state_texture;
			if (pressed_more) {
				more_state_texture = more_texture->get_button_state_texture(PRESSED);
			} else if (hover_more) {
				more_state_texture = more_texture->get_button_state_texture(HOVER);
			}
			if (more_state_texture.is_null()) {
				more_state_texture = more_texture;
			}
			more_state_texture->draw_rect(ci, more_rect, false);
		}

		if (track_texture.is_valid()) {
			Ref<GFXCorneredTileSupportingTexture> track_state_texture;
			if (pressed_track) {
				track_state_texture = track_texture->get_button_state_texture(PRESSED);
			} else if (hover_track) {
				track_state_texture = track_texture->get_button_state_texture(HOVER);
			}
			if (track_state_texture.is_null()) {
				track_state_texture = track_texture;
			}
			track_state_texture->draw_rect_cornered(ci, track_rect);
		}

		if (slider_texture.is_valid()) {
			Ref<Texture2D> slider_state_texture;
			if (pressed_slider) {
				slider_state_texture = slider_texture->get_button_state_texture(PRESSED);
			} else if (hover_slider) {
				slider_state_texture = slider_texture->get_button_state_texture(HOVER);
			}
			if (slider_state_texture.is_null()) {
				slider_state_texture = slider_texture;
			}
			slider_state_texture->draw_rect(ci, slider_rect, false);
		}

		if (range_limited) {
			if (range_limit_min != min_value && range_limit_min_texture.is_valid()) {
				range_limit_min_texture->draw_rect(ci, range_limit_min_rect, false);
			}

			if (range_limit_max != max_value && range_limit_max_texture.is_valid()) {
				range_limit_max_texture->draw_rect(ci, range_limit_max_rect, false);
			}
		}
	} break;
	}
}
