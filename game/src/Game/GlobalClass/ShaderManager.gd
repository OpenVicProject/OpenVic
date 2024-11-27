class_name ShaderManagerClass
extends RefCounted

const param_province_shape_tex : StringName = &"province_shape_tex"
const param_province_shape_subdivisions : StringName = &"province_shape_subdivisions"
const param_province_colour_tex : StringName = &"province_colour_tex"
const param_hover_index : StringName = &"hover_index"
const param_selected_index : StringName = &"selected_index"
const param_parchment_mix : StringName = &"parchment_mix"
const param_terrain_tex : StringName = &"terrain_tex"
const param_terrain_tile_factor : StringName = &"terrain_tile_factor"
const param_stripe_tex : StringName = &"stripe_tex"
const param_stripe_tile_factor : StringName = &"stripe_tile_factor"
const param_overlay_tex : StringName = &"overlay_tex"
const param_overlay_tile_factor : StringName = &"overlay_tile_factor"
const param_colormap_land_tex : StringName = &"colormap_land_tex"
const param_colormap_water_tex : StringName = &"colormap_water_tex"
const param_colormap_overlay_tex : StringName = &"colormap_overlay_tex"

func _set_shader_texture(
	shader_material : ShaderMaterial, texture_param : StringName, texture : Texture,
	tile_factor_param : StringName = &"", pixels_per_tile : float = 0.0
) -> Error:
	var err : Error = OK
	if texture != null:
		shader_material.set_shader_parameter(texture_param, texture)
	else:
		push_error("Invalid texture for shader parameter ", texture_param, " - null!")
		err = FAILED
	if tile_factor_param:
		# Set to 1.0 / pixels_per_tile as the shader can multiply faster than it can divide, and it will not automatically
		# optimize to multiplication by a reciprocal for fear of losing precision. As pixels_per_tile is often a power of two,
		# this doesn't actually lose any precision, and even if it did it would be insignificant.
		shader_material.set_shader_parameter(tile_factor_param, 1.0 / pixels_per_tile)
	return err

func _set_shader_asset_texture(
	shader_material : ShaderMaterial, texture_param : StringName, texture_path : StringName,
	tile_factor_param : StringName = &"", pixels_per_tile : float = 0.0
) -> Error:
	return _set_shader_texture(
		shader_material, texture_param, AssetManager.get_texture(texture_path), tile_factor_param, pixels_per_tile
	)

func set_up_shader(material : Material, add_cosmetic_textures : bool) -> Error:
	# Shader Material
	if material == null:
		push_error("material is null!")
		return FAILED
	if not material is ShaderMaterial:
		push_error("Invalid map mesh material class: ", material.get_class())
		return FAILED
	var shader_material : ShaderMaterial = material

	var ret : Error = OK

	# Province shape texture
	if _set_shader_texture(shader_material, param_province_shape_tex, GameSingleton.get_province_shape_texture()) != OK:
		push_error("Failed to set province shape shader texture array!")
		ret = FAILED

	var subdivisions : Vector2i = GameSingleton.get_province_shape_image_subdivisions()
	if subdivisions.x >= 1 and subdivisions.y >= 1:
		shader_material.set_shader_parameter(param_province_shape_subdivisions, Vector2(subdivisions))
	else:
		push_error("Invalid province shape image subdivision: ", subdivisions.x, "x", subdivisions.y)
		ret = FAILED

	if add_cosmetic_textures:

		# Province colour texture
		if _set_shader_texture(shader_material, param_province_colour_tex, GameSingleton.get_province_colour_texture()) != OK:
			push_error("Failed to set province colour shader texture!")
			ret = FAILED

		# Terrain texture
		const pixels_per_terrain_tile : float = 16.0
		if _set_shader_texture(
			shader_material,
			param_terrain_tex, GameSingleton.get_terrain_texture(),
			param_terrain_tile_factor, pixels_per_terrain_tile
		) != OK:
			push_error("Failed to set terrain shader texture array!")
			ret = FAILED

		# Stripe texture
		const pixels_per_stripe_tile : float = 8.0
		if _set_shader_asset_texture(
			shader_material,
			param_stripe_tex, &"map/terrain/stripes.dds",
			param_stripe_tile_factor, pixels_per_stripe_tile
		) != OK:
			push_error("Failed to set stripe shader texture!")
			ret = FAILED

		# Overlay texture
		const pixels_per_overlay_tile : float = 512.0
		if _set_shader_asset_texture(
			shader_material,
			param_overlay_tex, &"map/terrain/map_overlay_tile.dds",
			param_overlay_tile_factor, pixels_per_overlay_tile
		) != OK:
			push_error("Failed to set overlay shader texture!")
			ret = FAILED

		# Land colormap
		if _set_shader_asset_texture(shader_material, param_colormap_land_tex, &"map/terrain/colormap.dds") != OK:
			push_error("Failed to set land colormap shader texture!")
			ret = FAILED
		# Water colormap
		if _set_shader_asset_texture(shader_material, param_colormap_water_tex, &"map/terrain/colormap_water.dds") != OK:
			push_error("Failed to set water colormap shader texture!")
			ret = FAILED
		# Overlay colormap
		if _set_shader_asset_texture(shader_material, param_colormap_overlay_tex, &"map/terrain/colormap_political.dds") != OK:
			push_error("Failed to set overlay colormap shader texture!")
			ret = FAILED

	return ret
