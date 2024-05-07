extends Control

@export var _model_manager : ModelManager
@export var _game_session_menu : Control

func _ready() -> void:
	Events.Options.load_settings_from_file()
	if GameSingleton.setup_game(0) != OK:
		push_error("Failed to setup game")

	_model_manager.generate_units()
	_model_manager.generate_buildings()

func _process(_delta : float) -> void:
	GameSingleton.try_tick()

# REQUIREMENTS:
# * SS-42
func _on_game_session_menu_button_pressed() -> void:
	_game_session_menu.visible = !_game_session_menu.visible
