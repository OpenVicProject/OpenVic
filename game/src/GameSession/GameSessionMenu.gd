extends PanelContainer

@export var _main_menu_scene : PackedScene

@export var _main_menu_dialog : AcceptDialog
@export var _quit_dialog : AcceptDialog

var _main_menu_save_button : Button
var _main_menu_save_separator : Control
var _quit_save_button : Button
var _quit_save_separator : Control

signal options_button_pressed

func _ready() -> void:
	_main_menu_save_button = _main_menu_dialog.add_button("DIALOG_SAVE_AND_RESIGN", true, &"save_and_main_menu")
	_quit_save_button = _quit_dialog.add_button("DIALOG_SAVE_AND_QUIT", true, &"save_and_quit")

	# Neccessary to center the save buttons and preserve the reference to the separator elements
	var dialog_hbox : HBoxContainer = _main_menu_dialog.get_child(2, true)
	var index := _main_menu_save_button.get_index(true)
	dialog_hbox.move_child(_main_menu_save_button, _main_menu_dialog.get_ok_button().get_index(true))
	dialog_hbox.move_child(_main_menu_dialog.get_ok_button(), index)
	_main_menu_save_separator = dialog_hbox.get_child(_main_menu_save_button.get_index(true) - 1)

	dialog_hbox = _quit_dialog.get_child(2, true)
	index = _quit_save_button.get_index(true)
	dialog_hbox.move_child(_quit_save_button, _quit_dialog.get_ok_button().get_index(true))
	dialog_hbox.move_child(_quit_dialog.get_ok_button(), index)
	_quit_save_separator = dialog_hbox.get_child(_quit_save_button.get_index(true) - 1)

func hide_save_dialog_button() -> void:
	_main_menu_save_button.hide()
	_main_menu_save_separator.hide()
	_quit_save_button.hide()
	_quit_save_separator.hide()

func show_save_dialog_button() -> void:
	_main_menu_save_button.show()
	_main_menu_save_separator.show()
	_quit_save_button.show()
	_quit_save_separator.show()

# REQUIREMENTS:
# * SS-47
# * UIFUN-69
func _on_main_menu_confirmed() -> void:
	get_tree().change_scene_to_packed(_main_menu_scene)

# REQUIREMENTS:
# * SS-48
# * UIFUN-70
func _on_quit_confirmed() -> void:
	get_tree().quit()

# REQUIREMENTS:
# * SS-7, SS-46
# * UIFUN-11
func _on_options_button_pressed() -> void:
	options_button_pressed.emit()

func _on_main_menu_dialog_custom_action(action) -> void:
	match action:
		&"save_and_main_menu":
			_on_main_menu_confirmed()

func _on_quit_dialog_custom_action(action : StringName) -> void:
	match action:
		&"save_and_quit":
			_on_quit_confirmed()
