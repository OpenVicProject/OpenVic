#pragma once

#include <concepts>
#include <string>
#include <string_view>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/variant/char_string.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <openvic-simulation/core/template/Concepts.hpp>
#include <openvic-simulation/types/Colour.hpp>
#include <openvic-simulation/types/Vector.hpp>
#include <openvic-simulation/utility/Containers.hpp>

namespace OpenVic {
	template<typename To, typename From>
	To convert_to(From const& from) = delete;

	template<any_of<std::string, memory::string> To, any_of<godot::String, godot::StringName> From>
	[[nodiscard]] _FORCE_INLINE_ To convert_to(From const& from) {
		godot::CharString utf8 = static_cast<godot::String>(from).utf8();
		return { utf8.ptr(), static_cast<size_t>(utf8.length()) };
	}

	template<any_of<godot::String, godot::StringName> To, std::convertible_to<std::string_view> From>
	[[nodiscard]] _FORCE_INLINE_ To convert_to(From const& from) {
		if constexpr (std::same_as<From, std::string_view>) {
			return godot::String::utf8(from.data(), from.length());
		} else {
			return convert_to<godot::String>(static_cast<std::string_view>(from));
		}
	}

	template<IsColour To>
	[[nodiscard]] _FORCE_INLINE_ To convert_to(godot::Color const& from) {
		To result = To::from_floats(from.r, from.g, from.b);
		if constexpr (To::colour_traits::has_alpha) {
			result = result.with_alpha(from.a);
		}
		return result;
	}

	template<std::same_as<godot::Color> To, IsColour From>
	[[nodiscard]] _FORCE_INLINE_ To convert_to(From const& from) {
		return { from.redf(), from.greenf(), from.bluef(), from.alphaf() };
	}

	template<>
	[[nodiscard]] _FORCE_INLINE_ ivec2_t convert_to(godot::Vector2i const& from) {
		return { from.x, from.y };
	}

	template<>
	[[nodiscard]] _FORCE_INLINE_ ivec2_t convert_to(godot::Vector2 const& from) {
		return { static_cast<int32_t>(from.x), static_cast<int32_t>(from.y) };
	}

	template<>
	[[nodiscard]] _FORCE_INLINE_ godot::Vector2i convert_to(ivec2_t const& from) {
		return { from.x, from.y };
	}

	template<>
	[[nodiscard]] _FORCE_INLINE_ godot::Vector2i convert_to(fvec2_t const& from) {
		return { from.x.round<int32_t>(), from.y.round<int32_t>() };
	}

	template<>
	[[nodiscard]] _FORCE_INLINE_ godot::Vector2 convert_to(ivec2_t const& from) {
		return { static_cast<real_t>(from.x), static_cast<real_t>(from.y) };
	}

	template<>
	[[nodiscard]] _FORCE_INLINE_ godot::Vector2 convert_to(fvec2_t const& from) {
		return { static_cast<real_t>(from.x), static_cast<real_t>(from.y) };
	}
}
