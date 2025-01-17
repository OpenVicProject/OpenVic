extends Node

class CompatCursor:
	#cursor properties
	var cursor_name : StringName

	var resolutions : PackedVector2Array
	var frames : Array[ImageTexture]
	var hotspots : PackedVector2Array
	var is_animated : bool = false
	var sequence : PackedInt32Array = [0]
	var timings : PackedFloat32Array = [1.0]
	
	#Cursor state
	var current_frame : int = 0
	var time_to_frame : float = 1.0
	
	func _init(name_in : StringName) -> void:
		cursor_name = name_in
		resolutions = CursorSingleton.get_resolutions(cursor_name)

		frames = CursorSingleton.get_frames(cursor_name,0)
		hotspots = CursorSingleton.get_hotspots(cursor_name,0)

		is_animated = len(frames) > 1
		if is_animated:
			sequence = CursorSingleton.get_sequence(cursor_name)
			timings = CursorSingleton.get_display_rates(cursor_name)
			time_to_frame = timings[sequence[current_frame]]
	
	func reset() -> void:
		current_frame = 0
		time_to_frame = timings[sequence[0]]
	
	func set_resolution(resolution : Vector2) -> void:
		var index : int = resolutions.find(resolution)
		if index != -1:
			frames = CursorSingleton.get_frames(cursor_name,index)
			return

		#couldnt find it, so generate it based on the highest res available
		var highest_res_index : int = 0
		var highest_res_x : int = 0
		for i : int in range(len(resolutions)):
			if resolutions[i].x > highest_res_x:
				highest_res_x = resolutions[i].x
				highest_res_index = i
		generate_new_resolution(highest_res_index,resolution)
		
		resolutions = CursorSingleton.get_resolutions(cursor_name)
		frames = CursorSingleton.get_frames(cursor_name,len(resolutions)-1)
		hotspots = CursorSingleton.get_hotspots(cursor_name,len(resolutions)-1)
		
		assert(len(frames) != 0)

	func generate_new_resolution(base_res_index : int, resolution : Vector2) -> void:
		# resolution wasn't in among the default, need to generate it ourselves
		CursorSingleton.generate_resolution(cursor_name,base_res_index,resolution)

	#only bother with this if the cursor is animated
	func _process_cursor(delta : float, shape : Input.CursorShape = Input.CURSOR_ARROW) -> void:
		time_to_frame -= delta
		if(time_to_frame <= 0):
			current_frame = (current_frame + 1) % len(sequence)
			time_to_frame += timings[sequence[current_frame]] 
			set_hardware_cursor(current_frame, shape)

	func set_hardware_cursor(frame : int=0, shape : Input.CursorShape = Input.CURSOR_ARROW) -> void:
		var texture : ImageTexture = frames[sequence[frame]]
		var hotspot : Vector2 = hotspots[sequence[frame]]
		Input.set_custom_mouse_cursor(texture,shape,hotspot)


#TODO: This is set on game start, but we probably want this to be a video setting
var preferred_resolution : Vector2 = Vector2(32,32)

var active_cursor : CompatCursor
var active_shape : Input.CursorShape

#Shape > Cursor dictionaries
var current_cursors : Dictionary = {
	Input.CURSOR_ARROW:null,
	Input.CURSOR_BUSY:null,
	Input.CURSOR_IBEAM:null
}
var queued_cursors : Dictionary = {
	Input.CURSOR_ARROW:null,
	Input.CURSOR_BUSY:null,
	Input.CURSOR_IBEAM:null
}
var loaded_cursors : Dictionary = {}

func load_cursors() -> void:
	CursorSingleton.load_cursors()
	for cursor_name : StringName in CursorSingleton.cursor_names:
		var cursor : CompatCursor = CompatCursor.new(cursor_name)
		cursor.set_resolution(preferred_resolution)
		loaded_cursors[cursor_name] = cursor


#Handle queued cursor changes and cursor animations
func _process(delta : float) -> void:
	var mouse_shape : Input.CursorShape = Input.get_current_cursor_shape()

	for shape in current_cursors.keys():
		if current_cursors[shape] != queued_cursors[shape]:
			current_cursors[shape] = queued_cursors[shape]
			if current_cursors[shape] != null:
				current_cursors[shape].set_hardware_cursor(0, shape)
			else:
				Input.set_custom_mouse_cursor(null, shape)

	#The mouse's cursor shape changed (something like we started hovering over text)
	# reset the current cursor's frame, then switch the active cursor
	if mouse_shape != active_shape:
		#Current mouse type changed, need to make sure that if the cursor of this new type
		# is animated, we are providing its frames instead of the frames of the previous active cursor
		active_shape = mouse_shape
		active_cursor = current_cursors.get(active_shape, null)
		if active_cursor != null:
			active_cursor.reset()
			active_cursor.set_hardware_cursor(0, active_shape)
		else:
			Input.set_custom_mouse_cursor(null, active_shape)

	#if we didnt change cursors and are animated, do an update
	elif active_cursor != null and active_cursor.is_animated:
		active_cursor._process_cursor(delta,active_shape)


func set_preferred_resolution(res_in : Vector2) -> void:
	preferred_resolution = res_in

func set_compat_cursor(cursor_name : StringName, shape : Input.CursorShape = Input.CURSOR_ARROW) -> void:
	if cursor_name in loaded_cursors:
		var cursor : CompatCursor = loaded_cursors[cursor_name]
		cursor.set_resolution(preferred_resolution)
		queued_cursors[shape] = cursor
	else:
		if cursor_name != &"":
			push_warning("Cursor name %s is not among loaded cursors" % cursor_name)
		queued_cursors[shape] = null

#NOTE: Each cursor has a corresponding "shape"
# to indicate when window is busy, normal, doing a drag-select, etc. 
# You can set this per Control Node under Mouse > Default Cursor Shape 

# set_compat_cursor makes the named vic2 cursor the presently active
# one for the shape it is currently associated with. By default a cursor
# is associated with Input.CURSOR_ARROW, but you can override this with the second
# argument. Use set_compat_cursor as you find it used here in initial_cursor_setup().

func initial_cursor_setup() -> void:
	set_preferred_resolution(Vector2(32,32))
	load_cursors()

	set_compat_cursor(&"normal")
	# When hovered over a control node with mouse shape set to "busy" (ie. loading screens)
	#  use the pocket watch cursor
	set_compat_cursor(&"busy", Input.CURSOR_BUSY)
