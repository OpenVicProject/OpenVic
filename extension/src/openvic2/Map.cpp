#include "Map.hpp"

#include <sstream>
#include <iomanip>

using namespace OpenVic2;

std::string Province::to_string() const {
	std::ostringstream stream;
	stream << "(" << identifier << ", " << std::hex << std::setfill('0') << std::setw(6) << colour << ")";
	return stream.str();
}

bool Map::add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message) {
	Province new_province = { identifier, colour };
	for (Province const& province : provinces) {
		if (province.identifier == identifier) {
			error_message = "Duplicate province identifiers: " + province.to_string() + " and " + new_province.to_string();
			return false;
		}
		if (province.colour == colour) {
			error_message = "Duplicate province colours: " + province.to_string() + " and " + new_province.to_string();
			return false;
		}
	}
	provinces.push_back(new_province);
	error_message = "Added province: " + new_province.to_string();
	return true;
}