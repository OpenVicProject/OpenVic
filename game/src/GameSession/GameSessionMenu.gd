extends PanelContainer

@export var _main_menu_scene : PackedScene

signal options_button_pressed

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
	hide()
	options_button_pressed.emit()
