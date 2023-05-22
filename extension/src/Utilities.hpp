#pragma once

#include <godot_cpp/variant/string.hpp>

#include "openvic/Types.hpp"

#define ERR(x) ((x) == SUCCESS ? OK : FAILED)

namespace OpenVic {

	inline char const* godot_to_c_string(godot::String const& str) {
		return str.ascii().get_data();
	}

	inline std::string godot_to_std_string(godot::String const& str) {
		return godot_to_c_string(str);
	}

	inline godot::String std_to_godot_string(std::string const& str) {
		return str.c_str();
	}
}
