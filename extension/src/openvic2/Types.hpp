#pragma once

#include <string>
#include <vector>

#include "openvic2/Logger.hpp"

namespace OpenVic2 {
	using return_t = bool;
	// This mirrors godot::Error, where `OK = 0` and `FAILED = 1`.
	static constexpr return_t SUCCESS = false, FAILURE = true;

	class HasIdentifier {
		std::string identifier;
	protected:
		HasIdentifier(std::string const& new_identifier);
	public:
		std::string const& get_identifier() const;
	};
}
