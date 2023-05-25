#pragma once

#include <godot_cpp/classes/image.hpp>

#include "openvic/Types.hpp"

#define ERR(x) ((x) == SUCCESS ? OK : FAILED)

namespace OpenVic {

	inline std::string godot_to_std_string(godot::String const& str) {
		return str.ascii().get_data();
	}

	inline godot::String std_to_godot_string(std::string const& str) {
		return str.c_str();
	}

	godot::Ref<godot::Image> load_godot_image(godot::String const& path);
}
