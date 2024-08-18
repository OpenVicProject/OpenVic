extends Node

class compat_Cursor:
	#cursor properties
	var cursor_name:String
	var resolutions:Array[Vector2i]
	var numFrames:int = 1
	var is_animated:bool = false
	var sequence:Array[int] = [1]
	var timings:Array[float] = [1.0]
	var shape:Input.CursorShape = Input.CURSOR_ARROW
	
	#TODO: Should this be kept, improved? this can only store 1 set of new cursors
	#if the prefered resolution doesn't exist, generate new frames and hotspots
	#and store them here
	var custom_res_frames:Array[ImageTexture] = []
	var custom_res_hotspots:Array[Vector2i] = []
	
	#Cursor state
	var picked_resolution_index : int = 0
	var currentFrame : int = 0
	var timeToFrame : float = 1.0
	
	func _init(nameIn:String, shape:Input.CursorShape = Input.CURSOR_ARROW) -> void:
		self.cursor_name = nameIn
		self.resolutions = CursorSingleton.get_resolutions(self.cursor_name)
		self.numFrames = CursorSingleton.get_animationLength(self.cursor_name)
		self.picked_resolution_index = 0
		self.shape = shape

		self.is_animated = self.numFrames > 1
		if self.is_animated:
			self.sequence = CursorSingleton.get_sequence(self.cursor_name)
			self.timings = CursorSingleton.get_displayRates(self.cursor_name)
			self.timeToFrame = self.timings[self.sequence[currentFrame]]
		
	func set_resolution(resolution : Vector2i) -> void:
		var index = resolutions.find(resolution)
		if index != -1:
			picked_resolution_index = index
			return
		if len(custom_res_frames) > 0 and Vector2i(custom_res_frames[0].get_size()) == resolution:
			picked_resolution_index = -1
			return
		
		generate_new_res(picked_resolution_index,resolution)
		picked_resolution_index = -1
		
	func generate_new_res(base_res_index:int, resolution:Vector2i):
		# resolution wasn't in among the default, need to generate it ourselves
		for i in range(numFrames):
			var tex = get_image(i,picked_resolution_index)
			var image = Image.new()
			image.copy_from(tex.get_image())
			image.resize(resolution.x,resolution.y,Image.INTERPOLATE_BILINEAR)
			var orig_res:Vector2i = tex.get_size()
			var hotspot = get_hotspot(i,picked_resolution_index)
			
			var y = ImageTexture.new()
			custom_res_frames.push_back(y.create_from_image(image))
			custom_res_hotspots.push_back(hotspot * (resolution/orig_res))

	#only bother with this if the cursor is animated
	func _process_cursor(delta:float, advanceFrame:bool=true) -> void:
		timeToFrame -= delta
		if(timeToFrame <= 0):
			if advanceFrame:
				currentFrame = (currentFrame + 1) % self.numFrames
			
			# we dont use _calculate_index here because that's for images which
			# correspond to both an animation frame and a resolution, whereas timings
			# only correspond to an animation frame
			timeToFrame = timings[self.sequence[currentFrame]] 
			set_hardware_cursor(currentFrame)

	func set_hardware_cursor(frame:int=0) -> void:
		if picked_resolution_index == -1:
			#if this is a custom resolution, skip the _calculate_index call
			#made to find the correct index when there are multiple resolutions
			#contained in a list, just get the frame in the custom array
			if frame < len(custom_res_frames):
				var texture = custom_res_frames[frame]
				var hotspot = custom_res_hotspots[frame]
				Input.set_custom_mouse_cursor(texture,shape,hotspot)
			else:
				push_warning("error fetching cursor image for custom resolution: frame number too large")
			return
		
		var texture = get_image(frame,picked_resolution_index)
		var hotspot = get_hotspot(frame,picked_resolution_index)

		Input.set_custom_mouse_cursor(texture,shape,hotspot)

	func get_image(frame:int=0, resolution_index:int=0) -> ImageTexture:
		if frame < len(sequence):
			return CursorSingleton.get_image(self.cursor_name,_calculate_index(frame,resolution_index))
		else:
			push_warning("Error fetching cursor image: Frame number was larger than sequence length %s > %s" % [frame,len(self.sequence)])
		return null
		
	func get_hotspot(frame:int=0, resolution_index=0) -> Vector2i:
		if frame < len(sequence):
			return CursorSingleton.get_hotspot(self.cursor_name,_calculate_index(frame,resolution_index))
		else:
			push_warning("Error fetching cursor hotspot: Frame number was larger than sequence length %s > %s" % [frame,len(self.sequence)])
		return Vector2i(0,0)
		
	func _calculate_index(frame:int=0, resolution_index=0) -> int:
		if !is_animated:
			return resolution_index
		return resolution_index + len(resolutions)*sequence[frame]

#Cursor singleton is capable of loading the data for images
#but managing animated cursors must be done in an autoload (part of the scene tree)

var mouseOverWindow : bool = false
var windowFocused : bool = false
var activeCursor : compat_Cursor

#TODO: This is set on game start, but we probably want this to be a video setting
var preferred_res : Vector2i = Vector2i(32,32)

#Shape > Cursor dictionnaries
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

#temp
var cur_ind = 0
var cooldown = 0.0

#Handle queued cursor changes and cursor animations
func _process(delta) -> void:
	#test code
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

		#instead of changeQueued, we need to check if the queued cursor pointer matches the current pointer
		for shape in currentCursors.keys():
			if currentCursors[shape] != queuedCursors[shape] and queuedCursors[shape] != null:
				currentCursors[shape] = queuedCursors[shape]
				currentCursors[shape].set_hardware_cursor(0)
				if mouseShape == shape:
					advanceFrame = false
		
		if activeCursor != null and mouseShape != activeCursor.shape:
			#Current mouse type changed, need to make sure that if the cursor of this new type
			# is animated, we are providing its frames instead of the frames of the previous active cursor
			activeCursor.currentFrame = 0 # reset the frame in the sequence to use
			activeCursor.set_hardware_cursor(0)
			
			if mouseShape in currentCursors:
				activeCursor = currentCursors[mouseShape]
				activeCursor.currentFrame = 0
			advanceFrame = false
			
		#if we didnt change cursors, do an update
		if activeCursor != null and activeCursor.is_animated:
			activeCursor._process_cursor(delta,advanceFrame)
	

func set_prefered_res(res_in:Vector2i) -> void:
	preferred_res = res_in

#override_other_queued is to stop an animation frame from taking precedence over
#a cursor switch
func set_compat_cursor(cursor_name:String, cursor_shape:Input.CursorShape = Input.CURSOR_ARROW) -> void:
	activeCursor = compat_Cursor.new(cursor_name,cursor_shape)
	activeCursor.set_resolution(preferred_res)
	set_mouse_cursor(activeCursor)
	
# To safely change the mouse cursor, the mouse must be over the window
# these 2 functions help ensure we do it safely
func set_mouse_cursor(cursor:compat_Cursor) -> void:
	if mouseOverWindow and windowFocused:
		activeCursor = cursor
		currentCursors[cursor.shape] = cursor
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
