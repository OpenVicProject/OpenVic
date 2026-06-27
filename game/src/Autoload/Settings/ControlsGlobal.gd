@tool
extends Node

var actions: Dictionary[StringName, Keychain.InputAction] = {}
var groups: Dictionary[StringName, Keychain.InputGroup] = {}

func _ready() -> void:
	actions = {
		# Map Group
		&"map_north": Keychain.InputAction.new("Move North", "Map", true),
		&"map_east": Keychain.InputAction.new("Move East", "Map", true),
		&"map_south": Keychain.InputAction.new("Move South", "Map", true),
		&"map_west": Keychain.InputAction.new("Move West", "Map", true),
		&"map_zoom_in": Keychain.InputAction.new("Zoom In", "Map", true),
		&"map_zoom_out": Keychain.InputAction.new("Zoom Out", "Map", true),
		&"map_drag": Keychain.InputAction.new("Mouse Drag", "Map", true),
		&"map_click": Keychain.InputAction.new("Mouse Click", "Map", true),
		&"map_right_click": Keychain.InputAction.new("Mouse Right Click", "Map", true),
		# Time Group
		&"time_pause": Keychain.InputAction.new("Pause", "Time", true),
		&"time_speed_increase": Keychain.InputAction.new("Speed Increase", "Time", true),
		&"time_speed_decrease": Keychain.InputAction.new("Speed Decrease", "Time", true),
		# UI Group
		&"menu_pause": Keychain.InputAction.new("Open Pause Menu", "UI", true),
	}

	groups = {
		&"Map": Keychain.InputGroup.new("", false),
		&"Time": Keychain.InputGroup.new("", false),
		&"UI": Keychain.InputGroup.new("", false),
		&"Hotkeys": Keychain.InputGroup.new(&"UI")
	}

	if Engine.is_editor_hint(): return
	Keychain.actions = actions
	Keychain.groups = groups
