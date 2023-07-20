extends Node

signal resolution_added(value : Vector2i)

const error_resolution : Vector2i = Vector2i(-1,-1)

@export
var minimum_resolution : Vector2i = Vector2i(1,1)

# REQUIREMENTS:
# * SS-130, SS-131, SS-132, SS-133
const _starting_resolutions : Array[Vector2i] = [
	Vector2i(3840,2160),
	Vector2i(2560,1080),
	Vector2i(1920,1080),
	Vector2i(1366,768),
	Vector2i(1536,864),
	Vector2i(1280,720),
	Vector2i(1440,900),
	Vector2i(1600,900),
	Vector2i(1024,600),
	Vector2i(800,600)
]

var _resolutions : Array[Vector2i]

const _regex_pattern : String = "(\\d+)\\s*[xX,]\\s*(\\d+)"
var _regex : RegEx

func _ready():
	assert(minimum_resolution.x > 0 and minimum_resolution.y > 0, "Minimum resolution must be positive!")
	for resolution_value in _starting_resolutions:
		add_resolution(resolution_value)
	assert(not _resolutions.is_empty(), "No valid starting resolutions!")

	_regex = RegEx.new()
	var err := _regex.compile(_regex_pattern)
	assert(err == OK, "Resolution RegEx failed to compile!")

func has_resolution(resolution_value : Vector2i) -> bool:
	return resolution_value in _resolutions

func add_resolution(resolution_value : Vector2i) -> bool:
	if has_resolution(resolution_value): return true
	if resolution_value.x < minimum_resolution.x or resolution_value.y < minimum_resolution.y:
		push_error("Resolution %dx%d is smaller than minimum (%dx%d)" % [resolution_value.x, resolution_value.y, minimum_resolution.x, minimum_resolution.y])
		return false
	_resolutions.append(resolution_value)
	resolution_added.emit(resolution_value)
	return true

func get_resolution_value_list() -> Array[Vector2i]:
	var list : Array[Vector2i] = []
	# Return a sorted copy instead of a reference to the private array
	list.append_array(_resolutions)
	list.sort_custom(func(a, b): return a > b)
	return list

func get_resolution_value_from_string(resolution_string : String) -> Vector2i:
	if not resolution_string.is_empty():
		var result := _regex.search(resolution_string)
		if result: return Vector2i(result.get_string(1).to_int(), result.get_string(2).to_int())
	return error_resolution

func get_current_resolution() -> Vector2i:
	var viewport := get_viewport()
	if viewport != null:
		var window := viewport.get_window()
		if window != null:
			match window.mode:
				Window.MODE_EXCLUSIVE_FULLSCREEN, Window.MODE_FULLSCREEN:
					return window.content_scale_size
				_:
					return window.size
	push_error("Trying to get resolution before window exists!")
	return error_resolution

func set_resolution(resolution : Vector2i) -> void:
	if not has_resolution(resolution):
		push_warning("Setting resolution to non-standard value %sx%s" % [resolution.x, resolution.y])
	var viewport := get_viewport()
	if viewport != null:
		var window := viewport.get_window()
		if window != null:
			match window.mode:
				Window.MODE_EXCLUSIVE_FULLSCREEN, Window.MODE_FULLSCREEN:
					window.content_scale_size = resolution
				_:
					window.size = resolution
					window.content_scale_size = Vector2i(0,0)
			return
	push_error("Trying to set resolution before window exists!")

func get_current_window_mode() -> Window.Mode:
	var viewport := get_viewport()
	if viewport != null:
		var window := viewport.get_window()
		if window != null:
			return window.mode
	push_error("Trying to get window mode before it exists!")
	return Window.MODE_WINDOWED

func set_window_mode(mode : Window.Mode) -> void:
	var viewport := get_viewport()
	if viewport != null:
		var window := viewport.get_window()
		if window != null:
			var current_resolution := get_current_resolution()
			var current_monitor := window.current_screen
			window.mode = mode
			window.current_screen = current_monitor
			set_resolution(current_resolution)
			return
	push_error("Trying to set window mode before it exists!")

func get_current_monitor() -> int:
	var viewport := get_viewport()
	if viewport != null:
		var window := viewport.get_window()
		if window != null:
			return window.current_screen
	push_error("Trying to get monitor index before window exists!")
	return 0

func set_monitor(index : int) -> void:
	var viewport := get_viewport()
	if viewport != null:
		var window := viewport.get_window()
		if window != null:
			window.current_screen = index
			return
	push_error("Trying to set monitor index before window exists!")
