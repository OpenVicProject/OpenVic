extends Control

@export var _game_session_menu : Control

func _ready():
	Events.Options.load_settings_from_file()

# REQUIREMENTS:
# * SS-42
func _on_game_session_menu_button_pressed():
	_game_session_menu.visible = !_game_session_menu.visible
