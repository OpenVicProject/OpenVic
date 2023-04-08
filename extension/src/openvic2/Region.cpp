#include "Map.hpp"

#include <cassert>
#include <algorithm>

using namespace OpenVic2;

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
