#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "Types.hpp"

namespace OpenVic2 {
	struct Region;

	struct Province {
		using colour_t = uint32_t;
		using index_t = uint16_t;
		friend struct Map;
		static const colour_t NULL_COLOUR = 0, MAX_COLOUR = 0xFFFFFF;
		static const index_t NULL_INDEX = 0, MAX_INDEX = 0xFFFF;
	private:
		index_t index;
		std::string identifier;
		colour_t colour;
		Region* region = nullptr;

		Province(index_t new_index, std::string const& new_identifier, colour_t new_colour);
	public:
		static std::string colour_to_hex_string(colour_t colour);

		index_t get_index() const;
		std::string const& get_identifier() const;
		colour_t get_colour() const;
		Region* get_region() const;
		std::string to_string() const;
	};

	struct Region {
		friend struct Map;
	private:
		std::string identifier;
		std::vector<Province*> provinces;

		Region(std::string const& new_identifier);
	public:
		std::string const& get_identifier() const;
		size_t get_province_count() const;
		bool contains_province(Province const* province) const;
		std::vector<Province*> const& get_provinces() const;
	};

	struct Mapmode {
		using colour_func_t = Province::colour_t (*)(Map const& map, Province const& province);
		using index_t = size_t;
		friend struct Map;
	private:
		index_t index;
		std::string identifier;
		colour_func_t colour_func;

		Mapmode(index_t new_index, std::string const& new_identifier, colour_func_t new_colour_func);
	public:
		index_t get_index() const;
		std::string const& get_identifier() const;
		colour_func_t get_colour_func() const;
	};

	struct Map {
	private:
		std::vector<Province> provinces;
		std::vector<Region> regions;
		bool provinces_locked = false, regions_locked = false;

		size_t width = 0, height = 0;
		std::vector<Province::index_t> province_index_image;
		std::vector<Mapmode> mapmodes;
	public:
		return_t add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message);
		void lock_provinces();
		return_t add_region(std::string const& identifier, std::vector<std::string> const& province_identifiers, std::string& error_message);
		void lock_regions();
		size_t get_province_count() const;

		Province* get_province_by_index(Province::index_t index);
		Province const* get_province_by_index(Province::index_t index) const;
		Province* get_province_by_identifier(std::string const& identifier);
		Province const* get_province_by_identifier(std::string const& identifier) const;
		Province* get_province_by_colour(Province::colour_t colour);
		Province const* get_province_by_colour(Province::colour_t colour) const;

		return_t generate_province_index_image(size_t new_width, size_t new_height, uint8_t const* colour_data, std::string& error_message);
		size_t get_width() const;
		size_t get_height() const;
		std::vector<Province::index_t> const& get_province_index_image() const;

		return_t add_mapmode(std::string const& identifier, Mapmode::colour_func_t colour_func, std::string& error_message);
		size_t get_mapmode_count() const;
		Mapmode const* get_mapmode_by_index(Mapmode::index_t index) const;
		Mapmode const* get_mapmode_by_identifier(std::string const& identifier) const;
		return_t generate_mapmode_colours(Mapmode::index_t index, uint8_t* target, std::string& error_message) const;
	};
}
