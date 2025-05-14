#include "GUILineChart.hpp"

#include <godot_cpp/classes/gradient.hpp>

#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace OpenVic::Utilities::literals;
using namespace godot;

void GUILineChart::_bind_methods() {
	OV_BIND_METHOD(GUILineChart::clear);
	OV_BIND_METHOD(GUILineChart::clear_lines);

	OV_BIND_METHOD(GUILineChart::get_min_value);
	OV_BIND_METHOD(GUILineChart::get_max_value);

	OV_BIND_METHOD(GUILineChart::set_gfx_line_chart_name, { "new_gfx_line_chart_name" });
	OV_BIND_METHOD(GUILineChart::get_gfx_line_chart_name);

	OV_BIND_METHOD(
		GUILineChart::set_gradient_line, { "line_values", "central_value", "min_value_range" }, DEFVAL(0.0f), DEFVAL(0.0f)
	);
	OV_BIND_METHOD(GUILineChart::add_coloured_line, { "line_values", "line_colour" });
	OV_BIND_METHOD(GUILineChart::scale_coloured_lines);
}

GUILineChart::GUILineChart() {
	set_clip_contents(true);
	set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
}

void GUILineChart::clear() {
	gfx_line_chart = nullptr;
	clear_lines();
}

void GUILineChart::clear_lines() {
	int32_t child_index = get_child_count();

	while (child_index > 0) {
		Node* child = get_child(--child_index);
		remove_child(child);
		child->queue_free();
	}

	point_count = 0;
	min_value = 0.0f;
	max_value = 0.0f;
}

Error GUILineChart::set_gfx_line_chart(GFX::LineChart const* new_gfx_line_chart) {
	if (gfx_line_chart == new_gfx_line_chart) {
		return OK;
	}

	if (new_gfx_line_chart == nullptr) {
		clear();
		return OK;
	}

	gfx_line_chart = new_gfx_line_chart;

	set_custom_minimum_size(Utilities::to_godot_ivec2(gfx_line_chart->get_size()));

	return OK;
}

Error GUILineChart::set_gfx_line_chart_name(String const& new_gfx_line_chart_name) {
	if (new_gfx_line_chart_name.is_empty()) {
		return set_gfx_line_chart(nullptr);
	}

	GFX::Sprite const* sprite = UITools::get_gfx_sprite(new_gfx_line_chart_name);
	ERR_FAIL_NULL_V(sprite, FAILED);

	GFX::LineChart const* new_gfx_line_chart = sprite->cast_to<GFX::LineChart>();
	ERR_FAIL_NULL_V_MSG(
		new_gfx_line_chart, FAILED,
		vformat(
			"Invalid type for GFX sprite %s: %s (expected %s)", new_gfx_line_chart_name,
			Utilities::std_to_godot_string(sprite->get_type()),
			Utilities::std_to_godot_string(GFX::LineChart::get_type_static())
		)
	);

	return set_gfx_line_chart(new_gfx_line_chart);
}

String GUILineChart::get_gfx_line_chart_name() const {
	return gfx_line_chart != nullptr ? Utilities::std_to_godot_string(gfx_line_chart->get_name()) : String {};
}

Error GUILineChart::set_gradient_line(PackedFloat32Array const& line_values, float central_value, float min_value_range) {
	ERR_FAIL_COND_V(line_values.size() < 2, FAILED);
	ERR_FAIL_COND_V(min_value_range < 0.0f, FAILED);

	clear_lines();

	Line2D* line = memnew(Line2D);
	ERR_FAIL_NULL_V(line, FAILED);

	if (gfx_line_chart != nullptr) {
		line->set_width(static_cast<float>(gfx_line_chart->get_linewidth()));
	}

	Ref<Gradient> gradient;
	gradient.instantiate();
	ERR_FAIL_NULL_V(gradient, FAILED);

	// Remove hardcoded (1, WHITE) point
	gradient->remove_point(1);

	// Calculate the height of the graph in value-space
	for (int64_t index = 0; index < line_values.size(); ++index) {
		const float abs_value = std::abs(line_values[index] - central_value);

		if (min_value_range < abs_value) {
			min_value_range = abs_value;
		}
	}

	min_value = central_value - min_value_range;
	max_value = central_value + min_value_range;

	if (min_value_range == 0.0f) {
		min_value_range = 1.0f;
	} else {
		min_value_range *= 2.0f;
	}

	const Vector2 size = get_size();

	// Centre the graph vertically
	line->set_position({ 0.0_real, size.height / 2.0_real });

	// Negative so height scale flips values (y increases as you go down with Godot's 2D coordinate system)
	const Vector2 scale { size.width / (line_values.size() - 1), size.height / -min_value_range };

	Vector2 last_point;
	real_t distance = 0.0_real;

	for (int64_t index = 0; index < line_values.size(); ++index) {
		const float value = line_values[index] - central_value;

		const Vector2 point { index * scale.x, value * scale.y };

		line->add_point(point);

		if (index > 0) {
			distance += last_point.distance_to(point);
		}

		last_point = point;

		static const Color MAX_COLOUR { 0.0f, 1.0f, 0.0f };
		static const Color MIN_COLOUR { 1.0f, 0.0f, 0.0f };

		gradient->add_point(distance, MIN_COLOUR.lerp(MAX_COLOUR, value / min_value_range + 0.5f));
	}

	// Remove hardcoded (0, BLACK) point (couldn't remove earlier as the gradient can't be empty)
	gradient->remove_point(0);

	// Rescale the gradient points to be in the range [0, 1]
	for (int32_t index = 0; index < gradient->get_point_count(); ++index) {
		gradient->set_offset(index, gradient->get_offset(index) / distance);
	}

	line->set_gradient(gradient);

	add_child(line);

	return OK;
}

Error GUILineChart::add_coloured_line(PackedFloat32Array const& line_values, Color const& line_colour) {
	ERR_FAIL_COND_V(line_values.size() < 2, FAILED);

	if (point_count <= 0) {
		point_count = line_values.size();
	} else {
		ERR_FAIL_COND_V_MSG(
			point_count != line_values.size(), FAILED,
			vformat(
				"Mismatch between number of points in GUILineChart lines: new line %d != existing lines %d", line_values.size(),
				point_count
			)
		);
	}

	Line2D* line = memnew(Line2D);
	ERR_FAIL_NULL_V(line, FAILED);

	if (gfx_line_chart != nullptr) {
		line->set_width(static_cast<float>(gfx_line_chart->get_linewidth()));
	}

	line->set_default_color(line_colour);

	const Vector2 size = get_size();

	const float scale_x = size.width / (line_values.size() - 1);

	for (int64_t index = 0; index < line_values.size(); ++index) {
		float value = line_values[index];

		// Negative so height scale flips values (y increases as you go down with Godot's 2D coordinate system)
		line->add_point({ index * scale_x, -value });

		if (value < min_value) {
			min_value = value;
		} else if (value > max_value) {
			max_value = value;
		}
	}

	add_child(line);

	return OK;
}

void GUILineChart::scale_coloured_lines() {
	float translate_y, scale_y;
	if (min_value != max_value) {
		translate_y = max_value;
		scale_y = get_size().height / (max_value - min_value);
	} else {
		// No scaling needed if values are all 0 (min/max start at 0 and move away from it in opposite directions)
		translate_y = get_size().height;
		scale_y = 1.0f;
	}

	const int32_t child_count = get_child_count();

	for (int32_t line_index = 0; line_index < child_count; ++line_index) {
		Line2D* line = Object::cast_to<Line2D>(get_child(line_index));
		ERR_CONTINUE(line == nullptr);

		for (int32_t point_index = 0; point_index < line->get_point_count(); ++point_index) {
			Vector2 point = line->get_point_position(point_index);

			point.y += translate_y;
			point.y *= scale_y;

			line->set_point_position(point_index, point);
		}
	}
}
