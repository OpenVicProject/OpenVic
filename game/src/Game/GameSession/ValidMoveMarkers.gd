class_name ValidMoveMarkers
extends MultiMeshInstance3D

@export var manager : ProjectionManager

var projection_type_to_index : Dictionary

var time : float = 0.0
const MIN_INSTANCE_COUNT : int = 32
const INSTANCE_COUNT_GROW_AMOUNT : int = 4
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
	
	ages.resize(MIN_INSTANCE_COUNT)
	ages.fill(0.0)
	
	#unlike billboards, we don't know how many of these we'll need. Quads are pretty small
	# and multimesh means we can have a lot of them, so be generous, and resize later if we have to
	multimesh.instance_count = MIN_INSTANCE_COUNT
	multimesh.visible_instance_count = MIN_INSTANCE_COUNT
	
	for i : int in MIN_INSTANCE_COUNT:
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
	
	var slot_found : bool = false
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
			slot_found = true
			break
			
	if !slot_found:
		_grow_buffer()
		
		var i = ages.size()
		ages.resize(i + INSTANCE_COUNT_GROW_AMOUNT)
		
		multimesh.set_instance_transform(
			i,Transform3D(Basis(), marker_position + HEIGHT_ADD_FACTOR)
		)
		multimesh.set_instance_custom_data(i,Color(
			manager.Type_to_Index[type],time,0.0,0.0
		))
		ages[i] = duration

func set_time(timeIn : float) -> void:
	time = timeIn

func _process(delta : float) -> void:
	for i : int in ages.size():
		ages[i] -= delta

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
	multimesh.visible_instance_count += amount
	multimesh.set_buffer(temp_buffer)
