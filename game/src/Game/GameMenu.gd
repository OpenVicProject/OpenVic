extends Control

@export var _main_menu : Control
@export var _options_menu : Control
@export var _multiplayer_menu : Control
@export var _lobby_menu : Control
@export var _credits_menu : Control

# REQUIREMENTS
# * SS-10
func _ready() -> void:
	Events.Options.load_settings_from_file()

func _on_main_menu_new_game_button_pressed() -> void:
	_lobby_menu.show()
	_main_menu.hide()

# REQUIREMENTS
# * SS-6
# * UIFUN-5
func _on_main_menu_options_button_pressed() -> void:
	_options_menu.show()
	_main_menu.hide()


func _on_options_menu_back_button_pressed() -> void:
	_main_menu.show()
	_options_menu.hide()


func _on_lobby_menu_back_button_pressed() -> void:
	_main_menu.show()
	_lobby_menu.hide()


func _on_credits_back_button_pressed() -> void:
	_credits_menu.hide()
	_main_menu.show()


func _on_main_menu_credits_button_pressed() -> void:
	_credits_menu.show()
	_main_menu.hide()


func _on_multiplayer_menu_back_button_pressed() -> void:
	_multiplayer_menu.hide()
	_main_menu.show()


func _on_main_menu_multiplayer_button_pressed() -> void:
	_multiplayer_menu.show()
	_main_menu.hide()
