extends SettingOptionButton

enum ScreenMode { Unknown = -1, Fullscreen, Borderless, Windowed }

func get_screen_mode_from_window_mode(window_mode : int) -> ScreenMode:
	match window_mode:
		Window.MODE_EXCLUSIVE_FULLSCREEN:
			return ScreenMode.Fullscreen
		Window.MODE_FULLSCREEN:
			return ScreenMode.Borderless
		Window.MODE_WINDOWED:
			return ScreenMode.Windowed
		_:
			return ScreenMode.Unknown

func get_window_mode_from_screen_mode(screen_mode : int) -> Window.Mode:
	match screen_mode:
		ScreenMode.Fullscreen:
			return Window.MODE_EXCLUSIVE_FULLSCREEN
		ScreenMode.Borderless:
			return Window.MODE_FULLSCREEN
		ScreenMode.Windowed:
			return Window.MODE_WINDOWED
		_:
			return Window.MODE_EXCLUSIVE_FULLSCREEN

func _on_item_selected(index : int):
	print("Selected index: %d" % index)

	var window := get_viewport().get_window()
	var current_resolution := Resolution.get_current_resolution()
	window.mode = get_window_mode_from_screen_mode(index)
	Resolution.set_resolution(current_resolution)
