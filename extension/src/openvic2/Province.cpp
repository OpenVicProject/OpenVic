#include "Province.hpp"

#include <cassert>
#include <sstream>
#include <iomanip>

using namespace OpenVic2;

Province::Province(index_t new_index, std::string const& new_identifier, colour_t new_colour) :
	HasIdentifier(new_identifier), index(new_index), colour(new_colour) {
	assert(index != NULL_INDEX);
	assert(colour != NULL_COLOUR);
}

std::string Province::colour_to_hex_string(colour_t colour) {
	std::ostringstream stream;
	stream << std::hex << std::setfill('0') << std::setw(6) << colour;
	return stream.str();
}

Province::index_t Province::get_index() const {
	return index;
}

Province::colour_t Province::get_colour() const {
	return colour;
}

Region* Province::get_region() const {
	return region;
}

bool Province::is_water() const {
	return water;
}

Province::life_rating_t Province::get_life_rating() const {
	return life_rating;
}

std::string Province::to_string() const {
	return "(#" + std::to_string(index) + ", " + get_identifier() + ", 0x" + colour_to_hex_string(colour) + ")";
}
