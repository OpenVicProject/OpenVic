#pragma once

#include <cstdint>

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>

#include <openvic-simulation/interface/GUI.hpp>
#include <openvic-simulation/types/Signal.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GFXSpriteTexture.hpp"
#include "openvic-extension/classes/GUIHasTooltip.hpp"

namespace OpenVic {
	struct SliderValue;

	class GUIScrollbar : public godot::Control {
		GDCLASS(GUIScrollbar, godot::Control)

		GUI_TOOLTIP_DEFINITIONS

		GUI::Scrollbar const* PROPERTY(gui_scrollbar, nullptr);

		godot::Ref<GFXSpriteTexture> slider_texture;
		godot::Ref<GFXSpriteTexture> track_texture;
		godot::Ref<GFXSpriteTexture> less_texture;
		godot::Ref<GFXSpriteTexture> more_texture;

		godot::Rect2 slider_rect;
		real_t slider_start = 0.0F, slider_distance = 0.0F;
		godot::Rect2 track_rect;
		godot::Rect2 less_rect;
		godot::Rect2 more_rect;

		godot::Ref<GFXSpriteTexture> range_limit_min_texture;
		godot::Ref<GFXSpriteTexture> range_limit_max_texture;

		godot::Rect2 range_limit_min_rect;
		godot::Rect2 range_limit_max_rect;

		godot::Orientation PROPERTY(orientation, godot::HORIZONTAL);
		real_t PROPERTY(length_override, 0.0);

		fixed_point_t offset = 0;
		int32_t scale_numerator = 1;
		int32_t scale_denominator = 1;
		int32_t PROPERTY(step_count, 1);
		int32_t PROPERTY(value, 0);

		bool PROPERTY_CUSTOM_PREFIX(range_limited, is, false);
		std::optional<int32_t> upper_range_limit;
		std::optional<int32_t> lower_range_limit;

		bool hover_slider = false, hover_track = false, hover_less = false, hover_more = false;
		bool pressed_slider = false, pressed_track = false, pressed_less = false, pressed_more = false;

		/* The time between value changes while the less/more button is held down (in seconds). */
		static constexpr double BUTTON_CHANGE_DELAY = 1.0 / 60.0;
		/* The time between value change rate increases while the less/more button is held down (in seconds). */
		static constexpr double BUTTON_CHANGE_ACCELERATE_DELAY = 10.0 * BUTTON_CHANGE_DELAY;

		/* When the less/more button is pressed, button_change_value_base is initialised based on the shift and control keys
		 * and button_change_value is initialised to 0. Every BUTTON_CHANGE_ACCELERATE_DELAY seconds button_change_value
		 * increases by button_change_value_base, and every BUTTON_CHANGE_DELAY seconds value changes by button_change_value.
		 * If button_change_value is still 0 when the button is released, then value will be increased by
		 * button_change_value_base so that short clicks still have an effect.*/
		int32_t button_change_value_base = 0, button_change_value = 0;
		double button_change_accelerate_timer = 0.0, button_change_timer = 0.0;

		void _start_button_change(bool shift_pressed, bool control_pressed);
		void _stop_button_change();

		/* Changes value by button_change_value with the direction determined by orientation and pressed_less or pressed_more.
		 * Returns true if a change occurred, otherwise false. */
		bool _update_button_change();

		float _value_to_ratio(int32_t val) const;
		int32_t _fp_to_value(const fixed_point_t val) const;
		fixed_point_t _get_scaled_value(const int32_t val) const;

		void _calculate_rects();

		void _constrain_value();
		void _constrain_range_limits();

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		static godot::StringName const& signal_value_changed();
		mutable signal_property<GUIScrollbar> value_changed;

		GUIScrollbar();

		godot::Vector2 _get_minimum_size() const override;
		void _gui_input(godot::Ref<godot::InputEvent> const& event) override;

		void emit_value_changed();
		void reset();
		void clear();

		godot::Error set_gui_scrollbar(GUI::Scrollbar const* new_gui_scrollbar);
		godot::Error set_gui_scrollbar_name(godot::String const& gui_scene, godot::String const& gui_scrollbar_name);
		godot::String get_gui_scrollbar_name() const;

		void set_value(int32_t new_value);
		void set_scaled_value(fixed_point_t new_scaled_value);
		fixed_point_t get_max_value_scaled() const;
		void increment_value();
		void decrement_value();

		float get_value_as_ratio() const;
		void set_value_as_ratio(float new_ratio);

		fixed_point_t get_value_scaled_fp() const;
		float get_value_scaled() const;

		void set_step_count(const int32_t new_step_count);
		void set_scale(
			const fixed_point_t new_offset,
			const int32_t new_scale_numerator,
			const int32_t new_scale_denominator
		);
		void set_range_limits(
			const std::optional<int32_t> new_lower_range_limit,
			const std::optional<int32_t> new_upper_range_limit
		);
		void set_range_limits_fp(
			const std::optional<fixed_point_t> new_lower_range_limit,
			const std::optional<fixed_point_t> new_upper_range_limit
		);
		void set_range_limits_and_value_from_slider_value(
			ReadOnlyClampedValue& slider_value
		);

		/* Override the main dimension of gui_scollbar's size with the specified length. */
		void set_length_override(real_t new_length_override);
	};
}
