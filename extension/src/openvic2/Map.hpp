#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace OpenVic2 {

	struct Province {
		using colour_t = uint32_t;

		static const colour_t NULL_COLOUR = 0;

		std::string identifier;
		colour_t colour;
		
		std::string to_string() const;
	};

	struct Map {
	private:
		std::vector<Province> provinces;

	public:
		bool add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message);
	};

}
