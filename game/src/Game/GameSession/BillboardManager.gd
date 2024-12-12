extends MultiMeshInstance3D

@export var _map_view : MapView

const SCALE_FACTOR : float = 1.0 / 96.0

enum BillboardType { NONE, RGO, CRIME, NATIONAL_FOCUS, CAPITAL }
const BILLBOARD_NAMES : Dictionary = {
	BillboardType.RGO: &"tradegoods",
	BillboardType.CRIME: &"crimes",
	BillboardType.NATIONAL_FOCUS: &"national_focus",
	BillboardType.CAPITAL: &"capital"
}
const BILLBOARD_DIMS : Dictionary = {
	# Should be 0.8, but something else seems to be contributing to vertical
	# stretching so we use 0.7 to account for that.
	BillboardType.RGO: Vector2(1.0, 0.7),

	BillboardType.CRIME: Vector2(1.0, 1.0),
	BillboardType.NATIONAL_FOCUS: Vector2(1.0, 1.0),
	BillboardType.CAPITAL: Vector2(1.0, 1.0)
}
enum MapModes { REVOLT_RISK = 2, INFRASTRUCTURE = 5, COLONIAL = 6, NATIONAL_FOCUS = 9, RGO_OUTPUT = 10 }

var provinces_size : int = 0
var total_capitals_size : int = 0

# Given a BillboardType, get the index for the texture in the shader.
# This is to reduce the number of magic indeces in the code
# to get the proper billboard image
var billboard_type_to_index : Dictionary

var textures : Array[Texture2D]
var frames : PackedByteArray
var scales : PackedVector2Array

var current_province_billboard : BillboardType = BillboardType.NONE
var province_billboards_visible : bool = true

# ============== Billboards =============
# Billboards are displayed using a multimesh (batch drawn mesh)
#  with positions set to every province and every nation capital.
# A shader controls which billboard and frame from icon strips are displayed
#  at each province. It also makes billboards "look at" the camera
# To ensure billboards are displayed ontop of the map and units, it is contained in
#  a subviewport which renders above the main viewport, with a camera set to follow the primary camera

# multimesh only lets us send custom data to the shader as a single float vec4/Color variable
#  So we need to make the most of it.
# Info send to the shader is as follows:
# x: image to use from imageArray
# y: frame from selected image for icon strips. 0 indicate invisible (alpha = 0.0)
# z: unused, perhaps progress for progress bars if this shader is reused?
# w: unused

# "Province billboards" refer to billboards positioned on every province
#  for map modes such as RGO output, while "Capital billboards" refers to the
#  to country capitals.

func _ready() -> void:
	const name_key : StringName = &"name"
	const texture_key : StringName = &"texture"
	const scale_key : StringName = &"scale"
	const no_of_frames_key : StringName = &"noFrames"

	for billboard : Dictionary in MapItemSingleton.get_billboards():
		var billboard_name : StringName = billboard[name_key]

		var billboard_type : BillboardType = BillboardType.NONE
		for key : BillboardType in BILLBOARD_NAMES:
			if billboard_name == BILLBOARD_NAMES[key]:
				billboard_type = key
				break

		if billboard_type == BillboardType.NONE:
			continue

		var texture_name : StringName = billboard[texture_key]
		var billboard_scale : float = billboard[scale_key]
		var no_of_frames : int = billboard[no_of_frames_key]

		#fix the alpha edges of the billboard textures
		var texture : ImageTexture = AssetManager.get_texture(texture_name)
		if texture == null:
			push_error("Texture for billboard \"", billboard_name, "\" was null!")
			continue
		var image : Image = texture.get_image()
		image.fix_alpha_edges()
		texture.set_image(image)

		# We use the texture array size (which will be the same as frames and scales' sizes)
		# rather than billboard_index as the former only counts billboards we're actually using,
		# while the latter counts all billboards defined in the game's GFX files
		billboard_type_to_index[billboard_type] = textures.size()

		textures.push_back(texture)
		frames.push_back(no_of_frames)
		scales.push_back(BILLBOARD_DIMS[billboard_type] * billboard_scale * SCALE_FACTOR)

	var material : ShaderMaterial = multimesh.mesh.surface_get_material(0)
	if material == null:
		push_error("ShaderMaterial for billboards was null")
		return

	material.set_shader_parameter(&"billboards", textures)
	material.set_shader_parameter(&"numframes", frames)
	material.set_shader_parameter(&"sizes", scales)
	multimesh.mesh.surface_set_material(0, material)

	var positions : PackedVector2Array = MapItemSingleton.get_province_positions()
	provinces_size = positions.size()
	total_capitals_size = MapItemSingleton.get_max_capital_count()

	# 1) setting instance_count clears and resizes the buffer
	# so we want to find the max size once and leave it
	# 2) resize must occur after setting the transform format
	multimesh.instance_count = provinces_size + total_capitals_size

	if _map_view == null:
		push_error("MapView export varible for BillboardManager must be set!")
		return

	for province_index : int in provinces_size:
		multimesh.set_instance_transform(
			province_index + total_capitals_size,
			Transform3D(Basis(), _map_view._map_to_world_coords(positions[province_index]))
		)

	# These signals will trigger and update capitals and province icons right
	# at the beginning of (as well as later throughout) the game session
	GameSingleton.mapmode_changed.connect(_on_map_mode_changed)
	GameSingleton.gamestate_updated.connect(_on_game_state_changed)

# Fetch the nation capitals and setup billboards for them
func set_capitals() -> void:
	var capital_positions : PackedVector2Array = MapItemSingleton.get_capital_positions()
	var capitals_size : int = capital_positions.size()
	var image_index : int = billboard_type_to_index[BillboardType.CAPITAL]

	for capital_index : int in capitals_size:
		multimesh.set_instance_transform(
			capital_index,
			Transform3D(Basis(), _map_view._map_to_world_coords(capital_positions[capital_index]))
		)

		# capital image, frame=1, 2x unused
		# frame=1 because frame=0 would cause capitals not to show
		# and as an index its fine, since the shader UVs will wrap around
		# 1.0 to 2.0 --> 0.0 to 1.0 so the capital image is preserved
		multimesh.set_instance_custom_data(
			capital_index,
			Color(image_index, 1.0, 0.0, 0.0)
		)

	# For every country that doesn't exist, make the capital invisible
	for capital_index : int in range(capitals_size, total_capitals_size):
		multimesh.set_instance_custom_data(
			capital_index,
			Color(image_index, 0.0, 0.0, 0.0)
		)

# Should provinces display RGO, crime, ..., or no billboard
func update_province_billboards() -> void:
	# If current_province_billboard is NONE then image_index will fall back to -1
	var image_index : int = billboard_type_to_index.get(current_province_billboard, -1)
	if not province_billboards_visible or image_index < 0:
		multimesh.visible_instance_count = total_capitals_size
	else:
		var icons : PackedByteArray
		match current_province_billboard:
			BillboardType.RGO:
				icons = MapItemSingleton.get_rgo_icons()
			BillboardType.CRIME:
				icons = MapItemSingleton.get_crime_icons()
			BillboardType.NATIONAL_FOCUS:
				icons = MapItemSingleton.get_national_focus_icons()
			_:
				push_error("Invalid province billboard type: ", current_province_billboard)
				return

		# Capitals are first in the array, so start iterating after them
		for province_index : int in provinces_size:
			multimesh.set_instance_custom_data(
				province_index + total_capitals_size,
				Color(image_index, icons[province_index], 0.0, 0.0)
			)

		multimesh.visible_instance_count = total_capitals_size + provinces_size

func _on_game_state_changed() -> void:
	update_province_billboards()
	set_capitals()

# There are essentially 3 visibility states we can be in
# 1: parchment view -> no billboards visible
# 2: not parchment nor detail -> only capitals visible
# 3: detail map -> province and capital billboards visible
# So set_visible here is essentially to toggle the visibility of capitals

func detailed_map(visible : bool) -> void:
	province_billboards_visible = visible
	update_province_billboards()

func parchment_view(is_parchment : bool) -> void:
	set_visible(not is_parchment)

func _on_map_mode_changed(map_mode : int) -> void:
	match map_mode:
		MapModes.INFRASTRUCTURE, MapModes.COLONIAL, MapModes.RGO_OUTPUT:
			current_province_billboard = BillboardType.RGO
		MapModes.REVOLT_RISK:
			current_province_billboard = BillboardType.CRIME
		MapModes.NATIONAL_FOCUS:
			current_province_billboard = BillboardType.NATIONAL_FOCUS
		_:
			current_province_billboard = BillboardType.NONE
	update_province_billboards()
