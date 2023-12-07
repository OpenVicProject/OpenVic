#pragma once

#include <godot_cpp/classes/image_texture.hpp>

#include <openvic-simulation/interface/GFX.hpp>

namespace OpenVic {
	class GFXPieChartTexture : public godot::ImageTexture {
		GDCLASS(GFXPieChartTexture, godot::ImageTexture)

		using slice_t = std::pair<godot::Color, float>;

		GFX::PieChart const* PROPERTY(gfx_pie_chart);
		std::vector<slice_t> slices;
		float total_weight;
		godot::Ref<godot::Image> pie_chart_image;

		godot::Error _generate_pie_chart_image();

	protected:
		static void _bind_methods();

	public:
		GFXPieChartTexture();

		/* Set slices given new_slices, an Array of Dictionaries, each with the following keys:
		 *  - colour: Color
		 *  - weight: float
		 */
		godot::Error set_slices(godot::Array const& new_slices);

		/* Create a GFXPieChartTexture using the specific GFX::PieChart.
		 * Returns nullptr if setting gfx_pie_chart fails. */
		static godot::Ref<GFXPieChartTexture> make_gfx_pie_chart_texture(GFX::PieChart const* gfx_pie_chart);

		/* Reset gfx_pie_chart, flag_country and flag_type to nullptr/an empty string, and unreference all images.
		 * This does not affect the godot::ImageTexture, which cannot be reset to a null or empty image. */
		void clear();

		/* Set the GFX::PieChart and regenerate the pie chart image. */
		godot::Error set_gfx_pie_chart(GFX::PieChart const* new_gfx_pie_chart);

		/* Search for a GFX::PieChart with the specfied name and, if successful, set it using set_gfx_pie_chart. */
		godot::Error set_gfx_pie_chart_name(godot::String const& gfx_pie_chart_name);

		/* Return the name of the GFX::PieChart, or an empty String if it's null */
		godot::String get_gfx_pie_chart_name() const;
	};
}
