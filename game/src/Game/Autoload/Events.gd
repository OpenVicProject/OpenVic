extends Node

var GameDebug: GameDebugSingleton
var Options: OptionsSingleton
var Localisation: LocalisationSingleton
var ShaderManager: ShaderManagerSingleton

var _define_filepaths_dict : Dictionary = {
	GameSingleton.get_province_identifier_file_key(): "res://common/map/provinces.json",
	GameSingleton.get_water_province_file_key(): "res://common/map/water.json",
	GameSingleton.get_region_file_key(): "res://common/map/regions.json",
	GameSingleton.get_terrain_variant_file_key(): "res://common/map/terrain.json",
	GameSingleton.get_terrain_texture_dir_key(): "res://art/terrain/",
	GameSingleton.get_province_image_file_key(): "res://common/map/provinces.png",
	GameSingleton.get_terrain_image_file_key(): "res://common/map/terrain.png",
	GameSingleton.get_goods_file_key(): "res://common/goods.json",
	GameSingleton.get_good_icons_dir_key(): "res://art/economy/goods"
}

# REQUIREMENTS
# * FS-333, FS-334, FS-335, FS-341
func load_events(loading_screen: LoadingScreen):
	GameSingleton.setup_logger()
	loading_screen.update_loading_screen(5)
	
	# Set this to your Vic2 install dir or a mod's dir to enable compatibility mode
	# (this won't work for mods which rely on vanilla map assets, copy missing assets
	# into the mod's dir for a temporary fix)
	# Usage: OpenVic --compatibility-mode <path>

	var compatibility_mode_path : String
	if ProjectSettings.has_setting(ArgumentParser.argument_setting_path):
		var arg_dictionary : Dictionary = ProjectSettings.get_setting(ArgumentParser.argument_setting_path)
		compatibility_mode_path = arg_dictionary.get(&"compatibility-mode", compatibility_mode_path)

	var start := Time.get_ticks_usec()
	
	GameDebug = GameDebugSingleton.new()
	loading_screen.update_loading_screen(15)
	Options = OptionsSingleton.new()
	loading_screen.update_loading_screen(25)
	Localisation = LocalisationSingleton.new()
	loading_screen.update_loading_screen(45)
	ShaderManager = ShaderManagerSingleton.new()
	loading_screen.update_loading_screen(50, true)
	
	if compatibility_mode_path:
		if GameSingleton.load_defines_compatibility_mode(compatibility_mode_path) != OK:
			push_error("Errors loading game defines!")
	else:
		if GameSingleton.load_defines(_define_filepaths_dict) != OK:
			push_error("Errors loading game defines!")
	
	loading_screen.update_loading_screen(100)
	var end := Time.get_ticks_usec()
	print("Loading took ", float(end - start) / 1000000, " seconds")
	
	# change scene in a thread-safe way
	get_tree().call_deferred("change_scene_to_file", "res://src/Game/GameMenu.tscn")
