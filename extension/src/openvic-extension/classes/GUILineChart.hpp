#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/line2d.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>

namespace OpenVic {
	class GUILineChart : public godot::Control {
		GDCLASS(GUILineChart, godot::Control);

		GFX::LineChart const* gfx_line_chart = nullptr;

		int32_t point_count = 0;
		float PROPERTY(min_value, 0.0f);
		float PROPERTY(max_value, 0.0f);

	protected:
		static void _bind_methods();

	public:
		GUILineChart();

		// Clears the GFX::LineChart and all lines, resetting the node to its initial state.
		void clear();

		void clear_lines();

		godot::Error set_gfx_line_chart(GFX::LineChart const* new_gfx_line_chart);

		godot::Error set_gfx_line_chart_name(godot::String const& new_gfx_line_chart_name);

		godot::String get_gfx_line_chart_name() const;

		// Set central_value to adjust the value represented by the vertical midpoint of the graph.
		// Set min_value_range to ensure the min/max values of the graph are at least central_value +/- min_value_range.
		godot::Error set_gradient_line(
			godot::PackedFloat32Array const& line_values, float central_value = 0.0f, float min_value_range = 0.0f
		);
		godot::Error add_coloured_line(godot::PackedFloat32Array const& line_values, godot::Color const& line_colour);
		void scale_coloured_lines();
	};
}
