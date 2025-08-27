extends Node

func _init() -> void:
	var window_id := DisplayServer.get_window_list()[0]
	DisplayServer.window_set_size(Vector2(1280.0, 720.0), window_id)

func _ready() -> void:
	if ArgumentParser.get_option_value(&"help"): return
	_on_SceneTree_idle()

func _on_SceneTree_idle() -> void:
	if Engine.is_embedded_in_editor(): return
	var window := get_window()
	window.set_mode(Window.MODE_FULLSCREEN)
	await get_tree().process_frame
	window.transparent = false
	window.borderless = false
	var screen_pos := DisplayServer.screen_get_position(window.current_screen)
	var screen_size := DisplayServer.screen_get_size(window.current_screen)
	window.position = screen_pos + (screen_size - window.size) / 2
	ProjectSettings.set_setting.call_deferred("display/window/per_pixel_transparency/allowed", false)
