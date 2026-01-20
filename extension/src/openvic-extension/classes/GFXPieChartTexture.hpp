#pragma once

#include <godot_cpp/classes/image_texture.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>
#include <openvic-simulation/types/Colour.hpp>
#include <openvic-simulation/types/IndexedFlatMap.hpp>
#include <openvic-simulation/utility/Logger.hpp>

#include "openvic-extension/core/Convert.hpp"
#include "openvic-extension/utility/MapHelpers.hpp"
#include "openvic-simulation/core/template/Concepts.hpp"

namespace OpenVic {
	template<typename MapType>
	concept IsPieChartDistribution = (
			/* tsl::ordered_map<KeyType const*, ValueType>, KeyType derived from HasIdentifierAndColour */
			specialization_of<MapType, tsl::ordered_map>
			/* IndexedFlatMap<KeyType, ValueType>, KeyType derived from HasIdentifierAndColour */
			|| specialization_of<MapType, IndexedFlatMap>
		)
		&& has_get_identifier_and_colour<std::remove_pointer_t<map_key_t<MapType>>>
		&& (
			requires { static_cast<float>(std::declval<map_value_t<MapType>>()); }
			|| (
				is_strongly_typed<map_value_t<MapType>>
				&& requires { static_cast<float>(type_safe::get(std::declval<map_value_t<MapType>>())); }
			)
		);

	class GFXPieChartTexture : public godot::ImageTexture {
		GDCLASS(GFXPieChartTexture, godot::ImageTexture)

	public:
		using godot_pie_chart_data_t = godot::TypedArray<godot::Dictionary>;
		struct slice_t {
			godot::String identifier;
			godot::String tooltip;
			godot::Color colour;
			float weight;
		};

	private:
		GFX::PieChart const* PROPERTY(gfx_pie_chart, nullptr);
		std::vector<slice_t> slices;
		float PROPERTY(total_weight, 0.0f);
		godot::Ref<godot::Image> pie_chart_image;

		static godot::StringName const& _slice_identifier_key();
		static godot::StringName const& _slice_tooltip_key();
		static godot::StringName const& _slice_colour_key();
		static godot::StringName const& _slice_weight_key();

		godot::Error _generate_pie_chart_image();

	public:
		/* Generate slice data from a distribution of objects satisfying HasGetIdentifierAndGetColour, sorted by their weight.
		 * The resulting Array of Dictionaries can be used as an argument for set_slices_array. */
		template<IsPieChartDistribution MapType>
		static godot_pie_chart_data_t distribution_to_slices_array(
			MapType const& distribution,
			NodeTools::Functor<
				// return tooltip; args: key const*, identifier, weight, total weight
				godot::String, std::remove_pointer_t<map_key_t<MapType>> const*, godot::String const&, float, float
			> auto make_tooltip,
			godot::String const& identifier_suffix = {}
		) {
			using namespace godot;

			using key_type = std::remove_pointer_t<map_key_t<MapType>>;
			using entry_t = std::pair<key_type const*, float>;

			std::vector<entry_t> sorted_distribution;
			if constexpr (specialization_of<MapType, IndexedFlatMap>) {
				sorted_distribution.reserve(distribution.get_count());
			} else {
				sorted_distribution.reserve(distribution.size());
			}

			float total_weight = 0.0f;

			for (auto [key_ref_or_ptr, non_float_value] : distribution) {
				key_type const* key_ptr;
				if constexpr (std::same_as<decltype(key_ptr), decltype(key_ref_or_ptr)>) {
					key_ptr = key_ref_or_ptr;
				} else {
					key_ptr = &key_ref_or_ptr;
				}

				float value;
				if constexpr (is_strongly_typed<decltype(non_float_value)>) {
					value = static_cast<float>(type_safe::get(non_float_value));
				} else {
					value = static_cast<float>(non_float_value);
				}

				if (value > 0.0f) {
					sorted_distribution.emplace_back(key_ptr, value);

					total_weight += value;
				} else if (value < 0.0f) {
					spdlog::error_s(
						"Negative distribution value {} for key \"{}\"",
						value, *key_ptr
					);
				}
			}

			// To avoid division by zero.
			if (total_weight == 0.0f) {
				total_weight = 1.0f;
			}

			std::sort(
				sorted_distribution.begin(), sorted_distribution.end(),
				[](entry_t const& lhs, entry_t const& rhs) -> bool {
					return lhs.first < rhs.first;
				}
			);

			godot_pie_chart_data_t array;
			ERR_FAIL_COND_V(array.resize(sorted_distribution.size()) != OK, {});

			for (size_t index = 0; index < array.size(); ++index) {
				auto const& [key, value] = sorted_distribution[index];

				String identifier = convert_to<String>(key->get_identifier());
				identifier += identifier_suffix;

				Dictionary sub_dict;

				sub_dict[_slice_tooltip_key()] = make_tooltip(key, identifier, value, total_weight);
				sub_dict[_slice_identifier_key()] = std::move(identifier);
				sub_dict[_slice_colour_key()] = convert_to<Color>(key->get_colour());
				sub_dict[_slice_weight_key()] = value;

				array[index] = std::move(sub_dict);
			}
			return array;
		}

	protected:
		static void _bind_methods();

	public:
		GFXPieChartTexture();

		// Position must be centred and normalised so that coords are in [-1, 1].
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
