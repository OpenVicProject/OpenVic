#include "GFXPieChartTexture.hpp"

#include <numbers>

#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/utility/UITools.hpp"

using namespace godot;
using namespace OpenVic;
using namespace OpenVic::Utilities::literals;

StringName const& GFXPieChartTexture::_slice_identifier_key() {
	return OV_SNAME(identifier);
}
StringName const& GFXPieChartTexture::_slice_tooltip_key() {
	return OV_SNAME(tooltip);
}
StringName const& GFXPieChartTexture::_slice_colour_key() {
	return OV_SNAME(colour);
}
StringName const& GFXPieChartTexture::_slice_weight_key() {
	return OV_SNAME(weight);
}

GFXPieChartTexture::slice_t const* GFXPieChartTexture::get_slice(Vector2 const& position) const {
	if (slices.empty() || position.length_squared() > 1.0_real) {
		return nullptr;
	}

	static constexpr float TWO_PI = 2.0f * std::numbers::pi_v<float>;

	/* Calculate the anti-clockwise angle between the point and the centre of the image.
	 * The y coordinate is negated as the image coordinate system's y increases downwards. */
	float theta = atan2(-position.y, position.x);
	if (theta < 0.0f) {
		theta += TWO_PI;
	}

	/* Rescale angle so that total_weight is a full rotation. */
	theta *= total_weight / TWO_PI;

	/* Find the slice theta lies in. */
	for (slice_t const& slice : slices) {
		theta -= slice.weight;

		if (theta <= 0.0f) {
			return &slice;
		}
	}

	/* Default to the first slice in case theta never reaches 0 due to floating point inaccuracy. */
	return &slices.front();
}

Error GFXPieChartTexture::_generate_pie_chart_image() {
	ERR_FAIL_NULL_V(gfx_pie_chart, FAILED);
	ERR_FAIL_COND_V_MSG(
		gfx_pie_chart->get_size() <= 0, FAILED,
		Utilities::format("Invalid GFX::PieChart size for GFXPieChartTexture - %d", gfx_pie_chart->get_size())
	);

	const int32_t pie_chart_radius = gfx_pie_chart->get_size();
	const int32_t pie_chart_diameter = 2 * pie_chart_radius;

	/* Whether we've already set the ImageTexture to an image of the right dimensions,
	 * and so can update it without creating and setting a new image, or not. */
	const bool can_update = pie_chart_image.is_valid() && pie_chart_image->get_width() == pie_chart_diameter
		&& pie_chart_image->get_height() == pie_chart_diameter;

	if (!can_update) {
		pie_chart_image = Image::create(pie_chart_diameter, pie_chart_diameter, false, Image::FORMAT_RGBA8);
		ERR_FAIL_NULL_V(pie_chart_image, FAILED);
	}

	static const Color background_colour { 0.0f, 0.0f, 0.0f, 0.0f };

	if (!slices.empty()) {
		for (Vector2i point { 0, 0 }; point.y < pie_chart_diameter; ++point.y) {

			for (point.x = 0; point.x < pie_chart_diameter; ++point.x) {

				Vector2 offset = point;
				// Move to the centre of the pixel
				offset += Vector2 { 0.5_real, 0.5_real };
				// Normalise to [0, 2]
				offset /= pie_chart_radius;
				// Translate to [-1, 1]
				offset -= Vector2 { 1.0_real, 1.0_real };

				slice_t const* slice = get_slice(offset);

				pie_chart_image->set_pixelv(point, slice != nullptr ? slice->colour : background_colour);
			}
		}
	} else {

		pie_chart_image->fill(background_colour);

	}

	if (can_update) {
		update(pie_chart_image);
	} else {
		set_image(pie_chart_image);
	}

	return OK;
}

Error GFXPieChartTexture::set_slices_array(godot_pie_chart_data_t const& new_slices) {
	slices.clear();
	total_weight = 0.0f;
	for (int32_t i = 0; i < new_slices.size(); ++i) {
		Dictionary const& slice_dict = new_slices[i];
		ERR_CONTINUE_MSG(
			!slice_dict.has(_slice_identifier_key()) || !slice_dict.has(_slice_tooltip_key()) ||
				!slice_dict.has(_slice_colour_key()) || !slice_dict.has(_slice_weight_key()),
			Utilities::format("Invalid slice keys at index %d", i)
		);
		const slice_t slice {
			slice_dict[_slice_identifier_key()], slice_dict[_slice_tooltip_key()], slice_dict[_slice_colour_key()],
			slice_dict[_slice_weight_key()]
		};
		if (slice.weight > 0.0f) {
			total_weight += slice.weight;
			slices.push_back(slice);
		}
	}
	return _generate_pie_chart_image();
}

void GFXPieChartTexture::_bind_methods() {
	OV_BIND_METHOD(GFXPieChartTexture::clear);

	OV_BIND_METHOD(GFXPieChartTexture::set_gfx_pie_chart_name, { "gfx_pie_chart_name" });
	OV_BIND_METHOD(GFXPieChartTexture::get_gfx_pie_chart_name);

	OV_BIND_METHOD(GFXPieChartTexture::set_slices_array, { "new_slices" });
}

GFXPieChartTexture::GFXPieChartTexture() {}

Ref<GFXPieChartTexture> GFXPieChartTexture::make_gfx_pie_chart_texture(GFX::PieChart const* gfx_pie_chart) {
	Ref<GFXPieChartTexture> pie_chart_texture;
	pie_chart_texture.instantiate();
	ERR_FAIL_NULL_V(pie_chart_texture, nullptr);
	ERR_FAIL_COND_V(pie_chart_texture->set_gfx_pie_chart(gfx_pie_chart) != OK, nullptr);
	return pie_chart_texture;
}

void GFXPieChartTexture::clear() {
	gfx_pie_chart = nullptr;
	slices.clear();
	total_weight = 0.0f;

	pie_chart_image.unref();
}

Error GFXPieChartTexture::set_gfx_pie_chart(GFX::PieChart const* new_gfx_pie_chart) {
	if (gfx_pie_chart == new_gfx_pie_chart) {
		return OK;
	}
	if (new_gfx_pie_chart == nullptr) {
		clear();
		return OK;
	}

	gfx_pie_chart = new_gfx_pie_chart;

	return _generate_pie_chart_image();
}

Error GFXPieChartTexture::set_gfx_pie_chart_name(String const& gfx_pie_chart_name) {
	if (gfx_pie_chart_name.is_empty()) {
		return set_gfx_pie_chart(nullptr);
	}
	GFX::Sprite const* sprite = UITools::get_gfx_sprite(gfx_pie_chart_name);
	ERR_FAIL_NULL_V(sprite, FAILED);
	GFX::PieChart const* new_pie_chart = sprite->cast_to<GFX::PieChart>();
	ERR_FAIL_NULL_V_MSG(
		new_pie_chart, FAILED, Utilities::format(
			"Invalid type for GFX sprite %s: %s (expected %s)", gfx_pie_chart_name,
			Utilities::std_to_godot_string(sprite->get_type()),
			Utilities::std_to_godot_string(GFX::PieChart::get_type_static())
		)
	);
	return set_gfx_pie_chart(new_pie_chart);
}

String GFXPieChartTexture::get_gfx_pie_chart_name() const {
	return gfx_pie_chart != nullptr ? Utilities::std_to_godot_string(gfx_pie_chart->get_name()) : String {};
}
