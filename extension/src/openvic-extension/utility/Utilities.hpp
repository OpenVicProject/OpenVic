#pragma once

#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/image.hpp>

#include <openvic-simulation/types/Colour.hpp>
#include <openvic-simulation/types/Date.hpp>
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

	godot::String int_to_formatted_string(int64_t val);

	godot::String float_to_formatted_string(float val);

	godot::String date_to_formatted_string(Date date);

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

	/* Loads a Resource from a file in the Godot project directory given by a path beginning with "res://". */
	godot::Ref<godot::Resource> load_resource(godot::String const& path, godot::String const& type_hint = {});

	/* Load an Image from anywhere on the machine, using Godot's image-loading function or, in the case of
	 * ".dds" image files which Godot is unable to load at runtime, GLI's DDS loading function. */
	godot::Ref<godot::Image> load_godot_image(godot::String const& path);

	/* Load a Font from anywhere on the machine, combining the ".fnt" file loaded from the given path with the
	 * already-loaded image file containing the actual characters. */
	godot::Ref<godot::FontFile> load_godot_font(godot::String const& fnt_path, godot::Ref<godot::Image> const& image);
}
