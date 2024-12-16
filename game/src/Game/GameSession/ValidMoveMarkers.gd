class_name ValidMoveMarkers
extends MultiMeshInstance3D

@export var manager : ProjectionManager

var projection_type_to_index : Dictionary

var time_legal : float = 0.0
var time_illegal : float = 0.0
var loop_time_legal : float = 0.0
var loop_time_illegal : float = 0.0

const INSTANCE_COUNT : int = 64
const HEIGHT_ADD_FACTOR : Vector3 = ProjectionManager.HEIGHT_ADD_FACTOR
var ages : PackedFloat32Array

const LEGAL_TYPE : ProjectionManager.ProjectionType = ProjectionManager.ProjectionType.LEGAL_MOVE
const ILLEGAL_TYPE : ProjectionManager.ProjectionType = ProjectionManager.ProjectionType.ILLEGAL_MOVE
var legal_index : int = 0
var illegal_index : int = 0

# Called when the node enters the scene tree for the first time.
func setup(loop_times : PackedFloat32Array) -> void:
	legal_index = manager.Type_to_Index[LEGAL_TYPE]
	illegal_index = manager.Type_to_Index[ILLEGAL_TYPE]

	loop_time_legal = loop_times[legal_index]
	loop_time_illegal = loop_times[illegal_index]

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
#INSTANCE_CUSTOM (COLOR).x = type selection, .y = start time, .z = transparent override

func add_move_marker(marker_position : Vector3, was_legal_move : bool) -> void:
	var type : ProjectionManager.ProjectionType = ILLEGAL_TYPE
	var time : float = time_illegal
	if was_legal_move:
		type = LEGAL_TYPE
		time = time_legal
	
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
				manager.Type_to_Index[type],time,1.0,0.0
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
		manager.Type_to_Index[type],time,1.0,0.0
	))
	ages[oldest] = duration

const TR_CUSTOM_SIZE : int = 16
const MULTIMESH_TYPE_OFFSET : int = 12
const MULTIMESH_START_TIME_OFFSET : int = 13

func set_time(times : PackedFloat32Array) -> void:
	time_legal = times[legal_index]
	time_illegal = times[illegal_index]
	
	var do_loop_legal : bool = time_legal > loop_time_legal
	var do_loop_illegal : bool = time_illegal > loop_time_illegal
	if do_loop_legal or do_loop_illegal:
		for i : int in ages.size():
			var type : int = multimesh.buffer[TR_CUSTOM_SIZE*i + MULTIMESH_TYPE_OFFSET] as int
			var start_time := multimesh.buffer[TR_CUSTOM_SIZE*i + MULTIMESH_START_TIME_OFFSET] 
			# note: only do the subtraction if start_time was not already looped
			# (which we can check by looking for a negative start_time)
			if do_loop_legal and type == legal_index and start_time > 0:
				multimesh.buffer[TR_CUSTOM_SIZE*i + MULTIMESH_START_TIME_OFFSET] = start_time-loop_time_legal
			elif do_loop_illegal and type == illegal_index and start_time > 0:
				multimesh.buffer[TR_CUSTOM_SIZE*i + MULTIMESH_START_TIME_OFFSET] = start_time-loop_time_illegal

func _process(delta : float) -> void:
	for i : int in ages.size():
		ages[i] -= delta
