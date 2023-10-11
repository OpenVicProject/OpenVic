#include "Utilities.hpp"

#include <numbers>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <gli/convert.hpp>
#include <gli/load_dds.hpp>

using namespace godot;
using namespace OpenVic;

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

	PackedByteArray pixels;
	pixels.resize(texture.size());
	memcpy(pixels.ptrw(), texture.data(), pixels.size());
	UtilityFunctions::print("needs_bgr_to_rgb = ", needs_bgr_to_rgb);
	if (needs_bgr_to_rgb) {
		for (size_t i = 0; i < pixels.size(); i += 4) {
			std::swap(pixels[i], pixels[i+2]);
		}
	}

	const gli::texture2d::extent_type extent { texture.extent() };
	return Image::create_from_data(extent.x, extent.y, false, Image::FORMAT_RGBA8, pixels);
}

Ref<Image> Utilities::load_godot_image(String const& path) {
	if (path.begins_with("res://")) {
		ResourceLoader* loader = ResourceLoader::get_singleton();
		return loader ? loader->load(path) : nullptr;
	} else {
		if (path.ends_with(".dds")) {
			return load_dds_image(path);
		}
		return Image::load_from_file(path);
	}
}

// Get the polar coordinates of a pixel relative to the center
static Vector2 getPolar(Vector2 UVin, Vector2 center) {
	Vector2 relcoord = (UVin - center);
	float dist = relcoord.length();
	float theta = std::numbers::pi / 2 + atan2(relcoord.y, relcoord.x);
	if (theta < 0.0f) theta += std::numbers::pi * 2;
	return { dist, theta };
}

// From thebookofshaders, returns a gradient falloff
static inline float parabola(float base, float x, float k) {
	return powf(base * x * (1.0 - x), k);
}

static inline float parabola_shadow(float base, float x) {
	return base * x * x;
}

static Color pie_chart_fragment(Vector2 UV, float radius, Array const& stopAngles, Array const& colours,
	Vector2 shadow_displacement, float shadow_tightness, float shadow_radius, float shadow_thickness,
	Color trim_colour, float trim_size, float gradient_falloff, float gradient_base,
	bool donut, bool donut_inner_trim, float donut_inner_radius) {

	Vector2 coords = getPolar(UV, { 0.5, 0.5 });
	float dist = coords.x;
	float theta = coords.y;

	Vector2 shadow_polar = getPolar(UV, shadow_displacement);
	float shadow_peak = radius + (radius - donut_inner_radius) / 2.0;
	float shadow_gradient = shadow_thickness + parabola_shadow(shadow_tightness * -10.0, shadow_polar.x + shadow_peak - shadow_radius);

	// Inner hole of the donut => make it transparent
	if (donut && dist <= donut_inner_radius) {
		return { 0.1, 0.1, 0.1, shadow_gradient };
	}
	// Inner trim
	else if (donut && donut_inner_trim && dist <= donut_inner_radius + trim_size) {
		return { trim_colour, 1.0 };
	}
	// Interior
	else if (dist <= radius - trim_size) {
		Color col { 1.0f, 0.0f, 0.0f };
		for (int i = 0; i < stopAngles.size(); i++) {
			if (theta <= float(stopAngles[i])) {
				col = colours[i];
				break;
			}
		}
		float gradient = parabola(gradient_base, dist, gradient_falloff);
		return { col * (1.0 - gradient), 1.0 };
	}
	// Outer trim
	else if (dist <= radius) {
		return { trim_colour, 1.0 };
	}
	// Outside the circle
	else {
		return { 0.1, 0.1, 0.1, shadow_gradient };
	}
}

void Utilities::draw_pie_chart(Ref<Image> image,
	Array const& stopAngles, Array const& colours, float radius,
	Vector2 shadow_displacement, float shadow_tightness, float shadow_radius, float shadow_thickness,
	Color trim_colour, float trim_size, float gradient_falloff, float gradient_base,
	bool donut, bool donut_inner_trim, float donut_inner_radius) {

	ERR_FAIL_NULL_EDMSG(image, "Cannot draw pie chart to null image.");
	const int32_t width = image->get_width();
	const int32_t height = image->get_height();
	ERR_FAIL_COND_EDMSG(width <= 0 || height <= 0, "Cannot draw pie chart to empty image.");
	if (width != height) {
		UtilityFunctions::push_warning("Drawing pie chart to non-square image: ", width, "x", height);
	}
	const int32_t size = std::min(width, height);
	for (int32_t y = 0; y < size; ++y) {
		for (int32_t x = 0; x < size; ++x) {
			image->set_pixel(x, y, pie_chart_fragment(
				{ static_cast<float>(x) / static_cast<float>(size),
				  static_cast<float>(y) / static_cast<float>(size) },
				radius, stopAngles, colours,
				shadow_displacement, shadow_tightness, shadow_radius, shadow_thickness,
				trim_colour, trim_size, gradient_falloff, gradient_base,
				donut, donut_inner_trim, donut_inner_radius));
		}
	}
}
