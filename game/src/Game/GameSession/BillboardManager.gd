extends MultiMeshInstance3D

#given a name: get the index for the texture in the shader
# this is to reduce the number of magic indeces in the code
# to get the proper billboard image
var billboard_names : Dictionary = {}

@export var _map_view : MapView

const SCALE_FACTOR : float = 1.0/96.0

enum ProvinceBillboards { NONE, INVISIBLE, RGO, CRIME, NATIONAL_FOCUS }
enum MapModes { REVOLT_RISK = 2, INFRASTRUCTURE = 5, COLONIAL = 6, NATIONAL_FOCUS = 9, RGO_OUTPUT = 10 }

var provinces_size : int = 0
var total_capitals_size : int = 0

var textures : Array[Texture2D] = []
var frames : Array[int] = []
var scales : Array[float] = []

var current_province_billboard : ProvinceBillboards = ProvinceBillboards.NONE

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
	const noOfFrames_key : StringName = &"noFrames"
	
	var billboards : Array[Dictionary] = MapItemSingleton.get_billboards()
	for j : int in billboards.size():
		var billboard : Dictionary = billboards[j]
		
		var billboard_name : String = billboard[name_key]
		var texture_name : String = billboard[texture_key]
		var billboard_scale : float = billboard[scale_key]
		var noFrames : int = billboard[noOfFrames_key]
		
		#fix the alpha edges of the billboard textures
		var texture : Texture2D = AssetManager.get_texture(texture_name)
		var image : Image = texture.get_image()
		image.fix_alpha_edges()
		texture.set_image(image)
		
		textures.push_back(texture)
		frames.push_back(noFrames)
		scales.push_back(billboard_scale*SCALE_FACTOR)
		billboard_names[billboard_name] = j
	
	var material : ShaderMaterial = multimesh.mesh.surface_get_material(0)
	if material == null:
		push_error("ShaderMaterial for billboards was null")
		return
	
	material.set_shader_parameter("billboards",textures)
	material.set_shader_parameter("numframes",frames)
	material.set_shader_parameter("sizes",scales)
	multimesh.mesh.surface_set_material(0,material)
	
	var positions : PackedVector2Array = MapItemSingleton.get_province_positions()
	provinces_size = positions.size()
	total_capitals_size = MapItemSingleton.get_capital_count()
	
	# 1) setting instance_count clears and resizes the buffer
	# so we want to find the max size once and leave it
	# 2) resize must occur after setting the transform format
	multimesh.instance_count = provinces_size + total_capitals_size
	multimesh.visible_instance_count = provinces_size + total_capitals_size

	set_capitals()

	var map_positions : PackedVector3Array = to_map_coords(positions)

	for i : int in positions.size():
		multimesh.set_instance_transform(i + total_capitals_size, Transform3D(Basis(), 
			map_positions[i]
		))

	GameSingleton.mapmode_changed.connect(_on_map_mode_changed)
	GameSingleton.gamestate_updated.connect(_on_game_state_changed)

#TODO: Get rid of the vertical stretch, proper capitals placement

#fetch the nation capitals and setup billboards for them
func set_capitals() -> void:
	var positions : PackedVector2Array = MapItemSingleton.get_capital_positions()
	var capital_positions : PackedVector3Array = to_map_coords(positions)
	var image_index : int = billboard_names["capital"]
	
	#multimesh.visible_instance_count = capitals_begin_index + capital_positions.size()
	for i : int in capital_positions.size():
		multimesh.set_instance_transform(i,Transform3D(Basis(),
			capital_positions[i]
		))
		
		#capital image, frame=1 ,2x unused
		#frame=1 because frame=0 would cause capitals not to show
		#and as an index its fine, since the shader UVs will wrap around
		# 1.0 to 2.0 --> 0.0 to 1.0 so the capital image is preserved
		multimesh.set_instance_custom_data(i,Color(#capital_index,Color(
			image_index,1.0,0,0
		))
	# For every country that doesn't exist, make the capital invisible
	for i : int in total_capitals_size - capital_positions.size():
		multimesh.set_instance_custom_data(capital_positions.size() + i, Color(
			image_index,0.0,0,0
		))

# should provinces display RGO, crime, ..., or no billboard
func set_province_billboards(display : ProvinceBillboards = ProvinceBillboards.INVISIBLE) -> void:
	var image_index : int = 0
	var icons : PackedByteArray = PackedByteArray()
	icons.resize(provinces_size)
	icons.fill(0) #by default, display nothing (invisible)
	match display:
		ProvinceBillboards.RGO:
			image_index = billboard_names["tradegoods"]
			icons = MapItemSingleton.get_rgo_icons()
			current_province_billboard = display
		ProvinceBillboards.CRIME:
			image_index = billboard_names["crimes"]
			icons = MapItemSingleton.get_crime_icons()
			current_province_billboard = display
		ProvinceBillboards.NATIONAL_FOCUS:
			image_index = billboard_names["national_focus"]
			icons = MapItemSingleton.get_national_focus_icons()
			current_province_billboard = display
		ProvinceBillboards.NONE:
			current_province_billboard = display
		_: #display nothing, but keep the current billboard setting
			pass
	# capitals are first in the array, so start iterating after them
	for i : int in provinces_size:
		multimesh.set_instance_custom_data(i + total_capitals_size,Color(
			image_index,icons[i],0,0
		))

func _on_game_state_changed() -> void:
	set_province_billboards(current_province_billboard)
	set_capitals()

# There are essentially 3 visibility states we can be in
# 1: parchment view -> no billboards visible
# 2: not parchment nor detail -> only capitals visible
# 3: detail map -> province and capital billboards visible
# So set_visible here is essentially to toggle the visibility of capitals

func detailed_map(visible : bool) -> void:
	if visible:
		set_visible(true)
		set_province_billboards(current_province_billboard)
	else:
		set_visible(true)
		set_province_billboards()

func parchment_view(is_parchment : bool) -> void:
	if is_parchment:
		set_visible(false)
	else:
		detailed_map(false)

func _on_map_mode_changed(map_mode : int) -> void:
	match map_mode:
		MapModes.INFRASTRUCTURE, MapModes.COLONIAL, MapModes.RGO_OUTPUT:
			set_province_billboards(ProvinceBillboards.RGO)
		MapModes.REVOLT_RISK:
			set_province_billboards(ProvinceBillboards.CRIME)
		MapModes.NATIONAL_FOCUS:
			set_province_billboards(ProvinceBillboards.NATIONAL_FOCUS)
		_:
			set_province_billboards(ProvinceBillboards.NONE)

func to_map_coords(positions : PackedVector2Array) -> PackedVector3Array:
	var map_positions : PackedVector3Array = PackedVector3Array()
	for pos_in : Vector2 in positions:
		var pos : Vector3 = _map_view._map_to_world_coords(pos_in)
		map_positions.push_back(pos)
	return map_positions
