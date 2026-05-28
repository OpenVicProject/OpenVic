#pragma once

#include <functional>
#include <type_traits>

#include <godot_cpp/classes/image_texture.hpp>

#include <openvic-simulation/interface/GFXSprite.hpp>
#include <openvic-simulation/types/Colour.hpp>
#include <openvic-simulation/types/IndexedFlatMap.hpp>
#include <openvic-simulation/utility/Logger.hpp>

#include "openvic-extension/core/Convert.hpp"
#include "openvic-extension/utility/MapHelpers.hpp"
#include "openvic-simulation/core/template/Concepts.hpp"

#include <type_safe/strong_typedef.hpp>

namespace OpenVic {
	template<typename T>
	concept IsPieChartKey = has_get_identifier_and_colour<std::remove_pointer_t<T>>;

	template<typename T>
	concept IsPieChartValueTypeSafe = is_strongly_typed<T> && std::is_constructible_v<float, type_safe::underlying_type<T>>;
	template<typename T>
	concept IsPieChartValue = std::is_constructible_v<float, T> || IsPieChartValueTypeSafe<T>;

	template<typename MapType>
	concept IsPieChartDistribution = (
			/* tsl::ordered_map<KeyType const*, ValueType>, KeyType derived from HasIdentifierAndColour */
			specialization_of<MapType, tsl::ordered_map>
			/* IndexedFlatMap<KeyType, ValueType>, KeyType derived from HasIdentifierAndColour */
			|| specialization_of<MapType, IndexedFlatMap>
		)
		&& IsPieChartKey<map_key_t<MapType>>
		&& IsPieChartValue<map_value_t<MapType>>;

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
		template<IsPieChartKey KeyType, IsPieChartValue ValueType>
		static godot_pie_chart_data_t distribution_to_slices_array(
			std::span<std::add_const_t<KeyType>> keys,
			std::span<std::add_const_t<ValueType>> values,
			NodeTools::Functor<
				// return tooltip; args: key const*, identifier, weight, total weight
				godot::String, std::add_const_t<std::remove_pointer_t<KeyType>>&, godot::String const&, float, float
			> auto make_tooltip,
			godot::String const& identifier_suffix = {}
		) {
			assert(keys.size() == values.size());
			using key_t = std::add_const_t<std::remove_pointer_t<KeyType>>;
			using key_ref_wrap_t = std::reference_wrapper<key_t>;
			using entry_t = std::pair<
				key_ref_wrap_t,
				float
			>;

			memory::FixedVector<entry_t> sorted_distribution { create_empty, keys.size() };

			float total_weight = 0.0f;
			for (size_t i = 0; i < keys.size(); ++i) {
				key_t& key = [&]() -> key_t& {
					if constexpr (requires { static_cast<key_t&>(keys[i]); }) {
						return keys[i];
					} else if constexpr (requires { static_cast<key_t&>(*keys[i]); }) {
						return *keys[i];
					}
				}();
				float value;
				if constexpr (IsPieChartValueTypeSafe<ValueType>) {
					value = static_cast<float>(type_safe::get(values[i]));
				} else {
					value = static_cast<float>(values[i]);
				}

				if (value > 0.0f) {
					sorted_distribution.emplace_back(key, value);

					total_weight += value;
				} else if (value < 0.0f) {
					spdlog::error_s(
						"Negative distribution value {} for key \"{}\"",
						value, key
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
					if constexpr (requires { lhs.first.get() < rhs.first.get(); }) {
						return lhs.first.get() < rhs.first.get();
					} else if constexpr (requires { lhs.first.get().index < rhs.first.get().index; }) {
						return lhs.first.get().index < rhs.first.get().index;
					} else if constexpr (requires { lhs.first.get().get_identifier() < rhs.first.get().get_identifier(); }) {
						return lhs.first.get().get_identifier() < rhs.first.get().get_identifier();;
					} else {
						static_assert(!std::is_same_v<KeyType, KeyType>, "distribution_to_slices_array's sorting does not support KeyType");
					}
				}
			);

			godot_pie_chart_data_t array;
			ERR_FAIL_COND_V(array.resize(sorted_distribution.size()) != godot::OK, {});

			for (size_t index = 0; index < array.size(); ++index) {
				auto const& [key_ref, value] = sorted_distribution[index];
				key_t& key = key_ref.get();

				godot::String identifier = convert_to<godot::String>(key.get_identifier());
				identifier += identifier_suffix;

				godot::Dictionary sub_dict;

				sub_dict[_slice_tooltip_key()] = make_tooltip(key, identifier, value, total_weight);
				sub_dict[_slice_identifier_key()] = std::move(identifier);
				sub_dict[_slice_colour_key()] = convert_to<godot::Color>(key.get_colour());
				sub_dict[_slice_weight_key()] = value;

				array[index] = std::move(sub_dict);
			}
			return array;
		}

		template<IsPieChartDistribution MapType>
		static godot_pie_chart_data_t distribution_to_slices_array(
			MapType const& distribution,
			NodeTools::Functor<
				// return tooltip; args: key const*, identifier, weight, total weight
				godot::String, std::add_const_t<std::remove_pointer_t<map_key_t<MapType>>>&, godot::String const&, float, float
			> auto make_tooltip,
			godot::String const& identifier_suffix = {}
		) {
			memory::FixedVector<map_key_t<MapType>> keys { create_empty, distribution.size() };
			memory::FixedVector<map_value_t<MapType>> values { create_empty, distribution.size() };
			for (auto const& [k, v] : distribution) {
				keys.emplace_back(k);
				values.emplace_back(v);
			}
			return distribution_to_slices_array<map_key_t<MapType>, map_value_t<MapType>>(
				keys, values, make_tooltip, identifier_suffix
			);
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
