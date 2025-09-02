#include "Utilities.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <gli/convert.hpp>
#include <gli/load_dds.hpp>

using namespace godot;
using namespace OpenVic;

godot::StringName const& Utilities::get_short_value_placeholder() {
	static const godot::StringName value_placeholder = "$VAL$";
	return value_placeholder;
}
godot::StringName const& Utilities::get_long_value_placeholder() {
	static const godot::StringName value_placeholder = "$VALUE$";
	return value_placeholder;
}
godot::StringName const& Utilities::get_percentage_value_placeholder() {
	static const godot::StringName value_placeholder = "$PERC$";
	return value_placeholder;
}
godot::StringName const& Utilities::get_colour_and_sign(const fixed_point_t value) {
	static const godot::StringName green_plus = "G+";
	static const godot::StringName red = "R";
	static const godot::StringName yellow = "Y";

	return value > 0
		? green_plus
		: value < 0
			? red
			: yellow;
}

/* Int to 2 decimal place string in terms of the largest suffix less than or equal to it,
 * or normal integer string if less than the smallest suffix. */
String Utilities::int_to_string_suffixed(int64_t val) {
	static const std::vector<std::pair<int64_t, String>> suffixes {
		{ 1'000'000'000'000, "T" },
		{ 1'000'000'000, "B" },
		{ 1'000'000, "M" },
		{ 1'000, "k" }
	};
	static constexpr int64_t decimal_places_multiplier = 100;
	const bool negative = val < 0;
	if (negative) {
		val = -val;
	}
	for (auto const& [suffix_val, suffix_str] : suffixes) {
		if (val >= suffix_val) {
			const int64_t whole = val / suffix_val;
			const int64_t frac = (val * decimal_places_multiplier / suffix_val) % decimal_places_multiplier;
			return (negative ? "-" : "") + String::num_int64(whole) + "." +
				(frac < 10 ? "0" : "") + String::num_int64(frac) + suffix_str;
		}
	}
	return (negative ? "-" : "") + String::num_int64(val);
}

String Utilities::int_to_string_commas(int64_t val) {
	const bool negative = val < 0;
	if (negative) {
		val = -val;
	}

	const String string_val = String::num_int64(val);

	String result;
	int64_t length_remaining = string_val.length();

	static constexpr int64_t digits_per_comma = 3;
	static const String comma = ",";

	while (length_remaining > digits_per_comma) {
		result = comma + string_val.substr(length_remaining -= digits_per_comma, digits_per_comma) + result;
	}

	result = string_val.substr(0, length_remaining) + result;

	if (negative) {
		result = "-" + result;
	}

	return result;
}

String Utilities::float_to_string_suffixed(float val) {
	const float abs_val = std::abs(val);

	if (abs_val < 10'000.0f) {
		return float_to_string_dp(val, 1);
	}

	if (abs_val < 1'000'000.0f) {
		return float_to_string_dp(val / 1'000.0f, 2) + "k";
	}

	if (abs_val < 1'000'000'000.0f) {
		return float_to_string_dp(val / 1'000'000.0f, 2) + "M";
	}

	if (abs_val < 1'000'000'000'000.0f) {
		return float_to_string_dp(val / 1'000'000'000.0f, 2) + "B";
	}

	return float_to_string_dp(val / 1'000'000'000'000.0f, 2) + "T";
}

/* Float to string formatted with the specified number of decimal places. */
String Utilities::float_to_string_dp(float val, int32_t decimal_places) {
	String result = String::num(val, decimal_places);
	if (decimal_places >= 0) {
		return result.pad_decimals(decimal_places);
	} else {
		return result;
	}
}

String Utilities::fixed_point_to_string_dp(fixed_point_t val, int32_t decimal_places) {
	// We could use fixed point's own to_string method, but that allocates an intermediate string so better to go via float
	// return Utilities::std_to_godot_string(val.to_string(decimal_places));
	return Utilities::float_to_string_dp(val.to_float(), decimal_places);
}

String Utilities::percentage_to_string_dp(fixed_point_t val, int32_t decimal_places) {
	return Utilities::format(
		"%s%%",
		Utilities::float_to_string_dp(
			(100 * val).to_float(),
			decimal_places
		)
	);
}

String Utilities::float_to_string_dp_dynamic(float val) {
	const float abs_val = std::abs(val);
	return float_to_string_dp(val, abs_val < 2.0f ? 3 : abs_val < 10.0f ? 2 : 1);
}

String Utilities::cash_to_string_dp_dynamic(fixed_point_t val) {
	return format_with_currency(
		Utilities::float_to_string_dp_dynamic(
			val.to_float()
		)
	);
}

String Utilities::format_with_currency(godot::String const& text) {
	static const String currency_format = String::utf8("%s¤");
	return Utilities::format(currency_format, text);
}

String Utilities::date_to_string(Date date) {
	static const String date_template_string = String { "%d" } + Date::SEPARATOR_CHARACTER + "%d" +
		Date::SEPARATOR_CHARACTER + "%d";

	return Utilities::format(date_template_string, date.get_year(), date.get_month(), date.get_day());
}

/* Date formatted like one of these, with the month localised if possible:
 *  - "1 January, 1836" (if month_first is false)
 *  - "January 1, 1836" (if month_first is true) */
String Utilities::date_to_formatted_string(Date date, bool month_first) {
	String day = String::num_int64(date.get_day());
	String month = Utilities::std_to_godot_string(date.get_month_name());
	TranslationServer const* server = TranslationServer::get_singleton();
	if (server != nullptr) {
		month = server->translate(month);
	}
	String year = String::num_int64(date.get_year());

	if (month_first) {
		return month + " " + day + ", " + year;
	} else {
		return day + " " + month + ", " + year;
	}
}

Ref<Resource> Utilities::load_resource(String const& path, String const& type_hint) {
	ResourceLoader* loader = ResourceLoader::get_singleton();
	ERR_FAIL_NULL_V(loader, nullptr);
	return loader->load(path, type_hint);
}

static Ref<Image> load_dds_image(String const& path) {
	const gli::texture dds_file = gli::load_dds(Utilities::godot_to_std_string(path));
	ERR_FAIL_COND_V_MSG(dds_file.empty(), nullptr, Utilities::format("Failed to load DDS file: %s", path));
	gli::texture2d texture { dds_file };
	ERR_FAIL_COND_V_MSG(texture.empty(), nullptr, Utilities::format("Failed to create DDS texture: %s", path));

	static constexpr gli::format expected_format = gli::FORMAT_BGRA8_UNORM_PACK8;
	const bool needs_bgr_to_rgb = texture.format() == expected_format;
	if (!needs_bgr_to_rgb) {
		texture = gli::convert(texture, expected_format);
		ERR_FAIL_COND_V_MSG(texture.empty(), nullptr, Utilities::format("Failed to convert DDS file: %s", path));
	}

	const gli::texture2d::extent_type extent { texture.extent() };
	const int32_t width = extent.x, height = extent.y, size = width * height * 4;

	/* Only fail if there aren't enough bytes, everything seems to work fine if there are extra bytes and we ignore them */
	ERR_FAIL_COND_V_MSG(
		size > texture.size(), nullptr,
		Utilities::format("Texture size %d mismatched with dims-based size %d for %s", static_cast<int64_t>(texture.size()), size, path)
	);

	PackedByteArray pixels;
	ERR_FAIL_COND_V(pixels.resize(size) != OK, nullptr);

	/* Index offset used to control whether we are reading */
	const size_t rb_idx = 2 * needs_bgr_to_rgb;
	uint8_t const* ptr = static_cast<uint8_t const*>(texture.data());
	for (size_t i = 0; i < size; i += 4) {
		pixels[i + 0] = ptr[i + rb_idx];
		pixels[i + 1] = ptr[i + 1];
		pixels[i + 2] = ptr[i + 2 - rb_idx];
		pixels[i + 3] = ptr[i + 3];
	}

	return Image::create_from_data(width, height, false, Image::FORMAT_RGBA8, pixels);
}

Ref<Image> Utilities::load_godot_image(String const& path) {
	if (path.ends_with(".dds")) {
		return load_dds_image(path);
	}
	return Image::load_from_file(path);
}

Ref<FontFile> Utilities::load_godot_font(String const& fnt_path, Ref<Image> const& image) {
	ERR_FAIL_NULL_V(image, nullptr);
	Ref<FontFile> font;
	font.instantiate();
	ERR_FAIL_COND_V_MSG(font->load_bitmap_font(fnt_path) != OK, nullptr, Utilities::format("Failed to load font: %s", fnt_path));
	font->set_texture_image(0, { font->get_fixed_size(), 0 }, 0, image);
	return font;
}

Ref<Image> Utilities::make_solid_colour_image(Color const& colour, int32_t width, int32_t height, Image::Format format) {
	const Ref<Image> result = Image::create(width, height, false, format);
	ERR_FAIL_NULL_V(result, nullptr);
	result->fill(colour);
	return result;
}

Ref<ImageTexture> Utilities::make_solid_colour_texture(Color const& colour, int32_t width, int32_t height, Image::Format format) {
	const Ref<Image> image = make_solid_colour_image(colour, width, height, format);
	ERR_FAIL_NULL_V(image, nullptr);
	const Ref<ImageTexture> result = ImageTexture::create_from_image(image);
	ERR_FAIL_NULL_V(result, nullptr);
	return result;
}

Variant Utilities::get_project_setting(godot::StringName const& p_path, godot::Variant const& p_default_value) {
	if (!ProjectSettings::get_singleton()->has_setting(p_path)) {
		ProjectSettings::get_singleton()->set(p_path, p_default_value);
	}
	Variant result = ProjectSettings::get_singleton()->get_setting_with_override(p_path);

	ProjectSettings::get_singleton()->set_initial_value(p_path, p_default_value);
	return result;
}

namespace OpenVic::Utilities {
	thread_local memory::vector<godot::Array> _formatting_array_pool_1;
	thread_local memory::vector<godot::Array> _formatting_array_pool_2;
	thread_local memory::vector<godot::Array> _formatting_array_pool_3;
	thread_local memory::vector<godot::Array> _formatting_array_pool_4;
}