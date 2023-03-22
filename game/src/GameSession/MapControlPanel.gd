extends PanelContainer

signal game_session_menu_button_pressed

# REQUIREMENTS:
# * UIFUN-10
func _on_game_session_menu_button_pressed():
	game_session_menu_button_pressed.emit()
