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

Region::Region(std::string const& new_identifier) : identifier(new_identifier) {
	assert(!identifier.empty());
}

std::string const& Region::get_identifier() const {
	return identifier;
}

size_t Region::get_province_count() const {
	return provinces.size();
}

bool Region::contains_province(Province const* province) const {
	return province && std::find(provinces.begin(), provinces.end(), province) != provinces.end();
}

std::vector<Province*> const& Region::get_provinces() const {
	return provinces;
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
	if (identifier.empty()) {
		error_message = "Empty province identifier for colour " + Province::colour_to_hex_string(colour);
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

bool Map::add_region(std::string const& identifier, std::vector<std::string> const& province_identifiers, std::string& error_message) {
	if (regions_locked) {
		error_message = "The map's region list has already been locked!";
		return false;
	}
	if (identifier.empty()) {
		error_message = "Empty region identifier!";
		return false;
	}
	if (provinces.empty()) {
		error_message = "Empty province list for region " + identifier;
		return false;
	}
	Region new_region{ identifier };
	error_message = "Error message for region: " + identifier;
	static const std::string SEPARATOR = "\n - ";
	for (std::string const& province_identifier : province_identifiers) {
		Province* province = get_province_by_identifier(province_identifier);
		if (province) {
			if (new_region.contains_province(province))
				error_message += SEPARATOR + "Duplicate province identifier " + province_identifier;
			else {
				if (province->region) {
					error_message += SEPARATOR + "Province " + province_identifier + " is already part of ";
					const size_t other_region_index = reinterpret_cast<size_t>(province->region) - 1;
					if (other_region_index < regions.size())
						error_message += regions[other_region_index].get_identifier();
					else
						error_message += "an unknown region with index " + std::to_string(other_region_index);
				} else new_region.provinces.push_back(province);
			}
		} else error_message += SEPARATOR + "Invalid province identifier " + province_identifier;
	}
	if (!new_region.get_province_count()) {
		error_message += SEPARATOR + "No valid provinces in region's list";
		return false;
	}
	for (Region const& region : regions) {
		if (region.identifier == identifier) {
			error_message += SEPARATOR + "Duplicate region identifiers: " + region.get_identifier() + " and " + identifier;
			return false;
		}
	}
	regions.push_back(new_region);
	error_message += SEPARATOR + "Added region: " + identifier;
	// Used to detect provinces listed in multiple regions, will
	// be corrected once regions is stable (i.e. lock_regions).
	Region* tmp_region_index = reinterpret_cast<Region*>(regions.size());
	for (Province* province : new_region.provinces)
		province->region = tmp_region_index;
	return true;
}

void Map::lock_regions() {
	regions_locked = true;
	for (Region& region : regions)
		for (Province* province : region.provinces)
			province->region = &region;
}

size_t Map::get_province_count() const {
	return provinces.size();
}

Province* Map::get_province_by_index(Province::index_t index) {
	return index != Province::NULL_INDEX && index <= provinces.size() ? &provinces[index - 1] : nullptr;
}

Province const* Map::get_province_by_index(Province::index_t index) const {
	return index != Province::NULL_INDEX && index <= provinces.size() ? &provinces[index - 1] : nullptr;
}

Province* Map::get_province_by_identifier(std::string const& identifier) {
	if (!identifier.empty())
		for (Province& province : provinces)
			if (province.identifier == identifier) return &province;
	return nullptr;
}

Province const* Map::get_province_by_identifier(std::string const& identifier) const {
	if (!identifier.empty())
		for (Province const& province : provinces)
			if (province.identifier == identifier) return &province;
	return nullptr;
}

Province* Map::get_province_by_colour(Province::colour_t colour) {
	if (colour != Province::NULL_COLOUR)
		for (Province& province : provinces)
			if (province.colour == colour) return &province;
	return nullptr;
}

Province const* Map::get_province_by_colour(Province::colour_t colour) const {
	if (colour != Province::NULL_COLOUR)
		for (Province const& province : provinces)
			if (province.colour == colour) return &province;
	return nullptr;
}
