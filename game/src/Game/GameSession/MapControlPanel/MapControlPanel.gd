extends PanelContainer

signal game_session_menu_button_pressed
signal map_view_camera_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2)
signal minimap_clicked(pos_clicked : Vector2)
signal zoom_in_button_pressed
signal zoom_out_button_pressed

@export var _mapmodes_grid : GridContainer

var _mapmode_button_group : ButtonGroup

# REQUIREMENTS:
# * UI-550, UI-552, UI-554, UI-561, UI-562, UI-563
func _add_mapmode_button(identifier : String) -> void:
	var button := Button.new()
	button.text = identifier
	button.tooltip_text = identifier
	button.toggle_mode = true
	button.button_group = _mapmode_button_group
	button.mouse_filter = MOUSE_FILTER_PASS
	button.focus_mode = FOCUS_NONE
	_mapmodes_grid.add_child(button)
	if _mapmode_button_group.get_pressed_button() == null:
		button.button_pressed = true

func _ready() -> void:
	_mapmode_button_group = ButtonGroup.new()
	_mapmode_button_group.pressed.connect(_mapmode_pressed)
	for index : int in GameSingleton.get_mapmode_count():
		_add_mapmode_button(GameSingleton.get_mapmode_identifier(index))

# REQUIREMENTS:
# * UIFUN-10
func _on_game_session_menu_button_pressed() -> void:
	game_session_menu_button_pressed.emit()

# REQUIREMENTS:
# * SS-76
# * UIFUN-129, UIFUN-131, UIFUN-133, UIFUN-140, UIFUN-141, UIFUN-142
func _mapmode_pressed(button : BaseButton) -> void:
	GameSingleton.set_mapmode(button.tooltip_text)

func _on_map_view_camera_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2) -> void:
	map_view_camera_changed.emit(near_left, far_left, far_right, near_right)

func _on_minimap_clicked(pos_clicked : Vector2) -> void:
	minimap_clicked.emit(pos_clicked)

# REQUIREMENTS:
# * UIFUN-269
func _on_zoom_in_button_pressed() -> void:
	zoom_in_button_pressed.emit()

# REQUIREMENTS:
# * UIFUN-270
func _on_zoom_out_button_pressed() -> void:
	zoom_out_button_pressed.emit()
