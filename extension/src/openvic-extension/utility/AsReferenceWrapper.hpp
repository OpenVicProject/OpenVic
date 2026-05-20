#pragma once

#include <functional>
#include <type_traits>

namespace OpenVic {
	template<typename T>
	static constexpr std::reference_wrapper<std::add_const_t<T>> as_reference_wrapper(T const& x) {
		return x;
	}
	template<typename T>
	static constexpr std::reference_wrapper<std::add_const_t<T>> as_reference_wrapper(T const* const x) {
		return *x;
	}
}