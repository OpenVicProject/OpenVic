#pragma once

#include <godot_cpp/classes/image_texture.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>

#include "openvic-extension/utility/Utilities.hpp"

namespace OpenVic {
	class GFXPieChartTexture : public godot::ImageTexture {
		GDCLASS(GFXPieChartTexture, godot::ImageTexture)

		using slice_t = std::pair<godot::Color, float>;

		GFX::PieChart const* PROPERTY(gfx_pie_chart);
		std::vector<slice_t> slices;
		float total_weight;
		godot::Ref<godot::Image> pie_chart_image;

		static godot::StringName const& _slice_identifier_key();
		static godot::StringName const& _slice_colour_key();
		static godot::StringName const& _slice_weight_key();

		godot::Error _generate_pie_chart_image();

	protected:
		static void _bind_methods();

	public:
		GFXPieChartTexture();

		using godot_pie_chart_data_t = godot::TypedArray<godot::Dictionary>;

		/* Set slices given an Array of Dictionaries, each with the following key-value entries:
		 *  - colour: Color
		 *  - weight: float */
		godot::Error set_slices_array(godot_pie_chart_data_t const& new_slices);

		/* Generate slice data from a distribution of HasIdentifierAndColour derived objects, sorted by their weight.
		 * The resulting Array of Dictionaries can be used as an argument for set_slices_array. */
		template<std::derived_from<HasIdentifierAndColour> T>
		static godot_pie_chart_data_t distribution_to_slices_array(fixed_point_map_t<T const*> const& dist) {
			using namespace godot;
			using entry_t = std::pair<T const*, fixed_point_t>;
			std::vector<entry_t> sorted_dist;
			sorted_dist.reserve(dist.size());
			for (entry_t const& entry : dist) {
				ERR_CONTINUE_MSG(
					entry.first == nullptr, vformat("Null distribution key with value %f", entry.second.to_float())
				);
				sorted_dist.push_back(entry);
			}
			std::sort(sorted_dist.begin(), sorted_dist.end(), [](entry_t const& lhs, entry_t const& rhs) -> bool {
				return lhs.first < rhs.first;
			});
			godot_pie_chart_data_t array;
			ERR_FAIL_COND_V(array.resize(sorted_dist.size()) != OK, {});
			for (size_t idx = 0; idx < array.size(); ++idx) {
				auto const& [key, val] = sorted_dist[idx];
				Dictionary sub_dict;
				sub_dict[_slice_identifier_key()] = Utilities::std_view_to_godot_string(key->get_identifier());
				sub_dict[_slice_colour_key()] = Utilities::to_godot_color(key->get_colour());
				sub_dict[_slice_weight_key()] = val.to_float();
				array[idx] = std::move(sub_dict);
			}
			return array;
		}

		/* Create a GFXPieChartTexture using the specified GFX::PieChart. Returns nullptr if gfx_pie_chart fails. */
		static godot::Ref<GFXPieChartTexture> make_gfx_pie_chart_texture(GFX::PieChart const* gfx_pie_chart);

		/* Reset gfx_pie_chart, flag_country and flag_type to nullptr/an empty string, and unreference all images.
		 * This does not affect the godot::ImageTexture, which cannot be reset to a null or empty image. */
		void clear();

		/* Set the GFX::PieChart and regenerate the pie chart image. */
		godot::Error set_gfx_pie_chart(GFX::PieChart const* new_gfx_pie_chart);

		/* Search for a GFX::PieChart with the specfied name and, if successful, set it using set_gfx_pie_chart. */
		godot::Error set_gfx_pie_chart_name(godot::String const& gfx_pie_chart_name);

		/* Return the name of the GFX::PieChart, or an empty String if it's null. */
		godot::String get_gfx_pie_chart_name() const;
	};
}
