extends PanelContainer

signal game_session_menu_button_pressed

func _on_game_session_menu_button_pressed():
	game_session_menu_button_pressed.emit()
