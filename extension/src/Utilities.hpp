#pragma once

#include <godot_cpp/classes/image.hpp>

#include "openvic/Types.hpp"

#define ERR(x) ((x) == SUCCESS ? OK : FAILED)

namespace OpenVic {

	inline std::string godot_to_std_string(godot::String const& str) {
		return str.ascii().get_data();
	}

	inline godot::String std_to_godot_string(std::string const& str) {
		return str.c_str();
	}

	inline godot::Color to_godot_color(colour_t colour) {
		return { colour_byte_to_float((colour >> 16) & 0xFF), colour_byte_to_float((colour >> 8) & 0xFF), colour_byte_to_float(colour & 0xFF) };
	}

	godot::Ref<godot::Image> load_godot_image(godot::String const& path);

	void draw_pie_chart(godot::Ref<godot::Image> image,
		godot::Array const& stopAngles, godot::Array const& colours, float radius,
		godot::Vector2 shadow_displacement, float shadow_tightness, float shadow_radius, float shadow_thickness,
		godot::Color trim_colour, float trim_size, float gradient_falloff, float gradient_base,
		bool donut, bool donut_inner_trim, float donut_inner_radius);
}
