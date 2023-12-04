#include "Utilities.hpp"

#include <numbers>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <gli/convert.hpp>
#include <gli/load_dds.hpp>

using namespace godot;
using namespace OpenVic;

/* Date formatted like this: "January 1, 1836" (with the month localised, if possible). */
String Utilities::date_to_formatted_string(Date date) {
	std::string const& month_name = date.get_month_name();
	const String day_and_year = " " + String::num_int64(date.get_day()) + ", " + String::num_int64(date.get_year());
	TranslationServer const* server = TranslationServer::get_singleton();
	if (server != nullptr) {
		return server->translate(std_to_godot_string_name(month_name)) + day_and_year;
	} else {
		return std_to_godot_string(month_name) + day_and_year;
	}
}

Ref<Resource> Utilities::load_resource(String const& path, String const& type_hint) {
	ResourceLoader* loader = ResourceLoader::get_singleton();
	ERR_FAIL_NULL_V(loader, nullptr);
	return loader->load(path, type_hint);
}

static Ref<Image> load_dds_image(String const& path) {
	gli::texture2d texture { gli::load_dds(Utilities::godot_to_std_string(path)) };
	if (texture.empty()) {
		UtilityFunctions::push_error("Failed to load DDS file: ", path);
		return nullptr;
	}

	static constexpr gli::format expected_format = gli::FORMAT_BGRA8_UNORM_PACK8;
	const bool needs_bgr_to_rgb = texture.format() == expected_format;
	if (!needs_bgr_to_rgb) {
		texture = gli::convert(texture, expected_format);
		if (texture.empty()) {
			UtilityFunctions::push_error("Failed to convert DDS file: ", path);
			return nullptr;
		}
	}

	const gli::texture2d::extent_type extent { texture.extent() };
	const int width = extent.x, height = extent.y, size = width * height * 4;

	/* Only fail if there aren't enough bytes, everything seems to work fine if there are extra bytes and we ignore them */
	if (size > texture.size()) {
		UtilityFunctions::push_error(
			"Texture size ", static_cast<int64_t>(texture.size()), " mismatched with dims-based size ", size, " for ", path
		);
		return nullptr;
	}

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
	Ref<FontFile> font;
	font.instantiate();
	const Error err = font->load_bitmap_font(fnt_path);
	font->set_texture_image(0, { font->get_fixed_size(), 0 }, 0, image);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to load font (error ", err, "): ", fnt_path);
	}
	return font;
}
