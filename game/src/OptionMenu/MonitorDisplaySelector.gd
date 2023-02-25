extends SettingOptionButton

func _setup_button():
	clear()
	for screen_index in DisplayServer.get_screen_count():
		add_item("Monitor %d" % (screen_index + 1))
	default_selected = get_viewport().get_window().current_screen

func _on_item_selected(index : int):
	if _valid_index(index):
		var window := get_viewport().get_window()
		var mode := window.mode
		window.mode = Window.MODE_WINDOWED
		get_viewport().get_window().set_current_screen(index)
		window.mode = mode
	else:
		push_error("Invalid MonitorDisplaySelector index: %d" % index)
		reset_setting()
