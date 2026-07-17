extends Control

signal options_button_pressed
signal new_game_button_pressed
signal credits_button_pressed
signal multiplayer_button_pressed
signal continue_button_pressed

@export
var _new_game_button: BaseButton
@export
var _bottom_margin: MarginContainer

var _language_button: SettingsContainer.EnumOptionButton

# REQUIREMENTS:
# * SS-3


func _ready() -> void:
	_on_new_game_button_visibility_changed()

	var language_setting: GameSettings.Setting = GameSettings.get_setting(GameSettings.GENERAL_LANGUAGE)
	_language_button = SettingsContainer.EnumOptionButton.new(language_setting)
	_language_button.size_flags_horizontal = Control.SIZE_SHRINK_END
	_bottom_margin.add_child(_language_button)

	GameSettings.changed.connect(self._on_setting_updated)
	GameSettings.staged_changed.connect(self._on_setting_updated)


func _on_setting_updated(key: StringName) -> void:
	if key != GameSettings.GENERAL_LANGUAGE: return
	_language_button.update()

# REQUIREMENTS:
# * SS-14
# * UIFUN-32


func _on_new_game_button_pressed() -> void:
	print("Start a new game!")
	new_game_button_pressed.emit()


func _on_continue_button_pressed() -> void:
	print("Continue last game!")
	continue_button_pressed.emit()


func _on_multi_player_button_pressed() -> void:
	print("Have fun with friends!")
	multiplayer_button_pressed.emit()


func _on_options_button_pressed() -> void:
	print("Check out some options!")
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
	get_tree().root.propagate_notification(NOTIFICATION_WM_CLOSE_REQUEST)
	get_tree().quit()


func _on_new_game_button_visibility_changed() -> void:
	if visible:
		_new_game_button.grab_focus.call_deferred()
