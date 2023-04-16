#include "Map.hpp"

#include <cassert>
#include <sstream>
#include <iomanip>

using namespace OpenVic2;

Province::Province(index_t newIndex, std::string const& newIdentifier, colour_t newColour) :
	index(newIndex), identifier(newIdentifier), colour(newColour) {
	assert(index != NULL_INDEX);
	assert(!identifier.empty());
	assert(colour != NULL_COLOUR);
}

std::string Province::colourToHexString(colour_t colour) {
	std::ostringstream stream;
	stream << std::hex << std::setfill('0') << std::setw(6) << colour;
	return stream.str();
}

Province::index_t Province::getIndex() const {
	return index;
}

std::string const& Province::getIdentifier() const {
	return identifier;
}

Province::colour_t Province::getColour() const {
	return colour;
}

Region* Province::getRegion() const {
	return region;
}

std::string Province::toString() const {
	return "(#" + std::to_string(index) + ", " + identifier + ", 0x" + colourToHexString(colour) + ")";
}
