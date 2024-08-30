#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>

#include <openvic-simulation/interface/GUI.hpp>

#include "openvic-extension/classes/GFXSpriteTexture.hpp"
#include "openvic-extension/classes/GUIHasTooltip.hpp"

namespace OpenVic {
	class GUIScrollbar : public godot::Control {
		GDCLASS(GUIScrollbar, godot::Control)

		GUI_TOOLTIP_DEFINITIONS

		GUI::Scrollbar const* PROPERTY(gui_scrollbar);

		godot::Ref<GFXSpriteTexture> slider_texture;
		godot::Ref<GFXSpriteTexture> track_texture;
		godot::Ref<GFXSpriteTexture> less_texture;
		godot::Ref<GFXSpriteTexture> more_texture;

		godot::Rect2 slider_rect;
		real_t slider_start, slider_distance;
		godot::Rect2 track_rect;
		godot::Rect2 less_rect;
		godot::Rect2 more_rect;

		godot::Ref<GFXSpriteTexture> range_limit_min_texture;
		godot::Ref<GFXSpriteTexture> range_limit_max_texture;

		godot::Rect2 range_limit_min_rect;
		godot::Rect2 range_limit_max_rect;

		godot::Orientation PROPERTY(orientation);
		real_t PROPERTY(length_override);

		int32_t PROPERTY(value);
		int32_t PROPERTY(min_value);
		int32_t PROPERTY(max_value);

		bool PROPERTY_CUSTOM_PREFIX(range_limited, is);
		int32_t PROPERTY(range_limit_min);
		int32_t PROPERTY(range_limit_max);

		bool hover_slider, hover_track, hover_less, hover_more;
		bool pressed_slider, pressed_track, pressed_less, pressed_more;

		/* The time between value changes while the less/more button is held down (in seconds). */
		static constexpr double BUTTON_CHANGE_DELAY = 1.0 / 60.0;
		/* The time between value change rate increases while the less/more button is held down (in seconds). */
		static constexpr double BUTTON_CHANGE_ACCELERATE_DELAY = 10.0 * BUTTON_CHANGE_DELAY;

		/* When the less/more button is pressed, button_change_value_base is initialised based on the shift and control keys
		 * and button_change_value is initialised to 0. Every BUTTON_CHANGE_ACCELERATE_DELAY seconds button_change_value
		 * increases by button_change_value_base, and every BUTTON_CHANGE_DELAY seconds value changes by button_change_value.
		 * If button_change_value is still 0 when the button is released, then value will be increased by
		 * button_change_value_base so that short clicks still have an effect.*/
		int32_t button_change_value_base, button_change_value;
		double button_change_accelerate_timer, button_change_timer;

		void _start_button_change(bool shift_pressed, bool control_pressed);
		void _stop_button_change();

		/* Changes value by button_change_value with the direction determined by orientation and pressed_less or pressed_more.
		 * Returns true if a change occured, otherwise false. */
		bool _update_button_change();

		float _value_to_ratio(int32_t val) const;

		void _calculate_rects();

		void _constrain_value();
		godot::Error _constrain_range_limits();
		godot::Error _constrain_limits();

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		static godot::StringName const& signal_value_changed();

		GUIScrollbar();

		godot::Vector2 _get_minimum_size() const override;
		void _gui_input(godot::Ref<godot::InputEvent> const& event) override;

		void emit_value_changed();
		godot::Error reset();
		void clear();

		godot::Error set_gui_scrollbar(GUI::Scrollbar const* new_gui_scrollbar);
		godot::Error set_gui_scrollbar_name(godot::String const& gui_scene, godot::String const& gui_scrollbar_name);
		godot::String get_gui_scrollbar_name() const;

		void set_value(int32_t new_value, bool signal = true);
		void increment_value(bool signal = true);
		void decrement_value(bool signal = true);

		float get_value_as_ratio() const;
		void set_value_as_ratio(float new_ratio, bool signal = true);

		godot::Error set_range_limits(int32_t new_range_limit_min, int32_t new_range_limit_max, bool signal = true);
		godot::Error set_limits(int32_t new_min_value, int32_t new_max_value, bool signal = true);

		/* Override the main dimension of gui_scollbar's size with the specified length. */
		void set_length_override(real_t new_length_override);
	};
}
