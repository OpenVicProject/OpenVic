class_name ValidMoveMarkers
extends MultiMeshInstance3D

@export var manager : ProjectionManager

var projection_type_to_index : Dictionary

var time : float = 0.0
const INSTANCE_COUNT : int = 64
const HEIGHT_ADD_FACTOR : Vector3 = Vector3(0,0.002,0)
var ages : PackedFloat32Array

const LEGAL_TYPE : ProjectionManager.ProjectionType = ProjectionManager.ProjectionType.LEGAL_MOVE
const ILLEGAL_TYPE : ProjectionManager.ProjectionType = ProjectionManager.ProjectionType.ILLEGAL_MOVE
var legal_index : int = 0
var illegal_index : int = 0

# Called when the node enters the scene tree for the first time.
func setup() -> void:
	legal_index = manager.Type_to_Index[LEGAL_TYPE]
	illegal_index = manager.Type_to_Index[ILLEGAL_TYPE]

	ages.resize(INSTANCE_COUNT)
	ages.fill(0.0)
	
	#unlike billboards, we don't know how many of these we'll need. Quads are pretty small
	# and multimesh means we can have a lot of them, so be generous, and resize later if we have to
	multimesh.instance_count = INSTANCE_COUNT
	multimesh.visible_instance_count = INSTANCE_COUNT
	
	for i : int in INSTANCE_COUNT:
		multimesh.set_instance_transform(
			i,Transform3D(Basis(), Vector3(float(i),0.0,0.0))
		)

#interface for the instance uniforms is 
#INSTANCE_CUSTOM (COLOR).x = type selection, .y = start time

func add_move_marker(marker_position : Vector3, was_legal_move : bool) -> void:
	var type : ProjectionManager.ProjectionType = ILLEGAL_TYPE
	if was_legal_move:
		type = LEGAL_TYPE
	
	var duration : float = float(manager.durations[manager.Type_to_Index[type]])

	var oldest : int = 0
	var oldest_age : float = 0
	for i : int in ages.size():
		var age : float = ages[i]
		if age <= 0:
			multimesh.set_instance_transform(
				i,Transform3D(Basis(), marker_position + HEIGHT_ADD_FACTOR)
			)
			multimesh.set_instance_custom_data(i,Color(
				manager.Type_to_Index[type],time,0.0,0.0
			))
			ages[i] = duration
			return
		if age < oldest_age:
			oldest_age = age
			oldest = i
	# all slots were filled, use the oldest
	multimesh.set_instance_transform(
		oldest,Transform3D(Basis(), marker_position + HEIGHT_ADD_FACTOR)
	)
	multimesh.set_instance_custom_data(oldest,Color(
		manager.Type_to_Index[type],time,0.0,0.0
	))
	ages[oldest] = duration

func set_time(timeIn : float) -> void:
	time = timeIn

func _process(delta : float) -> void:
	for i : int in ages.size():
		ages[i] -= delta
