class_name MapText
extends Node3D

@export var _map_view : MapView

var _province_name_font : Font

const _province_name_scale : float = 1.0 / 48.0

func _ready() -> void:
	_province_name_font = AssetManager.get_font(&"mapfont_56")

func _clear_children() -> void:
	var child_count : int = get_child_count()
	while child_count > 0:
		child_count -= 1
		var child : Node = get_child(child_count)
		remove_child(child)
		child.queue_free()

func generate_map_names() -> void:
	_clear_children()

	for dict : Dictionary in GameSingleton.get_province_names():
		_add_province_name(dict)

func _add_province_name(dict : Dictionary) -> void:
	const identifier_key : StringName = &"identifier"
	const position_key : StringName = &"position"
	const rotation_key : StringName = &"rotation"
	const scale_key : StringName = &"scale"

	var label : Label3D = Label3D.new()

	label.set_draw_flag(Label3D.FLAG_DOUBLE_SIDED, false)
	label.set_modulate(Color.BLACK)
	label.set_outline_size(0)
	label.set_font(_province_name_font)
	label.set_vertical_alignment(VERTICAL_ALIGNMENT_BOTTOM)

	var identifier : String = dict[identifier_key]
	label.set_name(identifier)
	label.set_text(GUINode.format_province_name(identifier))

	label.set_position(_map_view._map_to_world_coords(dict[position_key]) + Vector3(0, 0.001, 0))

	label.rotate_x(-PI / 2)
	label.rotate_y(dict.get(rotation_key, 0.0))

	label.scale *= dict.get(scale_key, 1.0) * _province_name_scale

	add_child(label)
