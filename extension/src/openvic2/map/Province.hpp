#pragma once

#include "openvic2/map/Building.hpp"

namespace OpenVic2 {
	struct Map;
	struct Region;

	/* REQUIREMENTS:
	 * MAP-5, MAP-8, MAP-43, MAP-47
	 */
	struct Province : HasIdentifier {
		friend struct Map;

		using colour_t = uint32_t;
		using index_t = uint16_t;
		using life_rating_t = int8_t;

		static constexpr colour_t NULL_COLOUR = 0, MAX_COLOUR = 0xFFFFFF;
		static constexpr index_t NULL_INDEX = 0, MAX_INDEX = 0xFFFF;
	private:
		index_t index;
		colour_t colour;
		Region* region = nullptr;
		bool water = false;
		life_rating_t life_rating = 0;
		std::vector<Building> buildings;

		Province(index_t new_index, std::string const& new_identifier, colour_t new_colour);
	public:
		static std::string colour_to_hex_string(colour_t colour);

		index_t get_index() const;
		colour_t get_colour() const;
		Region* get_region() const;
		bool is_water() const;
		life_rating_t get_life_rating() const;
		std::vector<Building> const& get_buildings() const;
		return_t expand_building(std::string const& building_type_identifier);
		std::string to_string() const;

		void update_state(Date const& today);
		void tick(Date const& today);
	};
}
