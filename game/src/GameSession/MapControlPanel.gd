extends PanelContainer

signal game_session_menu_button_pressed
signal mapmode_changed
signal camera_change(_camera_pos_clicked: Vector2)

const _action_click : StringName = &"map_click"

@export var _mapmodes_grid : GridContainer
@export var _minimap : PanelContainer
@onready var _map_camera : Control = _minimap.get_node("RectangularCamera")

var _mouse_inside: bool = false

var _mapmode_button_group : ButtonGroup

func _add_mapmode_button(identifier : String) -> void:
	var button := Button.new()
	button.text = identifier
	button.tooltip_text = identifier
	button.toggle_mode = true
	button.button_group = _mapmode_button_group
	_mapmodes_grid.add_child(button)
	if _mapmode_button_group.get_pressed_button() == null:
		button.button_pressed = true

func _ready():
	_mapmode_button_group = ButtonGroup.new()
	_mapmode_button_group.pressed.connect(_mapmode_pressed)
	for index in MapSingleton.get_mapmode_count():
		_add_mapmode_button(MapSingleton.get_mapmode_identifier(index))

# REQUIREMENTS:
# * UIFUN-10
func _on_game_session_menu_button_pressed() -> void:
	game_session_menu_button_pressed.emit()

func _mapmode_pressed(button : BaseButton) -> void:
	MapSingleton.set_mapmode(button.tooltip_text)
	mapmode_changed.emit()

func _on_map_view_map_view_camera_changed(near_left, far_left, far_right, near_right):
	_map_camera._on_camera_view_changed(near_left, far_left, far_right, near_right)

func _process(delta):
	if _mouse_inside and Input.is_action_pressed(_action_click):
		camera_change.emit($VBoxContainer/Minimap.get_local_mouse_position())
		
func _on_minimap_mouse_entered():
	_mouse_inside = true

func _on_minimap_mouse_exited():
	_mouse_inside = false
