extends Node

var define_filepaths_dict : Dictionary = {
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

var ShaderManager : ShaderManagerClass

func _init():
	ShaderManager = ShaderManagerClass.new()
