extends Control

@export var _model_manager : ModelManager
@export var _game_session_menu : Control

func _ready() -> void:
	Events.Options.load_settings_from_file()
	if GameSingleton.start_game_session() != OK:
		push_error("Failed to setup game")

	_model_manager.generate_units()
	_model_manager.generate_buildings()
	MusicConductor.generate_playlist()
	MusicConductor.select_next_song()

	Keychain.initialize_profiles()

func _process(_delta : float) -> void:
	GameSingleton.update_clock()

# REQUIREMENTS:
# * SS-42
func _on_game_session_menu_button_pressed() -> void:
	_game_session_menu.visible = !_game_session_menu.visible
