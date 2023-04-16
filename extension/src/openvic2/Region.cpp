#include "Map.hpp"

#include <cassert>
#include <algorithm>

using namespace OpenVic2;

Region::Region(std::string const& newIdentifier) : identifier(newIdentifier) {
	assert(!identifier.empty());
}

std::string const& Region::getIdentifier() const {
	return identifier;
}

size_t Region::getProvinceCount() const {
	return provinces.size();
}

bool Region::containsProvince(Province const* province) const {
	return province && std::find(provinces.begin(), provinces.end(), province) != provinces.end();
}

std::vector<Province*> const& Region::getProvinces() const {
	return provinces;
}
