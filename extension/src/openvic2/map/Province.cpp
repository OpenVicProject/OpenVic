#include "openvic2/map/Province.hpp"

#include <cassert>
#include <sstream>
#include <iomanip>

using namespace OpenVic2;

Province::Province(index_t new_index, std::string const& new_identifier, colour_t new_colour) :
	HasIdentifier{ new_identifier }, index{ new_index }, colour{ new_colour } {
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

std::vector<Building> const& Province::get_buildings() const {
	return buildings;
}

return_t Province::expand_building(std::string const& building_type_identifier) {
	for (Building& building : buildings)
		if (building.get_type().get_identifier() == building_type_identifier)
			return building.expand();
	return FAILURE;
}

std::string Province::to_string() const {
	std::stringstream stream;
	stream << "(#" << std::to_string(index) << ", " << get_identifier() << ", 0x" << colour_to_hex_string(colour) << ")";
	return stream.str();
}

void Province::update_state(Date const& today) {
	for (Building& building : buildings)
		building.update_state(today);

}

void Province::tick(Date const& today) {
	for (Building& building : buildings)
		building.tick(today);
}
