extends PanelContainer

signal game_session_menu_button_pressed
signal mapmode_changed

@export var _mapmodes_grid : GridContainer
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
