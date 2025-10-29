#pragma once

#include <concepts>
#include <initializer_list>
#include <string_view>
#include <type_traits>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include "openvic-extension/utility/StringLiteral.hpp"

namespace godot {
	class Object;
}

#define OV_BIND_METHOD(Function, ...) \
	::OpenVic::detail::bind_method<::OpenVic::detail::get_function_name<#Function>()>(&Function __VA_OPT__(, ) __VA_ARGS__)

#define OV_BIND_SMETHOD(Function, ...) \
	::OpenVic::detail::bind_static_method<::OpenVic::detail::get_function_name<#Function>()>( \
		get_class_static(), &Function __VA_OPT__(, ) __VA_ARGS__ \
	)

#define OV_BIND_SMETHOD_T(ClassType, Function, ...) \
	::OpenVic::detail::bind_static_method<ClassType, ::OpenVic::detail::get_function_name<#Function>()>( \
		&Function __VA_OPT__(, ) __VA_ARGS__ \
	)

namespace OpenVic::detail {
	template<typename Func>
	concept IsFunctionPointer = std::is_function_v<std::remove_pointer_t<Func>>;
	template<typename Func>
	concept IsMemberFunctionPointer = std::is_member_function_pointer_v<Func>;

	template<StringLiteral In>
	consteval auto get_function_name() {
		constexpr auto result = [] {
			constexpr auto prefix = std::string_view { "::" };

			constexpr std::string_view in_sv = In;
			constexpr auto start = in_sv.find_last_of(prefix);

			if constexpr (start == std::string_view::npos) {
				return In;
			} else {
				constexpr auto result = in_sv.substr(start + 1);
				return StringLiteral<result.size()> { result };
			}
		}();

		return result;
	}

	template<StringLiteral Name, IsMemberFunctionPointer Func, typename... DefaultsT>
	void bind_method(Func func, std::initializer_list<godot::StringName> arg_names, DefaultsT&&... defaults) {
		godot::MethodDefinition definition { Name.data() };
		definition.args = { arg_names };
		godot::ClassDB::bind_method(definition, func, defaults...);
	}

	template<StringLiteral Name, IsMemberFunctionPointer Func, typename... DefaultsT>
	void bind_method(Func func, DefaultsT&&... defaults) {
		bind_method<Name, Func>(func, {}, defaults...);
	}

	template<StringLiteral Name, IsFunctionPointer Func, typename... DefaultsT>
	void bind_static_method(
		godot::StringName class_name, Func func, std::initializer_list<godot::StringName> arg_names, DefaultsT&&... defaults
	) {
		godot::MethodDefinition definition { Name.data() };
		definition.args = { arg_names };
		godot::ClassDB::bind_static_method(class_name, definition, func, defaults...);
	}

	template<StringLiteral Name, IsFunctionPointer Func, typename... DefaultsT>
	void bind_static_method(godot::StringName class_name, Func func, DefaultsT&&... defaults) {
		bind_static_method<Name>(class_name, func, {}, defaults...);
	}

	template<std::derived_from<godot::Object> ClassT, StringLiteral Name, IsFunctionPointer Func, typename... DefaultsT>
	void bind_static_method(Func func, std::initializer_list<godot::StringName> arg_names, DefaultsT&&... defaults) {
		bind_static_method<Name>(ClassT::get_class_static(), func, arg_names, defaults...);
	}

	template<std::derived_from<godot::Object> ClassT, StringLiteral Name, IsFunctionPointer Func, typename... DefaultsT>
	void bind_static_method(Func func, DefaultsT&&... defaults) {
		bind_static_method<ClassT, Name>(func, {}, defaults...);
	}
}
