#include "Utilities.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <gli/convert.hpp>
#include <gli/load_dds.hpp>

using namespace godot;
using namespace OpenVic;

/* Int to 2 decimal place string in terms of the largest suffix less than or equal to it,
 * or normal integer string if less than the smallest suffix. */
String Utilities::int_to_formatted_string(int64_t val) {
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

/* Float to formatted to 4 decimal place string. */
String Utilities::float_to_formatted_string(float val) {
	static constexpr int64_t decimal_places = 4;
	return String::num(val, decimal_places).pad_decimals(decimal_places);
}

/* Date formatted like this: "January 1, 1836" (with the month localised, if possible). */
String Utilities::date_to_formatted_string(Date date) {
	const String month_name = std_view_to_godot_string_name(date.get_month_name());
	const String day_and_year = " " + String::num_int64(date.get_day()) + ", " + String::num_int64(date.get_year());
	TranslationServer const* server = TranslationServer::get_singleton();
	if (server != nullptr) {
		return server->translate(month_name) + day_and_year;
	} else {
		return month_name + day_and_year;
	}
}

Ref<Resource> Utilities::load_resource(String const& path, String const& type_hint) {
	ResourceLoader* loader = ResourceLoader::get_singleton();
	ERR_FAIL_NULL_V(loader, nullptr);
	return loader->load(path, type_hint);
}

static Ref<Image> load_dds_image(String const& path) {
	gli::texture2d texture { gli::load_dds(Utilities::godot_to_std_string(path)) };
	ERR_FAIL_COND_V_MSG(texture.empty(), nullptr, vformat("Failed to load DDS file: %s", path));

	static constexpr gli::format expected_format = gli::FORMAT_BGRA8_UNORM_PACK8;
	const bool needs_bgr_to_rgb = texture.format() == expected_format;
	if (!needs_bgr_to_rgb) {
		texture = gli::convert(texture, expected_format);
		ERR_FAIL_COND_V_MSG(texture.empty(), nullptr, vformat("Failed to convert DDS file: %s", path));
	}

	const gli::texture2d::extent_type extent { texture.extent() };
	const int32_t width = extent.x, height = extent.y, size = width * height * 4;

	/* Only fail if there aren't enough bytes, everything seems to work fine if there are extra bytes and we ignore them */
	ERR_FAIL_COND_V_MSG(
		size > texture.size(), nullptr,
		vformat("Texture size %d mismatched with dims-based size %d for %s", static_cast<int64_t>(texture.size()), size, path)
	);

	PackedByteArray pixels;
	pixels.resize(size);
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
	ERR_FAIL_COND_V_MSG(font->load_bitmap_font(fnt_path) != OK, nullptr, vformat("Failed to load font: %s", fnt_path));
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
