#include "Map.hpp"

#include <sstream>
#include <iomanip>

using namespace OpenVic2;

Province::Province(std::string const& new_identifier, colour_t new_colour) :
	identifier(new_identifier), colour(new_colour) {}

std::string Province::colour_to_hex_string(colour_t colour) {
	std::ostringstream stream;
	stream << std::hex << std::setfill('0') << std::setw(6) << colour;
	return stream.str();
}

std::string const& Province::get_identifier() const {
	return identifier;
}

Province::colour_t Province::get_colour() const {
	return colour;
}

std::string Province::to_string() const {
	return "(" + identifier + ", " + colour_to_hex_string(colour) + ")";
}

bool Map::add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message) {
	if (colour == Province::NULL_COLOUR || colour > Province::MAX_COLOUR) {
		error_message = "Invalid province colour: " + Province::colour_to_hex_string(colour);
		return false;
	}
	Province new_province{ identifier, colour };
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

Province* Map::get_province_by_identifier(std::string const& identifier) {
	for (Province& province : provinces)
		if (province.identifier == identifier) return &province;
	return nullptr;
}

Province* Map::get_province_by_colour(Province::colour_t colour) {
	for (Province& province : provinces)
		if (province.colour == colour) return &province;
	return nullptr;
}
