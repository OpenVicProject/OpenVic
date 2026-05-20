#pragma once

#include <type_safe/strong_typedef.hpp>

#include <openvic-simulation/core/template/Concepts.hpp>

namespace OpenVic {
	template<is_strongly_typed T>
	static constexpr float as_float(T const& x) {
		return static_cast<float>(type_safe::get(x));
	}

	template<typename T>
	requires (!is_strongly_typed<T>)
	static constexpr float as_float(T const& x) {
		return static_cast<float>(x);
	}
}