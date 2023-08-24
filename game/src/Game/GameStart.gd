extends Control

const LoadingScreen = preload("res://src/Game/LoadingScreen.gd")
const SoundTabScene = preload("res://src/Game/Menu/OptionMenu/SoundTab.tscn")

@export_subgroup("Nodes")
@export var loading_screen : LoadingScreen

func _ready() -> void:
	# Hack to ensure Sound Options load
	var sound_tab := SoundTabScene.instantiate()
	sound_tab.visible = false
	add_child(sound_tab)
	Events.Options.load_settings_from_file()
	sound_tab.queue_free()

	loading_screen.start_loading_screen(_initialize_game)

# REQUIREMENTS
# * FS-333, FS-334, FS-335, FS-341
func _initialize_game() -> void:
	GameSingleton.setup_logger()
	loading_screen.try_update_loading_screen(5)

	# Set this to your Vic2 install dir or a mod's dir to enable compatibility mode
	# (this won't work for mods which rely on vanilla map assets, copy missing assets
	# into the mod's dir for a temporary fix)
	# Usage: OpenVic --compatibility-mode <path>

	var compatibility_mode_path : String = ArgumentParser.get_argument(&"compatibility-mode")

	var start := Time.get_ticks_usec()

	loading_screen.try_update_loading_screen(15)
	loading_screen.try_update_loading_screen(25)
	Localisation.initialize()
	loading_screen.try_update_loading_screen(45)
	loading_screen.try_update_loading_screen(50, true)

	# TODO: Loading takes way too long to keep the LoadingScreen at 50%
	# Should either split this up or seperately multithread the compatibility mode loader
	# Or both and emit a signal that allows us to add percentages to the LoadingScreen
	if compatibility_mode_path:
		if GameSingleton.load_defines_compatibility_mode(compatibility_mode_path) != OK:
			push_error("Errors loading game defines!")
	else:
		GameLoader.define_filepaths_dict.make_read_only()
		if GameSingleton.load_defines(GameLoader.define_filepaths_dict) != OK:
			push_error("Errors loading game defines!")

	loading_screen.try_update_loading_screen(100)
	var end := Time.get_ticks_usec()
	print("Loading took ", float(end - start) / 1000000, " seconds")

	# change scene in a thread-safe way
	get_tree().call_deferred("change_scene_to_file", "res://src/Game/GameMenu.tscn")

func _on_splash_container_splash_end():
	loading_screen.show()
