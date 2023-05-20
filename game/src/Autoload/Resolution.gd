extends Node

signal resolution_added(value : Vector2i, name : StringName, display_name : StringName)
signal resolution_changed(value : Vector2i)
signal window_mode_changed(value : Window.Mode)

const error_resolution : Vector2i = Vector2i(-1,-1)

@export
var minimum_resolution : Vector2i = Vector2i(1,1)

const _starting_resolutions : Dictionary = {
	Vector2i(3840,2160): &"4K",
	Vector2i(2560,1080): &"UW1080p",
	Vector2i(1920,1080): &"1080p",
	Vector2i(1366,768) : &"",
	Vector2i(1536,864) : &"",
	Vector2i(1280,720) : &"720p",
	Vector2i(1440,900) : &"",
	Vector2i(1600,900) : &"",
	Vector2i(1024,600) : &"",
	Vector2i(800,600)  : &""
}

var _resolutions : Dictionary

const _regex_pattern : String = "(\\d+)\\s*[xX,]\\s*(\\d+)"
var _regex : RegEx

func _ready():
	assert(minimum_resolution.x > 0 and minimum_resolution.y > 0, "Minimum resolution must be positive!")
	for resolution_value in _starting_resolutions:
		add_resolution(resolution_value, _starting_resolutions[resolution_value])
	assert(not _resolutions.is_empty(), "No valid starting resolutions!")

	_regex = RegEx.new()
	var err := _regex.compile(_regex_pattern)
	assert(err == OK, "Resolution RegEx failed to compile!")


func has_resolution(resolution_value : Vector2i) -> bool:
	return resolution_value in _resolutions

func add_resolution(resolution_value : Vector2i, resolution_name : StringName = &"") -> bool:
	if has_resolution(resolution_value): return true
	var res_dict := { value = resolution_value, name = &"" }
	var display_name := "%sx%s" % [resolution_value.x, resolution_value.y]
	if not resolution_name.is_empty():
		res_dict.name = resolution_name
		display_name = "%s (%s)" % [display_name, resolution_name]
	res_dict.display_name = StringName(display_name)
	if resolution_value.x < minimum_resolution.x or resolution_value.y < minimum_resolution.y:
		push_error("Resolution %s is smaller than minimum (%sx%s)" % [res_dict.display_name, minimum_resolution.x, minimum_resolution.y])
		return false
	resolution_added.emit(resolution_value, resolution_name, display_name)
	_resolutions[resolution_value] = res_dict
	return true

func get_resolution_value_list() -> Array:
	var list := _resolutions.keys()
	list.sort_custom(func(a, b): return a > b)
	return list

func get_resolution_name(resolution_value : Vector2i) -> StringName:
	return _resolutions.get(resolution_value, { name = &"unknown resolution" }).name

func get_resolution_display_name(resolution_value : Vector2i) -> StringName:
	return _resolutions.get(resolution_value, { display_name = &"unknown resolution" }).display_name

func get_resolution_value_from_string(resolution_string : String) -> Vector2i:
	if not resolution_string.is_empty():
		for resolution in _resolutions.values():
			if resolution_string == resolution.name or resolution_string == resolution.display_name:
				return resolution.value
		var result := _regex.search(resolution_string)
		if result: return Vector2i(result.get_string(1).to_int(), result.get_string(2).to_int())
	return error_resolution

func get_current_resolution() -> Vector2i:
	var window := get_viewport().get_window()
	match window.mode:
		Window.MODE_EXCLUSIVE_FULLSCREEN, Window.MODE_FULLSCREEN:
			return window.content_scale_size
		_:
			return window.size

func set_resolution(resolution : Vector2i) -> void:
	if not has_resolution(resolution):
		push_warning("Setting resolution to non-standard value %sx%s" % [resolution.x, resolution.y])
	var window := get_viewport().get_window()
	if get_current_resolution() != resolution:
		resolution_changed.emit(resolution)
	match window.mode:
		Window.MODE_EXCLUSIVE_FULLSCREEN, Window.MODE_FULLSCREEN:
			window.content_scale_size = resolution
		_:
			window.size = resolution
			window.content_scale_size = Vector2i(0,0)

func reset_resolution() -> void:
	set_resolution(get_current_resolution())
