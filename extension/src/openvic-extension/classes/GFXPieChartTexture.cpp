#include "GFXPieChartTexture.hpp"

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/UITools.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_view_to_godot_string;
using OpenVic::Utilities::std_view_to_godot_string_name;

StringName const& GFXPieChartTexture::_slice_identifier_key() {
	static StringName const slice_identifier_key = "identifier";
	return slice_identifier_key;
}
StringName const& GFXPieChartTexture::_slice_colour_key() {
	static StringName const slice_colour_key = "colour";
	return slice_colour_key;
}
StringName const& GFXPieChartTexture::_slice_weight_key() {
	static StringName const slice_weight_key = "weight";
	return slice_weight_key;
}

static constexpr float PI = std::numbers::pi_v<float>;

Error GFXPieChartTexture::_generate_pie_chart_image() {
	ERR_FAIL_NULL_V(gfx_pie_chart, FAILED);
	ERR_FAIL_COND_V_MSG(
		gfx_pie_chart->get_size() <= 0, FAILED,
		vformat("Invalid GFX::PieChart size for GFXPieChartTexture - %d", gfx_pie_chart->get_size())
	);
	const int32_t pie_chart_size = 2 * gfx_pie_chart->get_size();
	/* Whether we've already set the ImageTexture to an image of the right dimensions,
	 * and so can update it without creating and setting a new image, or not. */
	const bool can_update = pie_chart_image.is_valid() && pie_chart_image->get_width() == pie_chart_size
		&& pie_chart_image->get_height() == pie_chart_size;
	if (!can_update) {
		pie_chart_image = Image::create(pie_chart_size, pie_chart_size, false, Image::FORMAT_RGBA8);
		ERR_FAIL_NULL_V(pie_chart_image, FAILED);
	}

	static const Color background_colour { 0.0f, 0.0f, 0.0f, 0.0f };
	if (!slices.empty()) {
		const float pie_chart_radius = gfx_pie_chart->get_size();
		const Vector2 centre_translation = Vector2 { 0.5f, 0.5f } - static_cast<Vector2>(pie_chart_image->get_size()) * 0.5f;
		for (Vector2i point { 0, 0 }; point.y < pie_chart_image->get_height(); ++point.y) {
			for (point.x = 0; point.x < pie_chart_image->get_width(); ++point.x) {
				const Vector2 offset = centre_translation + point;
				if (offset.length() <= pie_chart_radius) {
					float theta = 0.5f * PI + atan2(offset.y, offset.x);
					if (theta < 0.0f) {
						theta += 2.0f * PI;
					}
					/* Rescale angle so that total_weight is a full rotation. */
					theta *= total_weight / (2.0f * PI);
					Color colour = slices.front().first;
					/* Find the slice theta lies in. */
					for (slice_t const& slice : slices) {
						if (theta <= slice.second) {
							colour = slice.first;
							break;
						} else {
							theta -= slice.second;
						}
					}
					pie_chart_image->set_pixelv(point, colour);
				} else {
					pie_chart_image->set_pixelv(point, background_colour);
				}
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

Error GFXPieChartTexture::set_slices_array(TypedArray<Dictionary> const& new_slices) {
	slices.clear();
	total_weight = 0.0f;
	for (int32_t i = 0; i < new_slices.size(); ++i) {
		Dictionary const& slice_dict = new_slices[i];
		ERR_CONTINUE_MSG(
			!slice_dict.has(_slice_colour_key()) || !slice_dict.has(_slice_weight_key()), vformat("Invalid slice keys at index %d", i)
		);
		slice_t slice = std::make_pair(slice_dict[_slice_colour_key()], slice_dict[_slice_weight_key()]);
		ERR_CONTINUE_MSG(slice.second <= 0.0f, vformat("Invalid slice values at index %d", i));
		total_weight += slice.second;
		slices.emplace_back(std::move(slice));
	}
	return _generate_pie_chart_image();
}

void GFXPieChartTexture::_bind_methods() {
	OV_BIND_METHOD(GFXPieChartTexture::clear);

	OV_BIND_METHOD(GFXPieChartTexture::set_gfx_pie_chart_name, { "gfx_pie_chart_name" });
	OV_BIND_METHOD(GFXPieChartTexture::get_gfx_pie_chart_name);

	OV_BIND_METHOD(GFXPieChartTexture::set_slices_array, { "new_slices" });
}

GFXPieChartTexture::GFXPieChartTexture() : gfx_pie_chart { nullptr }, total_weight { 0.0f } {}

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
		new_pie_chart, FAILED, vformat(
			"Invalid type for GFX sprite %s: %s (expected %s)", gfx_pie_chart_name,
			std_view_to_godot_string(sprite->get_type()), std_view_to_godot_string(GFX::PieChart::get_type_static())
		)
	);
	return set_gfx_pie_chart(new_pie_chart);
}

String GFXPieChartTexture::get_gfx_pie_chart_name() const {
	return gfx_pie_chart != nullptr ? std_view_to_godot_string(gfx_pie_chart->get_name()) : String {};
}
