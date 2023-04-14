extends Node

var Options = preload("Events/Options.gd").new()
var Localisation = preload("Events/Localisation.gd").new()

const _province_identifier_file : String = "res://common/map/provinces.json"
const _region_file : String = "res://common/map/regions.json"
const _province_shape_file : String = "res://common/map/provinces.png"

# REQUIREMENTS
# * FS-333, FS-335, FS-341
func _ready():
	if MapSingleton.load_province_identifier_file(_province_identifier_file) != OK:
		push_error("Failed to load province identifiers")
	if MapSingleton.load_region_file(_region_file) != OK:
		push_error("Failed to load regions")
	if MapSingleton.load_province_shape_file(_province_shape_file) != OK:
		push_error("Failed to load province shapes")
