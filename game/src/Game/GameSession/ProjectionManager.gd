extends Node3D
class_name ProjectionManager

enum ProjectionType {INVALIDTYPE, SELECTED, LEGAL_MOVE, ILLEGAL_MOVE}
const PROJECTION_NAME_TO_TYPES : Dictionary = {
	&"selection_projection": ProjectionType.SELECTED,
	&"legal_selection_projection": ProjectionType.LEGAL_MOVE,
	&"illegal_selection_projection": ProjectionType.ILLEGAL_MOVE
}

@export var selectionMarkers : SelectionMarkers
@export var moveMarkers : ValidMoveMarkers

var Type_to_Index : Dictionary

var material : ShaderMaterial
var textures : Array[Texture2D]
var sizes : PackedFloat32Array
var spins : PackedFloat32Array
var expansions : PackedFloat32Array
var durations : PackedFloat32Array
var transparency_mode : PackedByteArray

const SCALE_FACTOR : float = 1.0 / 256.0
const SPIN_FACTOR : float = 4.0
const HEIGHT_ADD_FACTOR : Vector3 = Vector3(0,0.002,0)
const GROW_FACTOR : float = 0.25

var times : PackedFloat32Array = [0,0,0]
var loop_times : PackedFloat32Array = [0,0,0]

# For the markers (selection compass, legal/illegal move markers)
# this class handles the setup of the projection shader, and the maintenance
# of the time variable for all 3 scripts.
#  the two multimeshes handle the individual instances.


func _ready() -> void:
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
		var projection_type : ProjectionType = PROJECTION_NAME_TO_TYPES.get(projection_name, ProjectionType.INVALIDTYPE)
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

		# We use the texture array size (which will be the same as frames and scales' sizes)
		# rather than projection_index as the former only counts projections we're actually using,
		# while the latter counts all projections defined in the game's GFX files
		Type_to_Index[projection_type] = textures.size()

		textures.push_back(texture)
		sizes.push_back(size * SCALE_FACTOR)
		spins.push_back(spin * SPIN_FACTOR)
		expansions.push_back(expanding * GROW_FACTOR)
		durations.push_back(duration)
		transparency_mode.push_back(additative)
		
		var loop_time : float = 2*PI*size/(expanding*GROW_FACTOR)
		if duration != 0:
			loop_time *= duration
		loop_times[Type_to_Index[projection_type]] = loop_time

	material = moveMarkers.multimesh.mesh.surface_get_material(0)
	if material == null:
		push_error("ShaderMaterial for projections was null")
		return

	material.set_shader_parameter(&"projections", textures)
	material.set_shader_parameter(&"sizes", sizes)
	material.set_shader_parameter(&"spin", spins)
	material.set_shader_parameter(&"expanding", expansions)
	material.set_shader_parameter(&"additative", transparency_mode)
	material.set_shader_parameter(&"duration",durations)
	material.set_shader_parameter(&"time", times)
	
	# to be safe, set selectionMarkers to be the same material
	selectionMarkers.multimesh.mesh.surface_set_material(0, material)

	moveMarkers.setup(loop_times)
	selectionMarkers.setup(loop_times[Type_to_Index[ProjectionType.SELECTED]])

func _process(delta : float) -> void:
	for i : int in times.size():
		times[i] += delta
	
	material.set_shader_parameter(&"time", times)
	moveMarkers.set_time(times)
	selectionMarkers.set_time(times[Type_to_Index[ProjectionType.SELECTED]])
	
	for i : int in times.size():
		if times[i] >= loop_times[i]:
			# use this instead of setting to 0 so that any fractional
			# component rolls over with the projection
			times[i] -= loop_times[i]
