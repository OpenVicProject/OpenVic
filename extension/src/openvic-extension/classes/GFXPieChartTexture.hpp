#pragma once

#include <godot_cpp/classes/image_texture.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>

#include <openvic-extension/utility/Utilities.hpp>

namespace OpenVic {
	class GFXPieChartTexture : public godot::ImageTexture {
		GDCLASS(GFXPieChartTexture, godot::ImageTexture)

	public:
		using godot_pie_chart_data_t = godot::TypedArray<godot::Dictionary>;
		struct slice_t {
			godot::String name;
			godot::Color colour;
			float weight;
		};

	private:
		GFX::PieChart const* PROPERTY(gfx_pie_chart);
		std::vector<slice_t> slices;
		float PROPERTY(total_weight);
		godot::Ref<godot::Image> pie_chart_image;

		static godot::StringName const& _slice_identifier_key();
		static godot::StringName const& _slice_colour_key();
		static godot::StringName const& _slice_weight_key();

		godot::Error _generate_pie_chart_image();

	public:
		/* Generate slice data from a distribution of objects satisfying HasGetIdentifierAndGetColour, sorted by their weight.
		 * The resulting Array of Dictionaries can be used as an argument for set_slices_array. */
		template<typename Container>
		static godot_pie_chart_data_t distribution_to_slices_array(Container const& dist)
		requires(
			(
				/* ordered_map<T const*, mapped_type>, T derived from HasIdentifierAndColour */
				utility::is_specialization_of_v<Container, tsl::ordered_map>
				/* IndexedMap<T, mapped_type>, T derived from HasIdentifierAndColour */
				|| utility::is_specialization_of_v<Container, IndexedMap>
			)
			&& HasGetIdentifierAndGetColour<std::remove_pointer_t<typename Container::key_type>>
			&& std::convertible_to<typename Container::mapped_type, float>
		) {
			using namespace godot;

			using key_type = std::remove_pointer_t<typename Container::key_type>;
			using entry_t = std::pair<key_type const*, float>;

			std::vector<entry_t> sorted_dist;
			sorted_dist.reserve(dist.size());

			if constexpr (utility::is_specialization_of_v<Container, tsl::ordered_map>) {
				for (auto const& [key, non_float_value] : dist) {
					const float value = static_cast<float>(non_float_value);

					ERR_CONTINUE_MSG(key == nullptr, vformat("Null distribution key with value %f", value));

					if (value != 0.0f) {
						sorted_dist.emplace_back(key, value);
					}
				}
			} else {
				for (size_t index = 0; index < dist.size(); ++index) {
					const float value = static_cast<float>(dist[index]);

					if (value != 0.0f) {
						key_type const* key = &dist(index);
						sorted_dist.emplace_back(key, value);
					}
				}
			}

			std::sort(sorted_dist.begin(), sorted_dist.end(), [](entry_t const& lhs, entry_t const& rhs) -> bool {
				return lhs.first < rhs.first;
			});

			godot_pie_chart_data_t array;
			ERR_FAIL_COND_V(array.resize(sorted_dist.size()) != OK, {});

			for (size_t idx = 0; idx < array.size(); ++idx) {
				auto const& [key, value] = sorted_dist[idx];
				Dictionary sub_dict;
				sub_dict[_slice_identifier_key()] = Utilities::std_to_godot_string(key->get_identifier());
				sub_dict[_slice_colour_key()] = Utilities::to_godot_color(key->get_colour());
				sub_dict[_slice_weight_key()] = value;
				array[idx] = std::move(sub_dict);
			}
			return array;
		}

	protected:
		static void _bind_methods();

	public:
		GFXPieChartTexture();

		// Position must be centered and normalized so that coords are in [-1, 1].
		slice_t const* get_slice(godot::Vector2 const& position) const;

		/* Set slices given an Array of Dictionaries, each with the following key-value entries:
		 *  - colour: Color
		 *  - weight: float */
		godot::Error set_slices_array(godot_pie_chart_data_t const& new_slices);

		/* Create a GFXPieChartTexture using the specified GFX::PieChart. Returns nullptr if gfx_pie_chart fails. */
		static godot::Ref<GFXPieChartTexture> make_gfx_pie_chart_texture(GFX::PieChart const* gfx_pie_chart);

		/* Reset gfx_pie_chart, flag_country and flag_type to nullptr/an empty string, and unreference all images.
		 * This does not affect the godot::ImageTexture, which cannot be reset to a null or empty image. */
		void clear();

		/* Set the GFX::PieChart and regenerate the pie chart image. */
		godot::Error set_gfx_pie_chart(GFX::PieChart const* new_gfx_pie_chart);

		/* Search for a GFX::PieChart with the specified name and, if successful, set it using set_gfx_pie_chart. */
		godot::Error set_gfx_pie_chart_name(godot::String const& gfx_pie_chart_name);

		/* Return the name of the GFX::PieChart, or an empty String if it's null. */
		godot::String get_gfx_pie_chart_name() const;
	};
}
