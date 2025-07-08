class_name SelectionMarkers
extends MultiMeshInstance3D

@export var manager : ProjectionManager

var time : float = 0.0
var loop_time : float = 0.0
const MIN_INSTANCE_COUNT : int = 32
# above this size, and clearing the selection will reset the buffer size to the min
#  so that it isn't kept large indefinitely
const INSTANCE_SOFT_CAP : int = 128 

#When the number of selections exceeds the current instance count, grow the instance buffer size
# by this much. Larger the number, the less often the buffer is resized, but there is a less fine grained number of instances.
const INSTANCE_COUNT_GROW_AMOUNT : int = 8 
const HEIGHT_ADD_FACTOR : Vector3 = ProjectionManager.HEIGHT_ADD_FACTOR

# we can control the multimesh buffer directly
# 12 floats for transform, 4 floats for colour, 4 floats for custom data
# colour and custom data are optional
const TR_CUSTOM_SIZE : int = 16
const MULTIMESH_START_TIME_OFFSET : int = 13

var id_to_index : Dictionary #int to int
var unused_indices : PackedInt32Array

const SELECT_TYPE : ProjectionManager.ProjectionType = ProjectionManager.ProjectionType.SELECTED
var select_index : int = 0

# Called when the node enters the scene tree for the first time.
func setup(loop_time_in : float = loop_time) -> void:
	select_index = manager.Type_to_Index[SELECT_TYPE]
	loop_time = loop_time_in
	# 1) setting instance_count clears and resizes the buffer
	# so we want to find the max size once and leave it
	# 2) resize must occur after setting the transform format
	
	#unlike billboards, we don't know how many of these we'll need. Quads are pretty small
	# and multimesh means we can have a lot of them, so be generous, and resize later if we have to
	multimesh.instance_count = MIN_INSTANCE_COUNT
	multimesh.visible_instance_count = MIN_INSTANCE_COUNT
	
	unused_indices.resize(MIN_INSTANCE_COUNT)
	for i : int in unused_indices.size():
		unused_indices[i] = i
		multimesh.set_instance_custom_data(i,Color(
			select_index,time,0.0,0.0
		))

#interface for the instance uniforms is 
#INSTANCE_CUSTOM (COLOR).x = type selection, .y = start time, .z = transparent override

func add_selection_marker(unit_id : int, unit_position : Vector3) -> bool:
	var index_to_use : int = -1
	if(id_to_index.get(unit_id,-1) != -1):
		return false # already has that id!
	
	for index : int in unused_indices:
		if index != -1:
			index_to_use = index
			unused_indices[index] = -1
			break
	if index_to_use == -1:
		index_to_use = unused_indices.size()
		_grow_buffer()
		unused_indices[index_to_use] = -1
	
	id_to_index[unit_id] = index_to_use
	multimesh.set_instance_transform(
		index_to_use,Transform3D(Basis(), unit_position + HEIGHT_ADD_FACTOR)
	)
	multimesh.set_instance_custom_data(index_to_use,Color(
		select_index,time,1.0,0.0
	))
	
	return true

func update_selection_marker(unit_id : int, unit_position : Vector3) -> bool:
	var index : int = id_to_index.get(unit_id,-1)
	if index == -1:
		return false #doesn't have that id!
	
	multimesh.set_instance_transform(
		index,Transform3D(Basis(), unit_position + HEIGHT_ADD_FACTOR)
	)
	return true

#TODO: Perhaps there's a more efficient method...
func add_selection_markers(unit_ids : PackedInt32Array, unit_positions : PackedVector3Array) -> bool:
	assert(unit_ids.size() == unit_positions.size())
	# don't increase visible count all at once here, in-case an id is invalid
	var ret : bool = true
	for index : int in unit_ids.size():
		if not add_selection_marker(unit_ids[index],unit_positions[index]):
			ret = false
	return ret

func update_selection_markers(unit_ids : PackedInt32Array, unit_positions : PackedVector3Array) -> bool:
	assert(unit_ids.size() == unit_positions.size())
	var ret : bool = true
	for index : int in unit_ids.size():
		if not update_selection_marker(unit_ids[index],unit_positions[index]):
			ret = false
	return ret

func clear_selection_markers() -> void:
	#no units should display
	if multimesh.buffer.size() > INSTANCE_SOFT_CAP*TR_CUSTOM_SIZE:
		#we exceeded the soft cap, reset instances to the minimum
		# now that we are clearing
		setup() 
		return
	else:
		for i : int in unused_indices.size():
			unused_indices[i] = i
	id_to_index.clear()
	for i : int in unused_indices.size():
		unused_indices[i] = i
		multimesh.set_instance_custom_data(i,Color(
			select_index,time,0.0,0.0
		))

func remove_selection_marker(unit_id : int) -> bool:
	var index : int = id_to_index.get(unit_id,-1)
	if index == -1:
		return false # doesn't have that id!
	unused_indices[index] = index #mark that index as unused
	id_to_index.erase(unit_id)
	multimesh.set_instance_custom_data(index,Color(
		select_index,time,0.0,0.0
	))
	return true

func is_id_selected(unit_id : int) -> bool:
	return id_to_index.has(unit_id)

func toggle_id_selected(unit_id : int, unit_position : Vector3) -> bool:
	if is_id_selected(unit_id):
		return remove_selection_marker(unit_id)
	else:
		return add_selection_marker(unit_id, unit_position)

func set_time(timeIn : float) -> void:
	time = timeIn
	if timeIn > loop_time:
		for i : int in id_to_index.values():
			if multimesh.buffer[TR_CUSTOM_SIZE*i + MULTIMESH_START_TIME_OFFSET] > 0:
				multimesh.buffer[TR_CUSTOM_SIZE*i + MULTIMESH_START_TIME_OFFSET] -= loop_time

#Grow the multimesh without losing the current data
func _grow_buffer(amount : int = INSTANCE_COUNT_GROW_AMOUNT) -> void:
	var temp_buffer : PackedFloat32Array = multimesh.buffer
	temp_buffer.resize(temp_buffer.size() + amount*TR_CUSTOM_SIZE)
	
	multimesh.instance_count += amount
	multimesh.visible_instance_count += amount
	multimesh.set_buffer(temp_buffer)
	
	var offset : int = unused_indices.size()
	unused_indices.resize(offset + amount)
	for i : int in amount:
		unused_indices[offset+i] = offset+i
