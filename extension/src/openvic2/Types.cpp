#include "Types.hpp"

#include <cassert>

using namespace OpenVic2;

HasIdentifier::HasIdentifier(std::string const& new_identifier) : identifier(new_identifier) {
	assert(!identifier.empty());
}

std::string const& HasIdentifier::get_identifier() const {
	return identifier;
}
