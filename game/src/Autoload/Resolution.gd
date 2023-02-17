extends Node

const _resolutions := {
	&"3840x2160": Vector2i(3840,2160),
	&"2560x1440": Vector2i(2560,1080),
	&"1920x1080": Vector2i(1920,1080),
	&"1366x768": Vector2i(1366,768),
	&"1536x864": Vector2i(1536,864),
	&"1280x720": Vector2i(1280,720),
	&"1440x900": Vector2i(1440,900),
	&"1600x900": Vector2i(1600,900),
	&"1024x600": Vector2i(1024,600),
	&"800x600": Vector2i(800,600)
}

func has_resolution(resolution_name : StringName) -> bool:
	return resolution_name in _resolutions

func get_resolution(resolution_name : StringName, default : Vector2i = Vector2i(1920, 1080)) -> Vector2i:
	return _resolutions.get(resolution_name, default)

func get_resolution_name_list() -> Array:
	return _resolutions.keys()

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
