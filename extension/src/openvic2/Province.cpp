#include "Map.hpp"

#include <cassert>
#include <sstream>
#include <iomanip>

using namespace OpenVic2;

Province::Province(index_t new_index, std::string const& new_identifier, colour_t new_colour) :
	index(new_index), identifier(new_identifier), colour(new_colour) {
	assert(index != NULL_INDEX);
	assert(!identifier.empty());
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

std::string const& Province::get_identifier() const {
	return identifier;
}

Province::colour_t Province::get_colour() const {
	return colour;
}

Region* Province::get_region() const {
	return region;
}

std::string Province::to_string() const {
	return "(#" + std::to_string(index) + ", " + identifier + ", 0x" + colour_to_hex_string(colour) + ")";
}
