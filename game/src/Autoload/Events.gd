extends Node

var GameDebug = preload("Events/GameDebug.gd").new()
var Options = preload("Events/Options.gd").new()
var Localisation = preload("Events/Localisation.gd").new()
var ShaderManager = preload("Events/ShaderManager.gd").new()

const _province_identifier_file : String = "res://common/map/provinces.json"
const _water_province_file : String = "res://common/map/water.json"
const _region_file : String = "res://common/map/regions.json"
const _terrain_file : String = "res://common/map/terrain.json"
const _province_image_file : String = "res://common/map/provinces.png"
const _terrain_image_file : String = "res://common/map/terrain.png"

# REQUIREMENTS
# * FS-333, FS-334, FS-335, FS-341
func _ready():
	if GameSingleton.load_province_identifier_file(_province_identifier_file) != OK:
		push_error("Failed to load province identifiers")
	if GameSingleton.load_water_province_file(_water_province_file) != OK:
		push_error("Failed to load water provinces")
	if GameSingleton.load_region_file(_region_file) != OK:
		push_error("Failed to load regions")
	if GameSingleton.load_terrain_file(_terrain_file) != OK:
		push_error("Failed to load terrain variants")
	if GameSingleton.load_map_images(_province_image_file, _terrain_image_file) != OK:
		push_error("Failed to load map images")
