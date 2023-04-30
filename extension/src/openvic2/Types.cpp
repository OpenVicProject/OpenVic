#include "Types.hpp"

#include <cassert>
#include <sstream>
#include <iomanip>

using namespace OpenVic2;

HasIdentifier::HasIdentifier(std::string const& new_identifier) : identifier{ new_identifier } {
	assert(!identifier.empty());
}

std::string const& HasIdentifier::get_identifier() const {
	return identifier;
}

HasColour::HasColour(colour_t const new_colour) : colour(new_colour) {
	assert(colour != NULL_COLOUR && colour <= MAX_COLOUR_RGB);
}

colour_t HasColour::get_colour() const { return colour; }

std::string OpenVic2::HasColour::colour_to_hex_string(colour_t const colour) {
	std::ostringstream stream;
	stream << std::hex << std::setfill('0') << std::setw(6) << colour;
	return stream.str();
}

std::string HasColour::colour_to_hex_string() const {
	return colour_to_hex_string(colour);
}
