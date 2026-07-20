@tool
extends Node

const _Keychain = preload("res://addons/keychain/Keychain.gd")

var actions: Dictionary[StringName, _Keychain.InputAction] = {}
var groups: Dictionary[StringName, _Keychain.InputGroup] = {}


func _ready() -> void:
	actions = {
		# Map Group
		&"map_north": _Keychain.InputAction.new("Move North", "Map", true),
		&"map_east": _Keychain.InputAction.new("Move East", "Map", true),
		&"map_south": _Keychain.InputAction.new("Move South", "Map", true),
		&"map_west": _Keychain.InputAction.new("Move West", "Map", true),
		&"map_zoom_in": _Keychain.InputAction.new("Zoom In", "Map", true),
		&"map_zoom_out": _Keychain.InputAction.new("Zoom Out", "Map", true),
		&"map_drag": _Keychain.InputAction.new("Mouse Drag", "Map", true),
		&"map_click": _Keychain.InputAction.new("Mouse Click", "Map", true),
		&"map_right_click": _Keychain.InputAction.new("Mouse Right Click", "Map", true),
		# Time Group
		&"time_pause": _Keychain.InputAction.new("Pause", "Time", true),
		&"time_speed_increase": _Keychain.InputAction.new("Speed Increase", "Time", true),
		&"time_speed_decrease": _Keychain.InputAction.new("Speed Decrease", "Time", true),
		# UI Group
		&"menu_pause": _Keychain.InputAction.new("Open Pause Menu", "UI", true),
	}

	groups = {
		&"Map": _Keychain.InputGroup.new("", false),
		&"Time": _Keychain.InputGroup.new("", false),
		&"UI": _Keychain.InputGroup.new("", false),
		&"Hotkeys": _Keychain.InputGroup.new(&"UI"),
	}

	if Engine.is_editor_hint(): return
	Keychain.actions = actions
	Keychain.groups = groups
