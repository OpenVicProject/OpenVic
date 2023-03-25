extends Control

signal options_button_pressed
signal new_game_button_pressed
signal credits_button_pressed

@export
var _new_game_button : BaseButton

@export
var _version_label : Label

@export
var _checksum_label : Label

const _version_number : String = "v0.01"
const _version_name : String = "Primum Mobile"

var _checksum : String = "XXXX"

# REQUIREMENTS:
# * SS-3
# * UIFUN-97
func _ready():
	print("From GDScript")
	TestSingleton.hello_singleton()
	_version_label.tooltip_text = "OpenVic2 %s \"%s\"" % [_version_number, _version_name]
	_version_label.text = _version_number
	# UI-111
	_checksum = Checksum.get_checksum_text()
	_update_checksum_label_text()
	_on_new_game_button_visibility_changed()

func _notification(what : int):
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_checksum_label_text()

func _update_checksum_label_text() -> void:
	_checksum_label.tooltip_text = tr("MAINMENU_CHECKSUM") % _checksum
	_checksum_label.text = "(%s)" % _checksum.substr(0, 4)

# REQUIREMENTS:
# * SS-14
# * UIFUN-32
func _on_new_game_button_pressed():
	print("Start a new game!")
	new_game_button_pressed.emit()


func _on_continue_button_pressed():
	print("Continue last game!")


func _on_multi_player_button_pressed():
	print("Have fun with friends!")


func _on_options_button_pressed():
	print("Check out some options!")
	options_button_pressed.emit()

# REQUIREMENTS
# * UI-32
# * UIFUN-36
func _on_credits_button_pressed():
	credits_button_pressed.emit()

# REQUIREMENTS
# * SS-4
# * UIFUN-3
func _on_exit_button_pressed():
	print("See you later!")
	get_tree().quit()

func _on_new_game_button_visibility_changed():
	if visible:
		_new_game_button.grab_focus.call_deferred()
