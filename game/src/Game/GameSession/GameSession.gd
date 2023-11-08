extends Control

@export var _game_session_menu : Control

func _ready():
	Events.Options.load_settings_from_file()
	if GameSingleton.setup_game() != OK:
		push_error("Failed to setup game")

	# Temporarily here for cosmetic reasons, will be moved to its
	# own child node later, similar to ProvinceOverviewPanel
	add_child(GameSingleton.generate_gui("topbar.gui", "topbar"))
	$topbar/topbar_outlinerbutton_bg.visible = false
	$topbar/topbar_outlinerbutton.visible = false

func _process(_delta : float):
	GameSingleton.try_tick()

# REQUIREMENTS:
# * SS-42
func _on_game_session_menu_button_pressed() -> void:
	_game_session_menu.visible = !_game_session_menu.visible
