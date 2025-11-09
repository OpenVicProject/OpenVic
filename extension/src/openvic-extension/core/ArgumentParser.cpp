
#include "ArgumentParser.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/color_names.inc.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_float64_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector4_array.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/transform2d.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector4i.hpp>

#include <openvic-simulation/utility/Typedefs.hpp>

#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/utility/Utilities.hpp"

#include "gen/commit_info.gen.hpp"
#include "gen/license_info.gen.hpp"

using namespace OpenVic;
using namespace godot;

ArgumentOption::ArgumentOption(
	StringName const& name, Variant const& default_value, String const& description, PackedStringArray const& aliases,
	String option_args
)
	: name { name }, default_value { default_value }, description { description }, aliases(aliases),
	  option_arguments { option_args } {
	aliases_set.reserve(aliases.size());
	for (String const& alias : aliases) {
		aliases_set.insert(alias);
	}
}

StringName ArgumentOption::get_name() const {
	return name;
}

String ArgumentOption::get_option_arguments() const {
	return option_arguments;
}

PackedStringArray ArgumentOption::get_aliases() const {
	return aliases;
}

bool ArgumentOption::has_alias(String const& p_alias) const {
	return aliases_set.has(p_alias);
}

Variant::Type ArgumentOption::get_type() const {
	return default_value.get_type();
}

Variant ArgumentOption::get_default_value() const {
	return default_value;
}

String ArgumentOption::get_description() const {
	return description;
}

String ArgumentOption::get_help_string(bool p_is_rich) const {
	static constexpr int64_t NAME_PADDING = 45;
	static constexpr int64_t TYPE_PADDING = 45;

	String alias_list;
	for (size_t i = 0; i < aliases.size(); i++) {
		alias_list += ", -";
		if (aliases[i].length() > 1) {
			alias_list += "-";
		}
		alias_list += aliases[i];
	}

	String help_name = "--" + name + alias_list;
	if (!option_arguments.is_empty()) {
		help_name += " " + option_arguments;
	}
	help_name = help_name.rpad(NAME_PADDING);

	if (p_is_rich) {
		int open_index = -1;
		do {
			open_index = option_arguments.find("[", open_index + 1);
			if (open_index == -1) {
				break;
			}

			int close_index = option_arguments.find("]", open_index + 1);
			if (close_index == -1) {
				break;
			}

			String result = option_arguments.substr(open_index, close_index - open_index + 1);
			help_name = help_name.replace(result, vformat("[color=cyan]%s[/color]", result));
		} while (true);

		help_name = vformat("[color=green]%s[/color]", help_name.replace("<", "[color=magenta]<").replace(">", ">[/color]"));
	}

	const Variant::Type type = get_type();

	String help_type;
	if (type == Variant::STRING || type == Variant::STRING_NAME && default_value == Variant()) {
		help_type = vformat("Type: %s", Variant::get_type_name(get_type())).rpad(TYPE_PADDING);
	} else {
		help_type = vformat("Type: %s - Default Value: %s", Variant::get_type_name(type), default_value).rpad(TYPE_PADDING);
	}

	return vformat("%s%s%s", help_name, help_type, description);
}

Variant ArgumentOption::_get_empty_value_for(Variant::Type p_type) {
	switch (p_type) {
		using enum Variant::Type;
	case NIL:				   return nullptr;
	case BOOL:				   return false;
	case INT:				   return 0;
	case FLOAT:				   return 0.0f;
	case STRING:			   return String();
	case VECTOR2:			   return Vector2();
	case VECTOR2I:			   return Vector2i();
	case RECT2:				   return Rect2();
	case RECT2I:			   return Rect2i();
	case VECTOR3:			   return Vector3();
	case VECTOR3I:			   return Vector3i();
	case TRANSFORM2D:		   return Transform2D();
	case VECTOR4:			   return Vector4();
	case VECTOR4I:			   return Vector4i();
	case PLANE:				   return Plane();
	case QUATERNION:		   return Quaternion();
	case AABB:				   return godot::AABB();
	case BASIS:				   return Basis();
	case TRANSFORM3D:		   return Transform3D();
	case PROJECTION:		   return Projection();
	case COLOR:				   return Color();
	case STRING_NAME:		   return StringName();
	case NODE_PATH:			   return NodePath();
	case RID:				   return godot::RID();
	case DICTIONARY:		   return Dictionary();
	case ARRAY:				   return Array();
	case PACKED_BYTE_ARRAY:	   return PackedByteArray();
	case PACKED_INT32_ARRAY:   return PackedInt32Array();
	case PACKED_INT64_ARRAY:   return PackedInt64Array();
	case PACKED_FLOAT32_ARRAY: return PackedFloat32Array();
	case PACKED_FLOAT64_ARRAY: return PackedFloat64Array();
	case PACKED_STRING_ARRAY:  return PackedStringArray();
	case PACKED_VECTOR2_ARRAY: return PackedVector2Array();
	case PACKED_VECTOR3_ARRAY: return PackedVector3Array();
	case PACKED_COLOR_ARRAY:   return PackedColorArray();
	case PACKED_VECTOR4_ARRAY: return PackedVector4Array();
	default:				   ERR_FAIL_V_MSG(nullptr, vformat("Empty value not supported for %s.", Variant::get_type_name(p_type)));
	}
}

Ref<ArgumentOption> ArgumentOption::create(
	StringName const& p_name, Variant::Type p_type, String const& p_description, PackedStringArray const& p_aliases,
	String p_option_args
) {
	return create_with_default(p_name, _get_empty_value_for(p_type), p_description, p_aliases, p_option_args);
}

Ref<ArgumentOption> ArgumentOption::create_with_default(
	StringName const& p_name, Variant const& p_default, String const& p_description, PackedStringArray const& p_aliases,
	String p_option_args
) {
	return memnew(ArgumentOption(p_name, p_default, p_description, p_aliases, p_option_args));
}

void ArgumentOption::_bind_methods() {
	OV_BIND_METHOD(ArgumentOption::get_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "name"), "", "get_name");

	OV_BIND_METHOD(ArgumentOption::get_option_arguments);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "option_arguments"), "", "get_option_arguments");

	OV_BIND_METHOD(ArgumentOption::get_aliases);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "aliases"), "", "get_aliases");

	OV_BIND_METHOD(ArgumentOption::has_alias, { "alias" });

	OV_BIND_METHOD(ArgumentOption::get_type);
	ADD_PROPERTY(
		PropertyInfo(
			Variant::INT, "type", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_CLASS_IS_ENUM, "Variant.Type"
		),
		"", "get_type"
	);

	OV_BIND_METHOD(ArgumentOption::get_default_value);
	ADD_PROPERTY(
		PropertyInfo(
			Variant::NIL, "default_value", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT
		),
		"", "get_default_value"
	);

	OV_BIND_METHOD(ArgumentOption::get_description);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description"), "", "get_description");

	OV_BIND_METHOD(ArgumentOption::get_help_string, { "is_rich" });
}

ArgumentParser::ArgumentParser() {
	singleton = this;

	options.push_back(ArgumentOption::create("help", Variant::BOOL, "Displays help and quits.", { "h" }));
	options.push_back(
		ArgumentOption::create("game-debug", Variant::BOOL, "Start the game in debug mode.", { "d", "debug", "debug-mode" })
	);
	options.push_back(
		ArgumentOption::create("base-path", Variant::STRING, "Load Victoria 2 assets from a specific path.", { "b" }, "[path]")
	);
	options.push_back(ArgumentOption::create(
		"search-path", Variant::STRING, "Search for Victoria 2 assets at a specific path.", { "s" }, "[path]"
	));
	options.push_back(ArgumentOption::create("mod", Variant::PACKED_STRING_ARRAY, "Load Victoria 2 mods.", { "m" }, "<mods>"));

	parse_arguments(OS::get_singleton()->get_cmdline_args(), false);
	parse_arguments(OS::get_singleton()->get_cmdline_user_args());

	Dictionary arguments = Utilities::get_project_setting(OV_INAME("openvic/data/arguments"), Dictionary());
	if (!arguments.is_empty()) {
		Array keys = arguments.keys();
		for (size_t i = 0; i < arguments.size(); i++) {
			set_option_value(keys[i], arguments[keys[i]]);
		}
	}

	if (get_option_value("help")) {
		print_line_rich(get_help(true));
	}
}

ArgumentParser::~ArgumentParser() {
	singleton = nullptr;
}

bool ArgumentParser::has_option(StringName const& p_arg_name, bool p_include_aliases) const {
	for (Ref<ArgumentOption> const& option : options) {
		if (option->get_name() == p_arg_name) {
			return true;
		} else if (p_include_aliases && option->has_alias(p_arg_name)) {
			return true;
		}
	}
	return false;
}

bool ArgumentParser::is_option_set(StringName const& p_arg_name, bool p_include_aliases) const {
	if (arguments.has(p_arg_name)) {
		return true;
	}

	if (p_include_aliases) {
		decltype(options)::Iterator it = const_cast<ArgumentParser*>(this)->_find_option(p_arg_name);
		if (it == const_cast<ArgumentParser*>(this)->options.end()) {
			return false;
		}
		return arguments.has(it->ptr()->get_name());
	}

	return false;
}

Variant ArgumentParser::get_option_value(StringName const& p_arg_name) const {
	decltype(arguments)::ConstIterator it = arguments.find(p_arg_name);
	if (it == arguments.end()) {
		// Godot doesn't support iterators that can convert to const iterators in most containers
		// Guaranteed to never violate const
		// See https://github.com/godotengine/godot-proposals/issues/13036
		decltype(options)::Iterator it = const_cast<ArgumentParser*>(this)->_find_option(p_arg_name);
		ERR_FAIL_COND_V_MSG(
			it == const_cast<ArgumentParser*>(this)->options.end(), Variant(),
			vformat("Could not find valid option '%s'.", p_arg_name)
		);

		return it->ptr()->get_default_value();
	}
	return it->value;
}

void ArgumentParser::set_option_value(StringName const& p_arg_name, Variant p_value) {
	decltype(options)::Iterator it = _find_option(p_arg_name);

	ERR_FAIL_COND_MSG(it == options.end(), vformat("No option name/alias found for '%s'.", p_arg_name));
	ERR_FAIL_COND_MSG(
		it->ptr()->get_type() != p_value.get_type(),
		vformat("Cannot set value of '%s' to type of '%s'", p_arg_name, Variant::get_type_name(p_value.get_type()))
	);

	arguments[it->ptr()->get_name()] = p_value;
}

String ArgumentParser::get_help(bool p_is_rich) const {
	if (!p_is_rich && !_help_string.is_empty()) {
		return _help_string;
	}

	if (p_is_rich && !_rich_help_string.is_empty()) {
		return _rich_help_string;
	}

	const StringName APP_NAME_PATH = OV_INAME("application/config/name");
	const StringName APP_DESCRIPTION_PATH = OV_INAME("application/config/description");

	PackedStringArray option_help;
	option_help.resize(options.size());
	for (size_t i = 0; String & help : option_help) {
		help = "  " + options[i]->get_help_string(p_is_rich);
		i++;
	}

	String app_name = ProjectSettings::get_singleton()->get_setting_with_override(APP_NAME_PATH);
	if (p_is_rich) {
		app_name = vformat("[color=#00afff]%s[/color]", app_name);
	}

	String app_website = "https://openvic.com";
	if (p_is_rich) {
		app_website = vformat("[u]%s[/u]", app_website);
	}

	String app_description = ProjectSettings::get_singleton()->get_setting_with_override(APP_DESCRIPTION_PATH);
	if (p_is_rich) {
		app_description = vformat("[color=gray]%s[/color]", app_description);
	}

	String app_copyright =
		"(c) " + Utilities::std_to_godot_string(GAME_COPYRIGHT_INFO[0].parts.front().copyright_statements.front());
	if (p_is_rich) {
		app_copyright = vformat("[color=gray]%s[/color]", app_copyright);
	}

	String usage = "Usage:";
	if (p_is_rich) {
		usage = vformat("[b][color=yellow]%s[/color][/b]", usage);
	}

	String options_arg_str = "[options]";
	if (p_is_rich) {
		options_arg_str = vformat("[b][color=cyan]%s[/color][/b]", options_arg_str);
	}

	String options_category_str = "Options:";
	if (p_is_rich) {
		options_category_str = vformat("[b][color=yellow]%s[/color][/b]", options_category_str);
	}

	godot::String help = vformat(
		"%s - %s - %s - %s\n%s\n%s\n\n%s\n  %s -- %s\n\n%s\n%s", //
		app_name, //
		Utilities::std_to_godot_string(GAME_TAG), //
		Utilities::std_to_godot_string(GAME_COMMIT_HASH).substr(0, 7), //
		app_website, //
		app_description, //
		app_copyright, //
		usage,
		OS::get_singleton()->get_executable_path().get_file(), //
		options_arg_str, //
		options_category_str, //
		String("\n").join(option_help)
	);

	if (p_is_rich) {
		_rich_help_string = help;
		return _rich_help_string;
	}

	_help_string = help;
	return _help_string;
}

decltype(ArgumentParser::options)::Iterator ArgumentParser::_find_option(godot::String const& p_arg_name) {
	for (decltype(options)::Iterator it = options.begin(); it != options.end(); ++it) {
		if (it->ptr()->get_name() == p_arg_name || it->ptr()->has_alias(p_arg_name)) {
			return it;
		}
	}

	return options.end();
}

Variant ArgumentParser::_parse_value( //
	StringName const& p_arg_name, String const& p_value_string, ArgumentOption const* p_option
) {
	if (p_option == nullptr) {
		decltype(options)::Iterator it = _find_option(p_arg_name);
		ERR_FAIL_COND_V_EDMSG(
			it == options.end(), Variant(), vformat("Could not find option name/alias for '%s'.", p_arg_name)
		);
		p_option = it->ptr();
	}

	ERR_FAIL_NULL_V_MSG(p_option, Variant(), vformat("No option name/alias can be set for '%s'", p_arg_name));

	switch (Variant::Type type = p_option->get_type(); type) {
		using enum Variant::Type;
	case NIL:  return Variant();
	case BOOL: {
		String v = p_value_string.to_lower();
		if (v == "1" || v == "t" || v == "y" || v == "yes" || v == "true") {
			return true;
		} else if (v == "0" || v == "f" || v == "n" || v == "no" || v == "false") {
			return false;
		}

		ERR_FAIL_V_EDMSG(
			Variant(), vformat("'%s' must be a valid boolean, '%s' is an invalid value.", p_arg_name, p_value_string)
		);
	}
	case INT:
		if (p_value_string.is_valid_int()) {
			return p_value_string.to_int();
		}
		ERR_FAIL_V_EDMSG(
			Variant(), vformat("'%s' must be a valid integer, '%s' is an invalid value.", p_arg_name, p_value_string)
		);
	case FLOAT:
		if (p_value_string.is_valid_float()) {
			return p_value_string.to_float();
		}
		ERR_FAIL_V_EDMSG(
			Variant(), vformat("'%s' must be a valid float, '%s' is an invalid value.", p_arg_name, p_value_string)
		);
	case STRING:
	case STRING_NAME: //
		return p_value_string;

	case VECTOR2:
	case VECTOR2I:
	case RECT2:
	case RECT2I:
	case VECTOR3:
	case VECTOR3I:
	case VECTOR4:
	case VECTOR4I: {
		PackedStringArray array = p_value_string.lstrip("(").rstrip(")").split(",", false);
		switch (type) {
		case VECTOR2:
		case VECTOR2I:
			ERR_FAIL_COND_V_EDMSG(
				array.size() == 2, Variant(),
				vformat(
					"'%s' must be a valid %s, '%s' is an invalid value.", p_arg_name, Variant::get_type_name(type),
					p_value_string
				)
			);

		case RECT2:
		case RECT2I:
		case VECTOR4:
		case VECTOR4I:
			ERR_FAIL_COND_V_EDMSG(
				array.size() == 4, Variant(),
				vformat(
					"'%s' must be a valid %s, '%s' is an invalid value.", p_arg_name, Variant::get_type_name(type),
					p_value_string
				)
			);

		case VECTOR3:
		case VECTOR3I:
			ERR_FAIL_COND_V_EDMSG(
				array.size() == 3, Variant(),
				vformat(
					"'%s' must be a valid %s, '%s' is an invalid value.", p_arg_name, Variant::get_type_name(type),
					p_value_string
				)
			);

		default: unreachable();
		}
		return UtilityFunctions::str_to_var(vformat("%s(%s)", String(",").join(array)));
	}
	case COLOR: {
		if (Color::html_is_valid(p_value_string)) {
			return Color::html(p_value_string);
		} else if (int index = Color::find_named_color(p_value_string); index != -1) {
			return named_colors[index].color;
		} else {
			ERR_FAIL_V_EDMSG(
				Variant(), vformat("'%s' must be a valid Color, '%s' is an invalid value.", p_arg_name, p_value_string)
			);
		}
	}
	case PACKED_STRING_ARRAY: {
		Variant* arg = arguments.getptr(p_option->get_name());
		if (arg != nullptr) {
			(arg->operator PackedStringArray()).append(p_value_string);
			return *arg;
		} else {
			return PackedStringArray { p_value_string };
		}
	}
	default:
		ERR_FAIL_V_EDMSG(
			Variant(),
			vformat("'%s' value of type '%s' requested but could not be parsed.", p_arg_name, Variant::get_type_name(type))
		);
	}
}

Error ArgumentParser::parse_arguments(PackedStringArray const& p_args, bool p_error_unknown) {
	String key;
	ArgumentOption const* option = nullptr;
	for (String const& arg : p_args) {
		const bool arg_begins_with_minus = arg.begins_with("-");

		if (option != nullptr) {
			if (!arg_begins_with_minus) {
				Variant value = _parse_value(key, arg, option);
				if (value != Variant()) {
					arguments[option->get_name()] = value;
				}
				option = nullptr;
				continue;
			} else {
				WARN_PRINT(vformat("Valid argument '%s' was not set as a value, skipping.", key));
			}
		}

		if (arg_begins_with_minus) {
			String arg_name = arg.substr(1);
			if (arg_name.length() > 1 && arg_name[0] != U'-' and arg_name[1] != U'=') {
				for (size_t i = 0; i < arg_name.length(); i++) {
					char32_t const& c = arg_name[i];
					if (!((c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z'))) {
						break;
					}

					decltype(options)::Iterator it = _find_option(arg_name);
					if (it == options.end()) {
						WARN_PRINT(vformat("Shorthand alias '%s' not found, skipping.", c));
						continue;
					}

					if (it->ptr()->get_type() == Variant::BOOL) {
						arguments[option->get_name()] = true;
					} else {
						WARN_PRINT(vformat("Shorthand alias '%s' is not a boolean type, skipping.", c));
					}
				}
				continue;
			}

			size_t equal_index = arg_name.find("=");
			String value;
			if (equal_index != -1) {
				key = arg_name.substr(0, equal_index);
				value = arg_name.substr(equal_index + 1);
			} else {
				key = arg_name;
			}

			if (key.length() > 2 && key.begins_with("-")) {
				key = key.substr(1);
			}

			decltype(options)::Iterator it = _find_option(key);
			if (it == options.end()) {
				if (p_error_unknown) {
					ERR_PRINT(vformat("Invalid argument '%s' found, skipping.", key));
				}
				continue;
			}
			option = it->ptr();

			if (equal_index != -1) {
				Variant arg_value = _parse_value(key, value, option);
				if (arg_value != Variant()) {
					arguments[option->get_name()] = arg_value;
					option = nullptr;
				}
			} else if (option->get_type() == Variant::BOOL) {
				arguments[option->get_name()] = true;
			} else {
				WARN_PRINT(vformat("Argument '%s' treated like a boolean but does not support a boolean value, skipping.", key)
				);
			}
		} else if (p_error_unknown) {
			ERR_PRINT(vformat("Unknown argument '%s' found, skipping.", arg));
		}
	}

	return OK;
}

void ArgumentParser::_bind_methods() {
	OV_BIND_METHOD(ArgumentParser::has_option, { "arg_name", "include_aliases" }, DEFVAL(false));
	OV_BIND_METHOD(ArgumentParser::is_option_set, { "arg_name", "include_aliases" }, DEFVAL(false));
	OV_BIND_METHOD(ArgumentParser::get_option_value, { "arg_name" });
	OV_BIND_METHOD(ArgumentParser::set_option_value, { "arg_name", "value" });
	OV_BIND_METHOD(ArgumentParser::get_help, { "is_rich" });
	OV_BIND_METHOD(ArgumentParser::parse_arguments, { "args", "error_unknown" }, DEFVAL(true));
}
