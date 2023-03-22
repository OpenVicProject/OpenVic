extends Control

@export var _game_session_menu : Control

func _ready():
	print("GameSession ready")

# REQUIREMENTS:
# * SS-42
func _on_game_session_menu_button_pressed():
	_game_session_menu.visible = !_game_session_menu.visible

# REQUIREMENTS:
# * SS-64
func _on_game_session_menu_close_button_pressed():
	_game_session_menu.hide()
