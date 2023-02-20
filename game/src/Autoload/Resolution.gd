extends Node

var _resolutions : Array[Dictionary]= [
	{ "name": &"", "value": Vector2i(3840,2160) },
	{ "name": &"", "value": Vector2i(2560,1080) },
	{ "name": &"", "value": Vector2i(1920,1080) },
	{ "name": &"", "value": Vector2i(1366,768) },
	{ "name": &"", "value": Vector2i(1536,864) },
	{ "name": &"", "value": Vector2i(1280,720) },
	{ "name": &"", "value": Vector2i(1440,900) },
	{ "name": &"", "value": Vector2i(1600,900) },
	{ "name": &"", "value": Vector2i(1024,600) },
	{ "name": &"", "value": Vector2i(800,600) }
]

func _ready():
	for resolution in _resolutions:
		resolution["tag"] = _get_name_of_resolution(resolution["name"], resolution["value"])

func has_resolution(resolution_name : StringName) -> bool:
	return resolution_name in _resolutions

func get_resolution(resolution_name : StringName, default : Vector2i = Vector2i(1920, 1080)) -> Vector2i:
	var resolution := _get_resolution_by_name(resolution_name)
	if resolution.x < 0 and resolution.y < 0:
		return default
	return resolution

func get_resolution_name_list() -> Array[StringName]:
	var result : Array[StringName] = []
	for resolution in _resolutions:
		result.append(resolution["tag"])
	return result

func get_current_resolution() -> Vector2i:
	var window := get_viewport().get_window()
	match window.mode:
		Window.MODE_EXCLUSIVE_FULLSCREEN, Window.MODE_FULLSCREEN:
			return window.content_scale_size
		_:
			return window.size

func set_resolution(resolution : Vector2i) -> void:
	var window := get_viewport().get_window()
	match window.mode:
		Window.MODE_EXCLUSIVE_FULLSCREEN, Window.MODE_FULLSCREEN:
			window.content_scale_size = resolution
		_:
			window.size = resolution
			window.content_scale_size = Vector2i(0,0)

func reset_resolution() -> void:
	set_resolution(get_current_resolution())

func _get_name_of_resolution(resolution_name : StringName, resolution_value : Vector2i) -> StringName:
	if resolution_name != null and not resolution_name.is_empty():
		return "%s (%sx%s)" % [resolution_name, resolution_value.x, resolution_value.y]
	return "%sx%s" % [resolution_value.x, resolution_value.y]

func _get_resolution_by_name(resolution_name : StringName) -> Vector2i:
	for resolution in _resolutions:
		if resolution["name"] == resolution_name or resolution["tag"] == resolution_name:
			return resolution["value"]
	return Vector2i(-1, -1)
