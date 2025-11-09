#pragma once

#include <concepts>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <openvic-simulation/types/Colour.hpp>
#include <openvic-simulation/types/Date.hpp>
#include <openvic-simulation/types/Vector.hpp>

#define ERR(x) ((x) ? OK : FAILED)

namespace godot {
	struct Object;
}

namespace OpenVic {
	struct CountryInstance;
	struct ModifierEffect;
	struct State;
}

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

	godot::String get_short_value_placeholder();
	godot::String get_long_value_placeholder();
	godot::String get_percentage_value_placeholder();
	godot::String get_colour_and_sign(const fixed_point_t value);

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
		return { static_cast<real_t>(vec.x), static_cast<real_t>(vec.y) };
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

	godot::Variant get_project_setting(godot::StringName const& p_path, godot::Variant const& p_default_value);

	godot::String get_state_name(godot::Object const& translation_object, State const& state);
	godot::String get_country_name(godot::Object const& translation_object, CountryInstance const& country);
	godot::String get_country_adjective(godot::Object const& translation_object, CountryInstance const& country);

	godot::String make_modifier_effect_value(
		godot::Object const& translation_object,
		ModifierEffect const& format_effect,
		fixed_point_t value,
		bool plus_for_non_negative
	);
	godot::String make_modifier_effect_value_coloured(
		godot::Object const& translation_object,
		ModifierEffect const& format_effect,
		fixed_point_t value,
		bool plus_for_non_negative
	);

#define OPTIMISED_FORMAT_CONCAT(a, b) a##b
#define OPTIMISED_FORMAT_EXPAND(a, b) OPTIMISED_FORMAT_CONCAT(a, b)

#define FOR_LOOP_0()
#define FOR_LOOP_1(BODY) BODY(0)
#define FOR_LOOP_2(BODY) FOR_LOOP_1(BODY) BODY(1)
#define FOR_LOOP_3(BODY) FOR_LOOP_2(BODY) BODY(2)
#define FOR_LOOP_4(BODY) FOR_LOOP_3(BODY) BODY(3)

#define OPTIMISED_FORMAT_COPY(n) args_array.set(n, p_arg##n);
#define OPTIMISED_FORMAT_ARRAY_INIT(count) OPTIMISED_FORMAT_EXPAND(FOR_LOOP_, count)(OPTIMISED_FORMAT_COPY)

#define OPTIMISED_FORMAT_SET_NULL(n) args_array.set(n, godot::Variant{});
#define OPTIMISED_FORMAT_ARRAY_CLEAR(count) OPTIMISED_FORMAT_EXPAND(FOR_LOOP_, count)(OPTIMISED_FORMAT_SET_NULL)

#define OPTIMISED_FORMAT_ARG(n) , const auto p_arg##n
#define OPTIMISED_FORMAT_GET_ARGS(count) OPTIMISED_FORMAT_EXPAND(FOR_LOOP_, count)(OPTIMISED_FORMAT_ARG)

#define OPTIMISED_FORMAT(count) \
	[[nodiscard]] godot::String format( \
		godot::String const& text_template \
		OPTIMISED_FORMAT_GET_ARGS(count) \
	) { \
		extern thread_local memory::vector<godot::Array> _formatting_array_pool_##count; \
		memory::vector<godot::Array>& array_pool = _formatting_array_pool_##count; \
		const bool was_empty = array_pool.empty(); \
		godot::Array args_array = was_empty \
			? godot::Array{} \
			: std::move(array_pool.back()); \
		if (was_empty) { \
			args_array.resize(count); \
		} else { \
			array_pool.pop_back(); \
		} \
		OPTIMISED_FORMAT_ARRAY_INIT(count) \
		const godot::String result = text_template % args_array; \
		OPTIMISED_FORMAT_ARRAY_CLEAR(count) \
		array_pool.push_back(std::move(args_array)); \
		return result; \
	}

	OPTIMISED_FORMAT(1)
	OPTIMISED_FORMAT(2)
	OPTIMISED_FORMAT(3)
	OPTIMISED_FORMAT(4)

#undef OPTIMISED_FORMAT_CONCAT
#undef OPTIMISED_FORMAT_EXPAND

#undef FOR_LOOP_0
#undef FOR_LOOP_1
#undef FOR_LOOP_2
#undef FOR_LOOP_3
#undef FOR_LOOP_4

#undef OPTIMISED_FORMAT_COPY
#undef OPTIMISED_FORMAT_ARRAY_INIT

#undef OPTIMISED_FORMAT_SET_NULL
#undef OPTIMISED_FORMAT_ARRAY_CLEAR

#undef OPTIMISED_FORMAT_ARG
#undef OPTIMISED_FORMAT_GET_ARGS

	template <typename... VarArgs>
	[[nodiscard]] godot::String format(godot::String const& text_template, const VarArgs... p_args) {
		return godot::vformat(text_template, p_args...);
	}

	namespace literals {
		constexpr real_t operator""_real(long double val) { return to_real_t(val); }
	}
}

template<>
struct fmt::formatter<godot::String> : formatter<string_view> {
	format_context::iterator format(godot::String const& str, format_context& ctx) const;
};

template<>
struct fmt::formatter<godot::StringName> : formatter<godot::String> {
	format_context::iterator format(godot::StringName const& str, format_context& ctx) const;
};