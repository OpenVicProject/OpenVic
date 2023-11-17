#pragma once

#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/image.hpp>

#include <openvic-simulation/types/Colour.hpp>
#include <openvic-simulation/types/Vector.hpp>

#define ERR(x) ((x) ? OK : FAILED)

namespace OpenVic::Utilities {

	inline std::string godot_to_std_string(godot::String const& str) {
		return str.ascii().get_data();
	}

	inline godot::String std_to_godot_string(std::string const& str) {
		return str.c_str();
	}

	inline godot::String std_view_to_godot_string(std::string_view str) {
		return std_to_godot_string(static_cast<std::string>(str));
	}

	inline godot::StringName std_to_godot_string_name(std::string const& str) {
		return str.c_str();
	}

	inline godot::StringName std_view_to_godot_string_name(std::string_view str) {
		return std_to_godot_string_name(static_cast<std::string>(str));
	}

	inline godot::Color to_godot_color(colour_t colour) {
		return {
			colour_byte_to_float((colour >> 16) & 0xFF),
			colour_byte_to_float((colour >> 8) & 0xFF),
			colour_byte_to_float(colour & 0xFF)
		};
	}

	inline godot::Vector2i to_godot_ivec2(ivec2_t vec) {
		return { vec.x, vec.y };
	}

	inline godot::Vector2 to_godot_fvec2(fvec2_t vec) {
		return { vec.x, vec.y };
	}

	godot::Ref<godot::Image> load_godot_image(godot::String const& path);

	godot::Ref<godot::FontFile> load_godot_font(godot::String const& fnt_path, godot::Ref<godot::Image> const& image);

	void draw_pie_chart(
		godot::Ref<godot::Image> image, godot::Array const& stopAngles, godot::Array const& colours, float radius,
		godot::Vector2 shadow_displacement, float shadow_tightness, float shadow_radius, float shadow_thickness,
		godot::Color trim_colour, float trim_size, float gradient_falloff, float gradient_base, bool donut,
		bool donut_inner_trim, float donut_inner_radius
	);
}
