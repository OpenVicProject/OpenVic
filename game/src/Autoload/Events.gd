extends Node

var GameDebug = preload("Events/GameDebug.gd").new()
var Options = preload("Events/Options.gd").new()
var Localisation = preload("Events/Localisation.gd").new()
var ShaderManager = preload("Events/ShaderManager.gd").new()

var _define_filepaths_dict : Dictionary = {
	GameSingleton.get_province_identifier_file_key(): "res://common/map/provinces.json",
	GameSingleton.get_water_province_file_key(): "res://common/map/water.json",
	GameSingleton.get_region_file_key(): "res://common/map/regions.json",
	GameSingleton.get_terrain_variant_file_key(): "res://common/map/terrain.json",
	GameSingleton.get_province_image_file_key(): "res://common/map/provinces.png",
	GameSingleton.get_terrain_image_file_key(): "res://common/map/terrain.png",
	GameSingleton.get_goods_file_key(): "res://common/goods.json",
	GameSingleton.get_good_icons_dir_key(): "res://art/economy/goods"
}

# REQUIREMENTS
# * FS-333, FS-334, FS-335, FS-341
func _ready():
	var start := Time.get_ticks_usec()
	if GameSingleton.load_defines(_define_filepaths_dict) != OK:
		push_error("Failed to load game defines!")
	var end := Time.get_ticks_usec()
	print("Loading took ", float(end - start) / 1000000, " seconds")
