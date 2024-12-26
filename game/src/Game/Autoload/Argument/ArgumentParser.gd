@tool
extends Node

const argument_setting_path := &"openvic/data/arguments"

@export var option_array: Array[ArgumentOption] = [
	ArgumentOption.new("help", TYPE_BOOL, "Displays help and quits.", false).add_alias(&"h")
]

const color_name_array: PackedStringArray = [
	"aliceblue",
	"antiquewhite",
	"aqua",
	"aquamarine",
	"azure",
	"beige",
	"bisque",
	"black",
	"blanchedalmond",
	"blue",
	"blueviolet",
	"brown",
	"burlywood",
	"cadetblue",
	"chartreuse",
	"chocolate",
	"coral",
	"cornflower",
	"cornsilk",
	"crimson",
	"cyan",
	"darkblue",
	"darkcyan",
	"darkgoldenrod",
	"darkgray",
	"darkgreen",
	"darkkhaki",
	"darkmagenta",
	"darkolivegreen",
	"darkorange",
	"darkorchid",
	"darkred",
	"darksalmon",
	"darkseagreen",
	"darkslateblue",
	"darkslategray",
	"darkturquoise",
	"darkviolet",
	"deeppink",
	"deepskyblue",
	"dimgray",
	"dodgerblue",
	"firebrick",
	"floralwhite",
	"forestgreen",
	"fuchsia",
	"gainsboro",
	"ghostwhite",
	"gold",
	"goldenrod",
	"gray",
	"green",
	"greenyellow",
	"honeydew",
	"hotpink",
	"indianred",
	"indigo",
	"ivory",
	"khaki",
	"lavender",
	"lavenderblush",
	"lawngreen",
	"lemonchiffon",
	"lightblue",
	"lightcoral",
	"lightcyan",
	"lightgoldenrod",
	"lightgray",
	"lightgreen",
	"lightpink",
	"lightsalmon",
	"lightseagreen",
	"lightskyblue",
	"lightslategray",
	"lightsteelblue",
	"lightyellow",
	"lime",
	"limegreen",
	"linen",
	"magenta",
	"maroon",
	"mediumaquamarine",
	"mediumblue",
	"mediumorchid",
	"mediumpurple",
	"mediumseagreen",
	"mediumslateblue",
	"mediumspringgreen",
	"mediumturquoise",
	"mediumvioletred",
	"midnightblue",
	"mintcream",
	"mistyrose",
	"moccasin",
	"navajowhite",
	"navyblue",
	"oldlace",
	"olive",
	"olivedrab",
	"orange",
	"orangered",
	"orchid",
	"palegoldenrod",
	"palegreen",
	"paleturquoise",
	"palevioletred",
	"papayawhip",
	"peachpuff",
	"peru",
	"pink",
	"plum",
	"powderblue",
	"purple",
	"rebeccapurple",
	"red",
	"rosybrown",
	"royalblue",
	"saddlebrown",
	"salmon",
	"sandybrown",
	"seagreen",
	"seashell",
	"sienna",
	"silver",
	"skyblue",
	"slateblue",
	"slategray",
	"snow",
	"springgreen",
	"steelblue",
	"tan",
	"teal",
	"thistle",
	"tomato",
	"transparent",
	"turquoise",
	"violet",
	"webgray",
	"webgreen",
	"webmaroon",
	"webpurple",
	"wheat",
	"white",
	"whitesmoke",
	"yellow",
	"yellowgreen"
]


func has_argument_support(arg_name: StringName, include_aliases: bool = false) -> bool:
	return option_array.any(
		func(opt: ArgumentOption) -> bool:
			if include_aliases and arg_name in opt.aliases:
				return true
			return opt.name == arg_name
	)


func get_argument(arg_name: StringName, default_val: Variant = null) -> Variant:
	if ProjectSettings.has_setting(argument_setting_path):
		var arg_setting: Variant = ProjectSettings.get_setting_with_override(argument_setting_path)
		if not arg_setting is Dictionary:
			push_error("Argument setting is not a dictionary.")
			return default_val
		return arg_setting.get(arg_name, default_val)
	else:
		return default_val


func set_argument(arg_name: StringName, value: Variant) -> void:
	if ProjectSettings.has_setting(argument_setting_path):
		var arg_setting: Variant = null
		arg_setting = ProjectSettings.get_setting_with_override(argument_setting_path)
		if not arg_setting is Dictionary:
			push_error("Argument setting is not a dictionary.")
			return
		arg_setting[arg_name] = value
		ProjectSettings.set_setting(argument_setting_path, arg_setting)
		return
	push_error("Argument setting has not been set yet.")


func _set_argument_setting() -> void:
	var argument_dictionary: Dictionary = {}
	if ProjectSettings.has_setting(argument_setting_path):
		argument_dictionary = ProjectSettings.get_setting_with_override(argument_setting_path)
	for option: ArgumentOption in option_array:
		argument_dictionary[option.name] = option.default_value

	_parse_argument_list(argument_dictionary, OS.get_cmdline_args())
	_parse_argument_list(argument_dictionary, OS.get_cmdline_user_args(), true)

	ProjectSettings.set_setting(argument_setting_path, argument_dictionary)


func _parse_value(arg_name: StringName, value_string: String, type: Variant.Type) -> Variant:
	match type:
		TYPE_NIL:
			return null
		TYPE_BOOL:
			value_string = value_string.to_lower()
			if (
				value_string == "true"
				or value_string == "t"
				or value_string == "yes"
				or value_string == "y"
			):
				return true
			if (
				value_string == "false"
				or value_string == "f"
				or value_string == "no"
				or value_string == "n"
			):
				return false
			push_error(
				"'%s' must be a valid boolean, '%s' is an invalid value." % [arg_name, value_string]
			)
			return null
		TYPE_INT:
			if value_string.is_valid_int():
				return value_string.to_int()
			push_error(
				"'%s' must be a valid integer, '%s' is an invalid value." % [arg_name, value_string]
			)
			return null
		TYPE_FLOAT:
			if value_string.is_valid_float():
				return value_string.to_float()
			push_error(
				"'%s' must be a valid float, '%s' is an invalid value." % [arg_name, value_string]
			)
			return null
		TYPE_STRING, TYPE_STRING_NAME, TYPE_PACKED_STRING_ARRAY:
			return value_string
		TYPE_COLOR:
			if Color.html_is_valid(value_string) or value_string.to_lower() in color_name_array:
				return Color.from_string(value_string, Color())
			push_error(
				(
					"'%s' must be an html Color or Color name, '%s' is an invalid value."
					% [arg_name, value_string]
				)
			)
			return null
		# Unsupported types
		TYPE_VECTOR2, TYPE_VECTOR2I, TYPE_VECTOR3, TYPE_VECTOR3I, TYPE_VECTOR4, TYPE_VECTOR4I, TYPE_RECT2, TYPE_RECT2I:
			push_warning("Value type '%s' may not be supported." % type)
			var data_array := value_string.lstrip("(").rstrip(")").split(",", false)
			for index: int in data_array.size():
				data_array[index] = " " + data_array[index].strip_edges()
			match type:
				TYPE_VECTOR2:
					if data_array.size() != 2:
						push_error(
							(
								"'%s' value must be a Vector2, '%s' is an invalid value."
								% [arg_name, value_string]
							)
						)
						return null
					return str_to_var("Vector2(%s )" % ",".join(data_array))
				TYPE_VECTOR2I:
					if data_array.size() != 2:
						push_error(
							(
								"'%s' value must be a Vector2i, '%s' is an invalid value."
								% [arg_name, value_string]
							)
						)
						return null
					return str_to_var("Vector2i(%s )" % ",".join(data_array))
				TYPE_VECTOR3:
					if data_array.size() != 2:
						push_error(
							(
								"'%s' value must be a Vector3, '%s' is an invalid value."
								% [arg_name, value_string]
							)
						)
						return null
					return str_to_var("Vector3(%s )" % ",".join(data_array))
				TYPE_VECTOR3I:
					if data_array.size() != 2:
						push_error(
							(
								"'%s' value must be a Vector3i, '%s' is an invalid value."
								% [arg_name, value_string]
							)
						)
						return null
					return str_to_var("Vector3i(%s )" % ",".join(data_array))
				TYPE_VECTOR4:
					if data_array.size() != 2:
						push_error(
							(
								"'%s' value must be a Vector4, '%s' is an invalid value."
								% [arg_name, value_string]
							)
						)
						return null
					return str_to_var("Vector4(%s )" % ",".join(data_array))
				TYPE_VECTOR4I:
					if data_array.size() != 2:
						push_error(
							(
								"'%s' value must be a Vector4i, '%s' is an invalid value."
								% [arg_name, value_string]
							)
						)
						return null
					return str_to_var("Vector4i(%s )" % ",".join(data_array))
				TYPE_RECT2:
					if data_array.size() != 2:
						push_error(
							(
								"'%s' value must be a Rect2, '%s' is an invalid value."
								% [arg_name, value_string]
							)
						)
						return null
					return str_to_var("Rect2(%s )" % ",".join(data_array))
				TYPE_RECT2I:
					if data_array.size() != 2:
						push_error(
							(
								"'%s' value must be a Rect2i, '%s' is an invalid value."
								% [arg_name, value_string]
							)
						)
						return null
					return str_to_var("Rect2i(%s )" % ",".join(data_array))
		_:
			push_error(
				"'%s' value of type '%s' requested but could not be parsed." % [arg_name, type]
			)
			return null

	return null


# Missing types
# TYPE_TRANSFORM2D = 11
# TYPE_VECTOR4 = 12
# TYPE_VECTOR4I = 13
# TYPE_PLANE = 14
# TYPE_QUATERNION = 15
# TYPE_AABB = 16
# TYPE_BASIS = 17
# TYPE_TRANSFORM3D = 18
# TYPE_PROJECTION = 19
# TYPE_NODE_PATH = 22
# TYPE_RID = 23
# TYPE_OBJECT = 24
# TYPE_CALLABLE = 25
# TYPE_SIGNAL = 26
# TYPE_DICTIONARY = 27
# TYPE_ARRAY = 28
# TYPE_PACKED_BYTE_ARRAY = 29
# TYPE_PACKED_INT32_ARRAY = 30
# TYPE_PACKED_INT64_ARRAY = 31
# TYPE_PACKED_FLOAT32_ARRAY = 32
# TYPE_PACKED_FLOAT64_ARRAY = 33
# TYPE_PACKED_VECTOR2_ARRAY = 35
# TYPE_PACKED_VECTOR3_ARRAY = 36
# TYPE_PACKED_COLOR_ARRAY = 37


func _add_argument(dictionary: Dictionary, option: ArgumentOption, argument: Variant) -> void:
	if option.type == TYPE_PACKED_STRING_ARRAY:
		dictionary[option.name] += PackedStringArray([argument])
	else:
		dictionary[option.name] = argument


func _parse_argument_list(
	dictionary: Dictionary, arg_list: PackedStringArray, is_game_args: bool = false
) -> Dictionary:
	var current_key: String = ""
	var current_option: ArgumentOption = null
	for arg: String in arg_list:
		if current_option != null:
			if not arg.begins_with("-"):
				var result: Variant = _parse_value(current_key, arg, current_option.type)
				if result != null:
					_add_argument(dictionary, current_option, result)
				current_option = null
				continue
			else:
				push_warning("Valid argument '%s' was not set as a value, skipping." % current_key)

		if arg.begins_with("-"):
			current_option = null
			arg = arg.substr(1)
			var key := &""
			var value := &""

			# Support for Unix shorthand of multiple boolean arguments
			# eg: "-abc" means a == true, b == true, c == true
			if arg.length() > 1 and arg[0] != "-" and arg[1] != "=":
				for c: String in arg:
					if not ((c >= "a" and c <= "z") or (c >= "A" and c <= "Z")):
						push_warning(
							(
								"Parsing shorthand alias containing '%s', perhaps you meant '--%s'? Skipping argument."
								% [c, arg]
							)
						)
						break
					var alias_found := false
					for o: ArgumentOption in option_array:
						if o.aliases.any(func(v: StringName) -> bool: return c == v):
							if o.type == TYPE_BOOL:
								dictionary[o.name] = true
							else:
								push_warning("Shorthand alias '%s is not boolean, skipping." % c)
								# TODO: Support POSIX 12.1 2.b?
								# https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
								break
							alias_found = true
							break
					if not alias_found:
						push_warning("Shorthand alias '%s' not found, skipping." % c)
				continue

			# Support for = key/value split
			# eg: "-v=5" and "--value=5" means v == 5 and value == 5
			var first_equal := arg.find("=")
			if first_equal > -1:
				key = arg.substr(0, first_equal)
				value = arg.substr(first_equal + 1)
			else:
				key = arg

			# Removes - for full name arguments
			if key.length() > 2 and key.begins_with("-"):
				key = key.substr(1)

			for o: ArgumentOption in option_array:
				if key == o.name or o.aliases.any(func(v: StringName) -> bool: return key == v):
					current_option = o
					break

			if current_option == null:
				push_warning("Invalid argument '%s' found, skipping." % key)
				continue

			current_key = key
			if first_equal > -1:
				var arg_result: Variant = _parse_value(key, value, current_option.type)
				if arg_result != null:
					_add_argument(dictionary, current_option, arg_result)
					current_option = null
			elif current_option.type == TYPE_BOOL:
				dictionary[current_option.name] = true
			else:
				push_warning(
					(
						"Argument '%s' treated like a boolean but does not support a boolean value, skipping."
						% key
					)
				)
		elif is_game_args:
			push_warning("Non-argument '%s' found, skipping." % arg)

	return dictionary


func _print_help() -> void:
	var project_name: StringName = ProjectSettings.get_setting_with_override(
		&"application/config/name"
	)
	var project_version: String = _GIT_INFO_.tag
	var project_hash: String = _GIT_INFO_.short_hash
	var project_website: String = "https://openvic.com"
	var project_description: String = ProjectSettings.get_setting_with_override(
		&"application/config/description"
	)
	print_rich(
		(
			"""
%s - %s - %s - %s
%s

%s

Options:
"""
			% [
				project_name,
				project_version,
				project_hash,
				project_website,
				project_description,
				"usage: %s [options]" % OS.get_executable_path().get_file()
			]
		)
	)
	for option: ArgumentOption in option_array:
		print_rich(
			(
				"  --%s%s%s"
				% [
					(
						(
							option.name
							+ (
								",-%s" % (",-".join(option.aliases))
								if option.aliases.size() > 0
								else ""
							)
						)
						. rpad(45)
					),
					(
						(
							"Type: %s - Default Value: %s"
							% [option.get_type_string(), option.default_value]
						)
						. rpad(45)
					),
					option.description
				]
			)
		)


func _ready() -> void:
	if Engine.is_editor_hint():
		return
	_set_argument_setting()
	GameDebug._singleton = GameDebug.new()
