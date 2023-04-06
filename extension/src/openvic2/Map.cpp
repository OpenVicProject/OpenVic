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

std::string Province::to_string() const {
	return "(#" + std::to_string(index) + ", " + identifier + ", 0x" + colour_to_hex_string(colour) + ")";
}

bool Map::add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message) {
	if (provinces_locked) {
		error_message = "The map's province list has already been locked!";
		return false;
	}
	if (provinces.size() >= Province::MAX_INDEX) {
		error_message = "The map's province list is full - there can be at most " + std::to_string(Province::MAX_INDEX) + " provinces";
		return false;
	}
	if (colour == Province::NULL_COLOUR || colour > Province::MAX_COLOUR) {
		error_message = "Invalid province colour: " + Province::colour_to_hex_string(colour);
		return false;
	}
	Province new_province{ static_cast<Province::index_t>(provinces.size() + 1), identifier, colour };
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

void Map::lock_provinces() {
	provinces_locked = true;
}

size_t Map::get_province_count() const {
	return provinces.size();
}

Province* Map::get_province_by_index(Province::index_t index) {
	return index != Province::NULL_INDEX && index <= provinces.size() ? &provinces[index - 1] : nullptr;
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
