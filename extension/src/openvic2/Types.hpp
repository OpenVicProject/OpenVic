#pragma once

#include <string>

namespace OpenVic2 {
	using return_t = bool;
	// This mirrors godot::Error, where `OK = 0` and `FAILED = 1`.
	static const return_t SUCCESS = false, FAILURE = true;

	struct HasIdentifier {
	private:
		std::string identifier;
	protected:
		HasIdentifier(std::string const& new_identifier);
	public:
		std::string const& get_identifier() const;
	};
}
