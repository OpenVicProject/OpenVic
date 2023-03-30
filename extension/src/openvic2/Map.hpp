#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace OpenVic2 {

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

		Province(index_t index, std::string const& identifier, colour_t colour);
	public:
		static std::string colour_to_hex_string(colour_t colour);

		index_t get_index() const;
		std::string const& get_identifier() const;
		colour_t get_colour() const;
		std::string to_string() const;
	};

	struct Map {
	private:
		std::vector<Province> provinces;
		bool provinces_locked = false;

	public:
		bool add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message);
		void lock_provinces();
		size_t get_province_count() const;

		Province* get_province_by_index(Province::index_t index);
		Province* get_province_by_identifier(std::string const& identifier);
		Province* get_province_by_colour(Province::colour_t colour);
	};

}
