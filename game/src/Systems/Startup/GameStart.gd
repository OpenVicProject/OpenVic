extends Control

const LoadingScreen := preload("res://src/Systems/Startup/LoadingScreen.gd")
const GameMenuScene := preload("res://src/UI/GameMenu/GameMenu/GameMenu.tscn")

@export_subgroup("Nodes")
@export var loading_screen : LoadingScreen

func _enter_tree() -> void:
	Keychain.keep_binding_check = func(action_name : StringName) -> bool:
		return action_name.begins_with("button_") and action_name.ends_with("_hotkey")

func _ready() -> void:
	Keychain.actions = {
		# Map Group
		&"map_north": Keychain.InputAction.new("Move North", "Map", true),
		&"map_east": Keychain.InputAction.new("Move East", "Map", true),
		&"map_south": Keychain.InputAction.new("Move South", "Map", true),
		&"map_west": Keychain.InputAction.new("Move West", "Map", true),
		&"map_zoom_in": Keychain.InputAction.new("Zoom In", "Map", true),
		&"map_zoom_out": Keychain.InputAction.new("Zoom Out", "Map", true),
		&"map_drag": Keychain.InputAction.new("Mouse Drag", "Map", true),
		&"map_click": Keychain.InputAction.new("Mouse Click", "Map", true),
		&"map_right_click": Keychain.InputAction.new("Mouse Right Click", "Map", true),
		# Time Group
		&"time_pause": Keychain.InputAction.new("Pause", "Time", true),
		&"time_speed_increase": Keychain.InputAction.new("Speed Increase", "Time", true),
		&"time_speed_decrease": Keychain.InputAction.new("Speed Decrease", "Time", true),
		# UI Group
		&"menu_pause": Keychain.InputAction.new("Open Pause Menu", "UI", true),
	}

	Keychain.groups = {
		"Map": Keychain.InputGroup.new("", false),
		"Time": Keychain.InputGroup.new("", false),
		"UI": Keychain.InputGroup.new("", false),
		"Hotkeys": Keychain.InputGroup.new("UI")
	}

	if ArgumentParser.get_option_value(&"help"):
		# For some reason this doesn't get freed properly
		# Godot will always quit before it frees the active StreamPlayback resource
		# This hack fixes that
		MusicManager.queue_free()
		get_tree().quit()
		return

	await _setup_compatibility_mode_paths()
	await loading_screen.start_loading_screen(_initialize_game)

func _setup_compatibility_mode_paths() -> void:
	# To test mods, set your base path to Victoria II and then pass mods in reverse order with --mod="mod" for each mod.

	var arg_base_path : String = ArgumentParser.get_option_value(&"base-path")
	var arg_search_path : String = ArgumentParser.get_option_value(&"search-path")

	var setting := Vic2Settings.get_setting(Vic2Settings.GENERAL_BASE_DEFINES_PATH)

	if arg_base_path:
		if arg_search_path:
			push_warning("Exact base path and search base path arguments both used:\nBase: ", arg_base_path, "\nSearch: ", arg_search_path)
		setting.set_value(arg_base_path)
	elif Vic2Settings.get_base_defines_path().is_empty() and arg_search_path:
		# This will also search for a Steam install if the hint doesn't help
		setting.set_value(Vic2Settings.find_base_path(arg_search_path))

	if Vic2Settings.get_base_defines_path().is_empty():
		# Check if the program is being run from inside the install directory,
		# and if not also search for a Steam install
		var root_base_path := Vic2Settings.find_base_path(".")
		if root_base_path.is_empty():
			await Vic2Settings.show_base_path_find_dialog()
		else:
			setting.set_value(root_base_path)

	# Add mod paths
	var load_list_setting := ModSettings.get_setting(ModSettings.MODS_LOAD_LIST)
	var load_list := ModSettings.get_load_list()
	for mod in ArgumentParser.get_option_value(&"mod"):
		if mod not in load_list and mod != "":
			load_list.append(mod)
	load_list_setting.set_value(load_list)

func _load_compatibility_mode() -> void:
	if GameSingleton.set_compatibility_mode_roots(Vic2Settings.get_base_defines_path()) != OK:
		push_error("Errors setting game roots!")
	
	CursorManager.initial_cursor_setup()
	setup_title_theme()

	if GameSingleton.load_defines_compatibility_mode(ModSettings.get_load_list()) != OK:
		push_error("Errors loading game defines!")

	SoundSingleton.load_sounds()
	SoundSingleton.load_music()
	MusicManager.add_compat_songs()

func setup_title_theme() -> void:
	SoundSingleton.load_title_theme()

	MusicManager.setup_compat_song(SoundSingleton.title_theme)

	var song_paths = MusicManager.get_all_song_paths()
	var title_index = song_paths.find(SoundSingleton.title_theme)
	if title_index != -1:
		MusicManager.start_song_by_index.call_deferred(title_index)
	if len(MusicManager._available_songs) <= 0:
		push_error("No song available to play")
	else:
		MusicManager.start_current_song.call_deferred()

# REQUIREMENTS
# * FS-333, FS-334, FS-335, FS-341
func _initialize_game() -> void:
	if Vic2Settings.get_base_defines_path().is_empty():
		return

	var start := Time.get_ticks_usec()
	loading_screen.try_update_loading_screen(0)
	GameSingleton.setup_logger()

	loading_screen.try_update_loading_screen(15, true)

	_load_compatibility_mode()
	loading_screen.try_update_loading_screen(75, true)

	loading_screen.try_update_loading_screen(100)

	var end := Time.get_ticks_usec()
	print("Loading took ", float(end - start) / 1000000, " seconds")

	# change scene in a thread-safe way
	get_tree().change_scene_to_packed.call_deferred(GameMenuScene)

func _on_splash_container_splash_end() -> void:
	loading_screen.show()
