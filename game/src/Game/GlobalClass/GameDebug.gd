class_name GameDebug
extends RefCounted

static var _singleton: GameDebug

static var debug_mode: bool:
	get = is_debug_mode,
	set = set_debug_mode


static func set_debug_mode(value: bool) -> void:
	if _singleton == null:
		push_warning("Debug mode could not be set.")
		return
	_singleton._set_debug_mode(value)


static func is_debug_mode() -> bool:
	if _singleton == null:
		push_warning("Could not get debug mode, returning false.")
		return false
	return _singleton._is_debug_mode()


func _set_debug_mode(value: bool) -> void:
	ArgumentParser.set_argument(&"game-debug", value)
	print("Set debug mode to: ", value)


func _is_debug_mode() -> bool:
	return ArgumentParser.get_argument(&"game-debug", false)
