class_name SelectionMarkers
extends MultiMeshInstance3D

@export var manager : ProjectionManager

var time : float = 0.0
const MIN_INSTANCE_COUNT : int = 32
# above this size, and clearing the selection will reset things to the min
const MAX_INSTANCE_COUNT : int = 128 
#When the number of selections exceeds the current instance count, grow the instance buffer size
# by this much. Larger the number, the less often the buffer is resized, but there is a less fine grained number of instances.
const INSTANCE_COUNT_GROW_AMOUNT : int = 8 
const HEIGHT_ADD_FACTOR : Vector3 = Vector3(0,0.002,0)

var ids : PackedInt32Array
const SELECT_TYPE : ProjectionManager.ProjectionType = ProjectionManager.ProjectionType.SELECTED
var select_index : int = 0

# Called when the node enters the scene tree for the first time.
func setup() -> void:
	select_index = manager.Type_to_Index[SELECT_TYPE]
	# 1) setting instance_count clears and resizes the buffer
	# so we want to find the max size once and leave it
	# 2) resize must occur after setting the transform format
	
	ids.resize(MIN_INSTANCE_COUNT)
	ids.fill(-1)
	#unlike billboards, we don't know how many of these we'll need. Quads are pretty small
	# and multimesh means we can have a lot of them, so be generous, and resize later if we have to
	multimesh.instance_count = MIN_INSTANCE_COUNT
	multimesh.visible_instance_count = 0
	
	for i : int in MIN_INSTANCE_COUNT:
		multimesh.set_instance_transform(
			i,Transform3D(Basis(), Vector3(float(i),0.0,0.0))
		)

# for markers with unlimited duration, time will rollover eventually. so we need a way to skip past
#  the expansion animation that occurs at start_time every hour. can probably do this by adding or subtracting
#  1/expansion somewhere

#interface for the instance uniforms is 
#INSTANCE_CUSTOM (COLOR).x = type selection, .y = start time

func add_selection_marker(unit_id : int, unit_position : Vector3) -> void:
	var slot_found : bool = false
	for i : int in ids.size():
		var id : int = ids[i]
		if id < 0:
			multimesh.set_instance_transform(
				i,Transform3D(Basis(), unit_position + HEIGHT_ADD_FACTOR)
			)
			multimesh.set_instance_custom_data(i,Color(
				select_index,time,1.0,0.0
			))
			ids[i] = unit_id
			slot_found = true
			break
			
	if !slot_found:
		_grow_buffer()
		
		var slot_index : int = ids.size()
		ids.resize(slot_index-1 + INSTANCE_COUNT_GROW_AMOUNT)
		
		multimesh.set_instance_transform(
			slot_index,Transform3D(Basis(), unit_position + HEIGHT_ADD_FACTOR)
		)
		multimesh.set_instance_custom_data(slot_index,Color(
			select_index,time,2.0,0.0
		))
		ids[slot_index] = unit_id
		for j : int in ids.size() - slot_index:
			ids[slot_index + j] = -1
	multimesh.visible_instance_count += 1

func update_selection_marker(unit_id : int, unit_position : Vector3) -> void:
	var index : int = ids.find(unit_id)
	multimesh.set_instance_transform(
		index,Transform3D(Basis(), unit_position + HEIGHT_ADD_FACTOR)
	)

#TODO: Perhaps there's a more efficient method...
func add_selection_markers(unit_ids : PackedInt32Array, unit_positions : PackedVector3Array) -> void:
	assert(unit_ids.size() == unit_positions.size())
	multimesh.visible_instance_count += unit_positions.size()
	for index : int in unit_ids.size():
		add_selection_marker(unit_ids[index],unit_positions[index])

func update_selection_markers(unit_ids : PackedInt32Array, unit_positions : PackedVector3Array) -> void:
	assert(unit_ids.size() == unit_positions.size())
	for index : int in unit_ids.size():
		update_selection_marker(unit_ids[index],unit_positions[index])

func clear_selection_markers() -> void:
	#no units should display
	if multimesh.buffer.size() > MAX_INSTANCE_COUNT:
		setup()
		return
	ids.fill(-1)
	multimesh.visible_instance_count = 0


func set_time(timeIn : float) -> void:
	time = timeIn

#Grow the multimesh without losing the current data
func _grow_buffer(amount : int = INSTANCE_COUNT_GROW_AMOUNT) -> void:
	# we can control the multimesh buffer directly
	# 12 floats for transform, 4 floats for colour, 4 floats for custom data
	# colour and custom data are optional
	const TR_CUSTOM_SIZE : int = 16
	
	var temp_buffer : PackedFloat32Array = multimesh.buffer
	temp_buffer.resize(temp_buffer.size() + amount*TR_CUSTOM_SIZE)
	
	multimesh.instance_count += amount
	multimesh.set_buffer(temp_buffer)
