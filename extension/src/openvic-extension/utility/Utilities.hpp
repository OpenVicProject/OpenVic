#pragma once

#include <concepts>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/image_texture.hpp>

#include <openvic-simulation/types/Colour.hpp>
#include <openvic-simulation/types/Date.hpp>
#include <openvic-simulation/types/Vector.hpp>

#define ERR(x) ((x) ? OK : FAILED)

namespace OpenVic::Utilities {
	_FORCE_INLINE_ std::string godot_to_std_string(godot::String const& str) {
		return str.utf8().get_data();
	}

	_FORCE_INLINE_ godot::String std_to_godot_string(std::string_view const& str) {
		return godot::String::utf8(str.data(), str.length());
	}

	_FORCE_INLINE_ godot::String read_riff_str(godot::Ref<godot::FileAccess> const& file, int64_t size = 4) {
		return file->get_buffer(size).get_string_from_ascii();
	}

	godot::StringName const& get_short_value_placeholder();
	godot::StringName const& get_long_value_placeholder();
	godot::StringName const& get_percentage_value_placeholder();
	godot::StringName const& get_colour_and_sign(const fixed_point_t value);

	godot::String int_to_string_suffixed(int64_t val);

	godot::String int_to_string_commas(int64_t val);

	godot::String float_to_string_suffixed(float val);

	godot::String float_to_string_dp(float val, int32_t decimal_places);

	godot::String fixed_point_to_string_dp(fixed_point_t val, int32_t decimal_places);

	godot::String percentage_to_string_dp(fixed_point_t val, int32_t decimal_places);

	// 3dp if abs(val) < 2 else 2dp if abs(val) < 10 else 1dp
	godot::String float_to_string_dp_dynamic(float val);
	godot::String cash_to_string_dp_dynamic(fixed_point_t val);
	godot::String format_with_currency(godot::String const& text);

	constexpr real_t to_real_t(std::floating_point auto val) {
		return static_cast<real_t>(val);
	}

	godot::String date_to_string(Date date);

	godot::String date_to_formatted_string(Date date, bool month_first);

	_FORCE_INLINE_ godot::Color to_godot_color(IsColour auto colour) {
		return { colour.redf(), colour.greenf(), colour.bluef(), colour.alphaf() };
	}

	_FORCE_INLINE_ godot::Vector2i to_godot_ivec2(ivec2_t const& vec) {
		return { vec.x, vec.y };
	}

	_FORCE_INLINE_ godot::Vector2 to_godot_fvec2(fvec2_t const& vec) {
		return { vec.x, vec.y };
	}

	_FORCE_INLINE_ ivec2_t from_godot_ivec2(godot::Vector2i const& vec) {
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

	godot::Ref<godot::Image> make_solid_colour_image(
		godot::Color const& colour, int32_t width, int32_t height,
		godot::Image::Format format = godot::Image::Format::FORMAT_RGBA8
	);

	godot::Ref<godot::ImageTexture> make_solid_colour_texture(
		godot::Color const& colour, int32_t width, int32_t height,
		godot::Image::Format format = godot::Image::Format::FORMAT_RGBA8
	);

	namespace literals {
		constexpr real_t operator""_real(long double val) { return to_real_t(val); }
	}
}
