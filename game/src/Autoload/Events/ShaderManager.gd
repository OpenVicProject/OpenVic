extends RefCounted

const param_province_shape_tex : StringName = &"province_shape_tex"
const param_province_shape_subdivisions : StringName = &"province_shape_subdivisions"
const param_province_colour_tex : StringName = &"province_colour_tex"
const param_hover_index : StringName = &"hover_index"
const param_selected_index : StringName = &"selected_index"
const param_terrain_tex : StringName = &"terrain_tex"
const param_terrain_tile_factor : StringName = &"terrain_tile_factor"

func set_up_shader(material : Material, add_cosmetic_textures : bool) -> Error:
	# Shader Material
	if material == null:
		push_error("material is null!")
		return FAILED
	if not material is ShaderMaterial:
		push_error("Invalid map mesh material class: ", material.get_class())
		return FAILED
	var shader_material : ShaderMaterial = material

	# Province shape texture
	var province_shape_texture := GameSingleton.get_province_shape_texture()
	if province_shape_texture == null:
		push_error("Failed to get province shape texture!")
		return FAILED
	shader_material.set_shader_parameter(param_province_shape_tex, province_shape_texture)
	var subdivisions := GameSingleton.get_province_shape_image_subdivisions()
	if subdivisions.x < 1 or subdivisions.y < 1:
		push_error("Invalid province shape image subdivision: ", subdivisions.x, "x", subdivisions.y)
		return FAILED
	shader_material.set_shader_parameter(param_province_shape_subdivisions, Vector2(subdivisions))

	if add_cosmetic_textures:
		# Province colour texture
		var map_province_colour_texture := GameSingleton.get_province_colour_texture()
		if map_province_colour_texture == null:
			push_error("Failed to get province colour image!")
			return FAILED
		shader_material.set_shader_parameter(param_province_colour_tex, map_province_colour_texture)

		# Terrain texture
		var terrain_texture := GameSingleton.get_terrain_texture()
		if terrain_texture == null:
			push_error("Failed to get terrain texture!")
			return FAILED
		shader_material.set_shader_parameter(param_terrain_tex, terrain_texture)

	return OK
