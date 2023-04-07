extends PanelContainer

signal game_session_menu_button_pressed
signal mapmode_changed
signal map_view_camera_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2)
signal minimap_clicked(pos_clicked : Vector2)

@export var _mapmodes_grid : GridContainer

var _mapmode_button_group : ButtonGroup

# REQUIREMENTS:
# * UI-550, UI-554
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

# REQUIREMENTS:
# * SS-76
# * UIFUN-129, UIFUN-133
func _mapmode_pressed(button : BaseButton) -> void:
	MapSingleton.set_mapmode(button.tooltip_text)
	mapmode_changed.emit()

func _on_map_view_camera_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2) -> void:
	map_view_camera_changed.emit(near_left, far_left, far_right, near_right)

func _on_minimap_clicked(pos_clicked : Vector2) -> void:
	minimap_clicked.emit(pos_clicked)
