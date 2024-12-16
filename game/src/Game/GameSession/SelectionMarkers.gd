class_name SelectionMarkers
extends MultiMeshInstance3D

enum ProjectionType {INVALIDTYPE, SELECTED, LEGAL_MOVE, ILLEGAL_MOVE}
const PROJECTION_NAMES : Dictionary = {
	ProjectionType.SELECTED : &"selection_projection",
	ProjectionType.LEGAL_MOVE : &"legal_selection_projection",
	ProjectionType.ILLEGAL_MOVE : &"illegal_selection_projection"
}

var projection_type_to_index : Dictionary

var material : ShaderMaterial
var textures : Array[Texture2D]
var sizes : PackedFloat32Array
var spins : PackedFloat32Array
var expansions : PackedFloat32Array
var durations : PackedFloat32Array
var transparency_mode : PackedByteArray

var time : float = 0.0
const MIN_INSTANCE_COUNT : int = 64
const INSTANCE_COUNT_GROW_AMOUNT : int = 16
const SCALE_FACTOR : float = 1.0 / 256.0
const SPIN_FACTOR : float = 4.0
const HEIGHT_ADD_FACTOR : Vector3 = Vector3(0,0.002,0)
const GROW_FACTOR : float = 0.25

var ids : PackedInt32Array


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	# To start, setup the shader for this multimesh
	const name_key : StringName  = &"name";
	const texture_key : StringName  = &"texture";
	const size_key : StringName  = &"size";
	const spin_key : StringName  = &"spin";
	const expanding_key : StringName  = &"expanding";
	const duration_key : StringName  = &"duration";
	const additative_key : StringName  = &"additative";

	for projection : Dictionary in MapItemSingleton.get_projections():
		var projection_name : StringName = projection[name_key]

		#keep only projections we are currently handling
		var projection_type : ProjectionType = ProjectionType.INVALIDTYPE
		for key : ProjectionType in PROJECTION_NAMES:
			if projection_name == PROJECTION_NAMES[key]:
				projection_type = key
				break

		if projection_type == ProjectionType.INVALIDTYPE:
			continue

		var texture_name : StringName = projection[texture_key]
		var size : float = projection[size_key]
		var spin : float = projection[spin_key]
		var expanding : float = projection[expanding_key]
		var duration : float = projection[duration_key]
		var additative : bool = projection[additative_key]

		#fix the alpha edges of the projection textures
		var texture : ImageTexture = AssetManager.get_texture(texture_name)
		if texture == null:
			push_error("Texture for projection \"", projection_name, "\" was null!")
			continue
		var image : Image = texture.get_image()
		image.fix_alpha_edges()
		texture.set_image(image)

		# We use the texture array size (which will be the same as frames and scales' sizes)
		# rather than projection_index as the former only counts projections we're actually using,
		# while the latter counts all projections defined in the game's GFX files
		projection_type_to_index[projection_type] = textures.size()

		textures.push_back(texture)
		sizes.push_back(size * SCALE_FACTOR)
		spins.push_back(spin * SPIN_FACTOR)
		expansions.push_back(expanding * GROW_FACTOR)
		durations.push_back(duration)
		transparency_mode.push_back(additative)

	material = multimesh.mesh.surface_get_material(0)
	if material == null:
		push_error("ShaderMaterial for projections was null")
		return

	material.set_shader_parameter(&"projections", textures)
	material.set_shader_parameter(&"sizes", sizes)
	material.set_shader_parameter(&"spin", spins)
	material.set_shader_parameter(&"expanding", expansions)
	material.set_shader_parameter(&"additative", transparency_mode)
	material.set_shader_parameter(&"duration",durations)
	
	multimesh.mesh.surface_set_material(0, material)

	# The shader is setup, now get the management code ready

	# 1) setting instance_count clears and resizes the buffer
	# so we want to find the max size once and leave it
	# 2) resize must occur after setting the transform format
	
	ids.resize(MIN_INSTANCE_COUNT)
	ids.fill(-1)
	#unlike billboards, we don't know how many of these we'll need. Quads are pretty small
	# and multimesh means we can have a lot of them, so be generous, and resize later if we have to
	multimesh.instance_count = MIN_INSTANCE_COUNT
	multimesh.visible_instance_count = 0#MIN_INSTANCE_COUNT
	
	for i in MIN_INSTANCE_COUNT:
		multimesh.set_instance_transform(
			i,Transform3D(Basis(), Vector3(float(i),0.0,0.0))
		)

# intent of ProjectionManager is to provide an interface for mapview to add projections
# to the map at specific coordinates

func add_selection_marker(unit_id:int, unit_position:Vector3) -> void:
	var slot_found : bool = false
	for i : int in ids.size():
		var id : int = ids[i]
		if id < 0:
			multimesh.set_instance_transform(
				i,Transform3D(Basis(), unit_position + HEIGHT_ADD_FACTOR)
			)
			multimesh.set_instance_custom_data(i,Color(
				projection_type_to_index[ProjectionType.SELECTED],time,2.0,0.0 #type
			))
			ids[i] = unit_id
			slot_found = true
			break
			
	if !slot_found:
		_grow_buffer()
		
		var i = ids.size()
		ids.resize(i + INSTANCE_COUNT_GROW_AMOUNT)
		
		multimesh.set_instance_transform(
			i,Transform3D(Basis(), unit_position + HEIGHT_ADD_FACTOR)
		)
		multimesh.set_instance_custom_data(i,Color(
			projection_type_to_index[ProjectionType.SELECTED],time,2.0,0.0
		))
		ids[i] = unit_id
	multimesh.visible_instance_count += 1

func update_selection_marker(unit_id:int, unit_position:Vector3) -> void:
	var index : int = ids.find(unit_id)
	multimesh.set_instance_transform(
		index,Transform3D(Basis(), unit_position + HEIGHT_ADD_FACTOR)
	)

#TODO: Perhaps there's a more efficient method...
func add_selection_markers(unit_ids:PackedInt32Array, unit_positions:PackedVector3Array) -> void:
	assert(unit_ids.size() == unit_positions.size())
	multimesh.visible_instance_count += unit_positions.size()
	for index in unit_ids.size():
		add_selection_marker(unit_ids[index],unit_positions[index])

func update_selection_markers(unit_ids:PackedInt32Array, unit_positions:PackedVector3Array) -> void:
	assert(unit_ids.size() == unit_positions.size())
	for index in unit_ids.size():
		update_selection_marker(unit_ids[index],unit_positions[index])

func clear_selection_markers() -> void:
	#no units should display
	ids.fill(-1)
	multimesh.visible_instance_count = 0

func _process(delta: float) -> void:
	time += delta
	#set the material's time property (shared between instances)
	
	#time is an instance uniform, this way we have an independent time from
	#the legal/illegal move marker projections
	#multimesh.set_instance_shader_parameter(&"time", time)
	

#Grow the multimesh without losing the current data
func _grow_buffer(amount : int = INSTANCE_COUNT_GROW_AMOUNT) -> void:
	# we can control the multimesh buffer directly
	# 12 floats for transform, 4 floats for colour, 4 floats for custom data
	# colour and custom data are optional
	const TR_SIZE : int = 12
	const TR_CUSTOM_SIZE : int = 16
	const TR_COLOR_CUSTOM_SIZE : int = 20
	
	var temp_buffer : PackedFloat32Array = multimesh.buffer
	temp_buffer.resize(temp_buffer.size() + amount*TR_CUSTOM_SIZE)
	
	multimesh.instance_count += amount
	#multimesh.visible_instance_count += amount
	
	multimesh.set_buffer(temp_buffer)

# for markers with unlimited duration, TIME will rollover every hour. so we need a way to skip past
#  the expansion animation that occurs at start_time every hour. can probably do this by adding or subtracting
#  1/expansion somewhere

# for new markers, we should iterate through the list, and reuse the first empty instance
# if there aren't enough instances, position data needs to be backed up, and the instance_count
#  resized, to fit the markers, then the markers put back.

#interface for the instance uniforms is 
#INSTANCE_CUSTOM (COLOR).x = type selection, .y = start time
