extends Control

signal new_game_button_pressed
signal continue_button_pressed
signal multiplayer_button_pressed
signal mod_button_pressed
signal options_button_pressed
signal credits_button_pressed

@export
var _new_game_button : BaseButton

# REQUIREMENTS:
# * SS-3
func _ready() -> void:
	_on_new_game_button_visibility_changed()

# REQUIREMENTS:
# * SS-14
# * UIFUN-32
func _on_new_game_button_pressed() -> void:
	new_game_button_pressed.emit()

func _on_continue_button_pressed() -> void:
	continue_button_pressed.emit()

func _on_multi_player_button_pressed() -> void:
	multiplayer_button_pressed.emit()

func _on_mod_button_pressed():
	mod_button_pressed.emit()

func _on_options_button_pressed() -> void:
	options_button_pressed.emit()

# REQUIREMENTS
# * UI-32
# * UIFUN-36
func _on_credits_button_pressed() -> void:
	credits_button_pressed.emit()

# REQUIREMENTS
# * SS-4
# * UIFUN-3
func _on_exit_button_pressed() -> void:
	print("See you later!")
	get_tree().quit()

func _on_new_game_button_visibility_changed() -> void:
	if visible:
		_new_game_button.grab_focus.call_deferred()
