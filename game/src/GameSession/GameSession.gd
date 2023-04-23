extends Control

@export var _game_session_menu : Control

func _ready():
	Events.Options.load_settings_from_file()

func _process(delta : float):
	GameSingleton.try_tick()

# REQUIREMENTS:
# * SS-42
func _on_game_session_menu_button_pressed() -> void:
	_game_session_menu.visible = !_game_session_menu.visible
