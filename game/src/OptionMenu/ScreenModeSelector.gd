extends SettingRevertButton

# REQUIREMENTS
# * UIFUN-42

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

func _setup_button():
	default_selected = get_screen_mode_from_window_mode(get_viewport().get_window().mode)
	selected = default_selected

func _on_option_selected(index : int, by_user : bool) -> void:
	if _valid_index(index):
		if by_user:
			print("Start Revert Countdown!")
			revert_dialog.show_dialog.call_deferred(self)
			previous_index = get_screen_mode_from_window_mode(get_viewport().get_window().mode)

		var current_resolution := Resolution.get_current_resolution()
		var window_mode := get_window_mode_from_screen_mode(index)
		Resolution.window_mode_changed.emit(window_mode)
		get_viewport().get_window().mode = window_mode
		Resolution.set_resolution(current_resolution)
	else:
		push_error("Invalid ScreenModeSelector index: %d" % index)
		reset_setting(not by_user)
