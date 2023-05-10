extends Node

#the default value
const error_guiscale : float = 1

@export
var minimum_guiscale : float = 0.1

const _starting_guiscales : Dictionary = {
	float(0.5): &"0.5x",
	float(0.75): &"0.75x",
	float(1): &"1x",
	float(1.5): &"1.5x",
	float(2): &"2x",
}

var _guiscales: Dictionary

#Similar to Resolution.gd, but we don't bother checking for strings from files
#and we have floats instead of vector2 integers

func _ready():
	assert(minimum_guiscale > 0, "Minimum gui scale must be positive")
	for guiscale_value in _starting_guiscales:
		add_guiscale(guiscale_value, _starting_guiscales[guiscale_value])
	assert(not _guiscales.is_empty(), "No valid starting gui scales!")

func has_guiscale(guiscale_value : float) -> bool:
	return guiscale_value in _guiscales
	
func add_guiscale(guiscale_value: float, guiscale_name: StringName=&"") -> bool:
	if has_guiscale(guiscale_value): return true
	var scale_dict := { value = guiscale_value }
	var display_name := "%sx" % [guiscale_value]
	if not guiscale_name.is_empty():
		scale_dict.name = guiscale_name
		#don't need to change the display name
	scale_dict.display_name = StringName(display_name)
	if guiscale_value < minimum_guiscale:
		push_error("GUI scale %s is smaller than the minimum %s" % [scale_dict.display_name,minimum_guiscale])
		return false
	_guiscales[guiscale_value] = scale_dict
	return true
	
#returns floats
func get_guiscale_value_list() -> Array:
	var list := _guiscales.keys()
	list.sort_custom(func(a, b): return a > b)
	return list
	
func get_guiscale_display_name(guiscale_value : float) -> StringName:
	return _guiscales.get(guiscale_value, {display_name = &"unknown gui scale"}).display_name

func get_current_guiscale() -> float:
	return get_tree().root.content_scale_factor
	
func set_guiscale(guiscale:float) -> void:
	print("New GUI scale: %f" % guiscale)
	if not has_guiscale(guiscale):
		push_warning("Setting GUI Scale to non-standard value %sx" % [guiscale])
	get_tree().root.content_scale_factor = guiscale
	
func reset_guiscale() -> void:
	set_guiscale(get_current_guiscale())
	
