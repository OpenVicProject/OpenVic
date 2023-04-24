#include "Region.hpp"

#include <cassert>

using namespace OpenVic2;

size_t ProvinceSet::get_province_count() const {
	return provinces.size();
}

bool ProvinceSet::contains_province(Province const* province) const {
	return province && std::find(provinces.begin(), provinces.end(), province) != provinces.end();
}

std::set<Province*> const& ProvinceSet::get_provinces() const {
	return provinces;
}

Region::Region(std::string const& new_identifier) : HasIdentifier { new_identifier } {}

colour_t Region::get_colour() const {
	if (provinces.empty()) return 0xFF0000;
	return (*provinces.cbegin())->get_colour();
}
