#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace OpenVic2 {

	struct Province {
		using colour_t = uint32_t;
		friend struct Map;
		static const colour_t NULL_COLOUR = 0, MAX_COLOUR = 0xFFFFFF;
	private:
		std::string identifier;
		colour_t colour;

		Province(std::string const& identifier, colour_t colour);
	public:
		static std::string colour_to_hex_string(colour_t colour);

		std::string const& get_identifier() const;
		colour_t get_colour() const;
		std::string to_string() const;
	};

	struct Map {
	private:
		std::vector<Province> provinces;

	public:
		bool add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message);
		Province* get_province_by_identifier(std::string const& identifier);
		Province* get_province_by_colour(Province::colour_t colour);
	};

}
