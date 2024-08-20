extends Node

class compat_Cursor:
	#cursor properties
	var cursor_name:String
	var shape:Input.CursorShape = Input.CURSOR_ARROW
	
	var resolutions:Array[Vector2i]
	var frames:Array
	var hotspots:Array[Vector2i]
	var is_animated:bool = false
	var sequence:Array[int] = [0]
	var timings:Array[float] = [1.0]
	
	#Cursor state
	var currentFrame : int = 0
	var timeToFrame : float = 1.0
	
	func _init(nameIn:String, shapeIn:Input.CursorShape = Input.CURSOR_ARROW) -> void:
		self.cursor_name = nameIn
		self.resolutions = CursorSingleton.get_resolutions(self.cursor_name)

		self.shape = shapeIn
		self.frames = CursorSingleton.get_frames(self.cursor_name,0)
		self.hotspots = CursorSingleton.get_hotspots(self.cursor_name,0)

		self.is_animated = len(frames) > 1
		if self.is_animated:
			self.sequence = CursorSingleton.get_sequence(self.cursor_name)
			self.timings = CursorSingleton.get_displayRates(self.cursor_name)
			self.timeToFrame = self.timings[self.sequence[currentFrame]]
	
	func reset():
		currentFrame = 0
		timeToFrame = self.timings[self.sequence[0]]
	
	func set_resolution(resolution : Vector2i) -> void:
		var index = resolutions.find(resolution)
		if index != -1:
			frames = CursorSingleton.get_frames(cursor_name,index)
			return

		#couldnt find it, so generate it based on the highest res available
		var highest_res_ind:int = 0
		var highest_res_x = 0
		for i in range(len(resolutions)):
			if resolutions[i].x > highest_res_x:
				highest_res_x = resolutions[i].x
				highest_res_ind = i
		generate_new_res(highest_res_ind,resolution)
		
		self.resolutions = CursorSingleton.get_resolutions(self.cursor_name)
		self.frames = CursorSingleton.get_frames(self.cursor_name,len(self.resolutions)-1)
		self.hotspots = CursorSingleton.get_hotspots(self.cursor_name,len(self.resolutions)-1)
		
		assert(len(self.frames ) != 0)

		
	func generate_new_res(base_res_index:int, resolution:Vector2i) -> void:
		# resolution wasn't in among the default, need to generate it ourselves
		CursorSingleton.generate_resolution(cursor_name,base_res_index,resolution)

	#only bother with this if the cursor is animated
	func _process_cursor(delta:float, advanceFrame:bool=true) -> void:
		timeToFrame -= delta
		if(timeToFrame <= 0):
			if advanceFrame:
				currentFrame = (currentFrame + 1) % len(sequence)
			timeToFrame = timings[self.sequence[currentFrame]] 
			set_hardware_cursor(currentFrame)

	func set_hardware_cursor(frame:int=0) -> void:
		var texture = frames[sequence[frame]]
		var hotspot = hotspots[sequence[frame]]
		Input.set_custom_mouse_cursor(texture,shape,hotspot)


#Cursor singleton is capable of loading the data for images
#but managing animated cursors must be done in an autoload (part of the scene tree)

var mouseOverWindow : bool = false
var windowFocused : bool = false
var activeCursor : compat_Cursor

#TODO: This is set on game start, but we probably want this to be a video setting
var preferred_res : Vector2i = Vector2i(32,32)

#Shape > Cursor dictionnaries
#NOTE: In terms of V2, this is unnecessary, as the only cursor that isn't of shape
#"arrow" in v2 is "busy", which needs to be manually triggered anyways like arrow.
#This is needed for shapes like IBEAM which get switched to automatically
var currentCursors : Dictionary = {
	Input.CURSOR_ARROW:null,
	Input.CURSOR_BUSY:null,
	Input.CURSOR_IBEAM:null
}
var queuedCursors : Dictionary = {
	Input.CURSOR_ARROW:null,
	Input.CURSOR_BUSY:null,
	Input.CURSOR_IBEAM:null
}

var loaded_cursors : Dictionary = {}

func load_cursors() -> void:
	CursorSingleton.load_cursors()
	for cursor_name in CursorSingleton.cursor_names:
		var shape:Input.CursorShape = Input.CURSOR_ARROW
		if cursor_name == "busy":
			shape = Input.CURSOR_BUSY
		var cursor = compat_Cursor.new(cursor_name,shape)
		cursor.set_resolution(preferred_res)
		loaded_cursors[cursor_name] = cursor

#temp TODO: Remove when done testing
var cur_ind = 0
var cooldown = 0.0

#Handle queued cursor changes and cursor animations
func _process(delta) -> void:
	#TODO: Remove test code when done testing
	cooldown -= delta
	var cursor_names = [
		"aero_busy", "drum", "aero_link_i",
		"attack_move","busy","cant_move","deploy_not_valid","deploy_valid","dragselect",
		"embark","exploration","friendly_move","no_move","normal","objective","selected"
	]
	if Input.is_mouse_button_pressed(MOUSE_BUTTON_RIGHT) and cooldown <= 0:
		cooldown = 0.3
		cur_ind = (cur_ind + 1) % cursor_names.size()
		set_compat_cursor(cursor_names[cur_ind])
	#end test code

	#only attempt to update the mouse when this wont crash anything
	if mouseOverWindow:
		var advanceFrame = true #dont go to next frame if we just switched cursors
		var mouseShape:Input.CursorShape = Input.get_current_cursor_shape()

		for shape in currentCursors.keys():
			if currentCursors[shape] != queuedCursors[shape] and queuedCursors[shape] != null:
				currentCursors[shape] = queuedCursors[shape]
				currentCursors[shape].set_hardware_cursor()
				#this is the currently active shape, set the active cursor, make it frame 0 and 
				#make sure we dont skip this frame
				if mouseShape == shape:
					advanceFrame = false
					activeCursor = currentCursors[shape]
					activeCursor.reset()
		
		#The mouse's cursor shape changed (something like we started hovering over text)
		# reset the current cursor's frame, then switch the active cursor
		if activeCursor != null and mouseShape != activeCursor.shape:
			#Current mouse type changed, need to make sure that if the cursor of this new type
			# is animated, we are providing its frames instead of the frames of the previous active cursor
			activeCursor.reset() # reset the frame in the sequence to use
			activeCursor.set_hardware_cursor()
			
			if mouseShape in currentCursors and currentCursors[mouseShape] != null:
				activeCursor = currentCursors[mouseShape]
				activeCursor.reset()
			advanceFrame = false
			
		#if we didnt change cursors and are animated, do an update
		if activeCursor != null and activeCursor.is_animated:
			activeCursor._process_cursor(delta,advanceFrame)
	

func set_prefered_res(res_in:Vector2i) -> void:
	preferred_res = res_in

#override_other_queued is to stop an animation frame from taking precedence over
#a cursor switch
func set_compat_cursor(cursor_name:String, cursor_shape:Input.CursorShape = -1) -> void:
	#var cursor = compat_Cursor.new(cursor_name,cursor_shape)
	if cursor_name in loaded_cursors:
		var cursor = loaded_cursors[cursor_name]
		if cursor_shape != -1:
			cursor.shape = cursor_shape 
		cursor.set_resolution(preferred_res)
		set_mouse_cursor(cursor)
	else:
		push_warning("Cursor name %s is not among loaded cursors" % cursor_name)
	
# To safely change the mouse cursor, the mouse must be over the window
# these 2 functions help ensure we do it safely
func set_mouse_cursor(cursor:compat_Cursor) -> void:
	if mouseOverWindow and windowFocused:
		activeCursor = cursor
		activeCursor.currentFrame = 0
		currentCursors[cursor.shape] = cursor
		queuedCursors[cursor.shape] = null
		activeCursor.set_hardware_cursor(0)
	else:
		queuedCursors[cursor.shape] = cursor

func _notification(what):
	match(what):
		NOTIFICATION_WM_MOUSE_ENTER:
			mouseOverWindow = true
		NOTIFICATION_WM_MOUSE_EXIT:
			mouseOverWindow = false
		NOTIFICATION_WM_WINDOW_FOCUS_IN:
			windowFocused = true
		NOTIFICATION_WM_WINDOW_FOCUS_OUT:
			windowFocused = false
