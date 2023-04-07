#pragma once

#include <string>
#include <cstdint>
#include <vector>

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

	struct Map {
	private:
		std::vector<Province> provinces;
		std::vector<Region> regions;
		bool provinces_locked = false, regions_locked = false;

	public:
		bool add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message);
		void lock_provinces();
		bool add_region(std::string const& identifier, std::vector<std::string> const& province_identifiers, std::string& error_message);
		void lock_regions();
		size_t get_province_count() const;

		Province* get_province_by_index(Province::index_t index);
		Province const* get_province_by_index(Province::index_t index) const;
		Province* get_province_by_identifier(std::string const& identifier);
		Province const* get_province_by_identifier(std::string const& identifier) const;
		Province* get_province_by_colour(Province::colour_t colour);
		Province const* get_province_by_colour(Province::colour_t colour) const;
	};

}
