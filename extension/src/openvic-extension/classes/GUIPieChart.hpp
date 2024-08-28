#pragma once

#include <godot_cpp/classes/texture_rect.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"

namespace OpenVic {
	class GUIPieChart : public godot::TextureRect {
		GDCLASS(GUIPieChart, godot::TextureRect)

		godot::Ref<GFXPieChartTexture> gfx_pie_chart_texture;

	protected:
		static void _bind_methods();

	public:
		godot::Error set_gfx_pie_chart(GFX::PieChart const* gfx_pie_chart);

		godot::Ref<GFXPieChartTexture> get_gfx_pie_chart_texture() const;

		godot::Error set_gfx_pie_chart_name(godot::String const& gfx_pie_chart_name);

		godot::String get_gfx_pie_chart_name() const;

		godot::Error set_slices_array(GFXPieChartTexture::godot_pie_chart_data_t const& new_slices) const;
	};
}
