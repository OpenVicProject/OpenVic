#pragma once

#include <functional>

#include "openvic2/map/Region.hpp"

namespace OpenVic2 {

	struct Mapmode : HasIdentifier {
		friend struct Map;

		using colour_func_t = std::function<Province::colour_t (Map const&, Province const&)>;
		using index_t = size_t;
	private:
		index_t index;
		colour_func_t colour_func;

		Mapmode(index_t new_index, std::string const& new_identifier, colour_func_t new_colour_func);
	public:
		index_t get_index() const;
		colour_func_t get_colour_func() const;
	};

	/* REQUIREMENTS:
	 * MAP-4
	 */
	struct Map {
	private:
		std::vector<Province> provinces;
		std::vector<Region> regions;
		bool provinces_locked = false, water_provinces_locked = false, regions_locked = false;
		size_t water_province_count = 0;

		size_t width = 0, height = 0;
		std::vector<Province::index_t> province_index_image;
		std::vector<Mapmode> mapmodes;
	public:
		return_t add_province(std::string const& identifier, Province::colour_t colour);
		void lock_provinces();
		return_t set_water_province(std::string const& identifier);
		void lock_water_provinces();
		return_t add_region(std::string const& identifier, std::vector<std::string> const& province_identifiers);
		void lock_regions();

		size_t get_province_count() const;
		Province* get_province_by_index(Province::index_t index);
		Province const* get_province_by_index(Province::index_t index) const;
		Province* get_province_by_identifier(std::string const& identifier);
		Province const* get_province_by_identifier(std::string const& identifier) const;
		Province* get_province_by_colour(Province::colour_t colour);
		Province const* get_province_by_colour(Province::colour_t colour) const;
		Province::index_t get_province_index_at(size_t x, size_t y) const;

		Region* get_region_by_identifier(std::string const& identifier);
		Region const* get_region_by_identifier(std::string const& identifier) const;

		return_t generate_province_index_image(size_t new_width, size_t new_height, uint8_t const* colour_data);
		size_t get_width() const;
		size_t get_height() const;
		std::vector<Province::index_t> const& get_province_index_image() const;

		return_t add_mapmode(std::string const& identifier, Mapmode::colour_func_t colour_func);
		size_t get_mapmode_count() const;
		Mapmode const* get_mapmode_by_index(Mapmode::index_t index) const;
		Mapmode const* get_mapmode_by_identifier(std::string const& identifier) const;
		return_t generate_mapmode_colours(Mapmode::index_t index, uint8_t* target) const;

		void generate_province_buildings(BuildingManager const& manager);

		void update_state(Date const& today);
		void tick(Date const& today);
	};
}
