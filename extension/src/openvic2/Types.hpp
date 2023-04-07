#pragma once

namespace OpenVic2 {
	using return_t = bool;
	// This mirrors godot::Error, where `OK = 0` and `FAILED = 1`.
	static const return_t SUCCESS = false, FAILURE = true;
}
