#include "GFXPieChartTexture.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_view_to_godot_string;
using OpenVic::Utilities::std_view_to_godot_string_name;

#define PI std::numbers::pi_v<float>

Error GFXPieChartTexture::_generate_pie_chart_image() {
	ERR_FAIL_NULL_V(gfx_pie_chart, FAILED);
	if (gfx_pie_chart->get_size() <= 0) {
		UtilityFunctions::push_error("Invalid GFX::PieChart size for GFXPieChartTexture - ", gfx_pie_chart->get_size());
		return FAILED;
	}
	const int32_t pie_chart_size = 2 * gfx_pie_chart->get_size();
	bool can_update = true;
	if (
		pie_chart_image.is_null() || pie_chart_image->get_width() != pie_chart_size ||
		pie_chart_image->get_height() != pie_chart_size
	) {
		pie_chart_image = Image::create(pie_chart_size, pie_chart_size, false, Image::FORMAT_RGBA8);
		ERR_FAIL_NULL_V(pie_chart_image, FAILED);
		can_update = false;
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

Error GFXPieChartTexture::set_slices(Array const& new_slices) {
	static const StringName colour_key = "colour";
	static const StringName weight_key = "weight";

	slices.clear();
	total_weight = 0.0f;
	for (int32_t i = 0; i < new_slices.size(); ++i) {
		Dictionary const& slice_dict = new_slices[i];
		if (!slice_dict.has(colour_key) || !slice_dict.has(weight_key)) {
			UtilityFunctions::push_error("Invalid slice keys at index ", i, " - ", slice_dict);
			continue;
		}
		const slice_t slice = std::make_pair(slice_dict[colour_key], slice_dict[weight_key]);
		if (slice.second <= 0.0f) {
			UtilityFunctions::push_error("Invalid slice weight at index ", i, " - ", slice.second);
			continue;
		}
		total_weight += slice.second;
		slices.emplace_back(std::move(slice));
	}
	return _generate_pie_chart_image();
}

void GFXPieChartTexture::_bind_methods() {
	OV_BIND_METHOD(GFXPieChartTexture::clear);

	OV_BIND_METHOD(GFXPieChartTexture::set_gfx_pie_chart_name, { "gfx_pie_chart_name" });
	OV_BIND_METHOD(GFXPieChartTexture::get_gfx_pie_chart_name);

	OV_BIND_METHOD(GFXPieChartTexture::set_slices, { "new_slices" });
}

GFXPieChartTexture::GFXPieChartTexture() : total_weight { 0.0f } {}

Ref<GFXPieChartTexture> GFXPieChartTexture::make_gfx_pie_chart_texture(GFX::PieChart const* gfx_pie_chart) {
	Ref<GFXPieChartTexture> pie_chart_texture;
	pie_chart_texture.instantiate();
	ERR_FAIL_NULL_V(pie_chart_texture, nullptr);
	if (pie_chart_texture->set_gfx_pie_chart(gfx_pie_chart) == OK) {
		return pie_chart_texture;
	} else {
		return nullptr;
	}
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
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	GFX::Sprite const* sprite = game_singleton->get_game_manager().get_ui_manager().get_sprite_by_identifier(
		godot_to_std_string(gfx_pie_chart_name)
	);
	ERR_FAIL_NULL_V_MSG(sprite, FAILED, vformat("GFX sprite not found: %s", gfx_pie_chart_name));
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
