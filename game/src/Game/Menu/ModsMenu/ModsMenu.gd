extends Control

signal back_button_pressed

@export var _apply_changes_button : Button
@export var _mods_listbox : BoxContainer

func _ready() -> void:
	pass

func _on_back_button_pressed() -> void:
	back_button_pressed.emit()

func _on_apply_changes_button_pressed() -> void:
	OS.set_restart_on_exit(true)
	get_tree().root.propagate_notification(NOTIFICATION_WM_CLOSE_REQUEST)
	get_tree().quit()
