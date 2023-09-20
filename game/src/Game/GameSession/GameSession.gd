extends Control

@export var _game_session_menu : Control

func _ready():
	GameSingleton.quazz_map_convert("res://common/map/provinces.json", "res://common/map/provinces.csv")
	Events.Options.load_settings_from_file()
	if GameSingleton.setup_game() != OK:
		push_error("Failed to setup game")

func _process(_delta : float):
	GameSingleton.try_tick()

# REQUIREMENTS:
# * SS-42
func _on_game_session_menu_button_pressed() -> void:
	_game_session_menu.visible = !_game_session_menu.visible
