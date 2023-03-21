extends PanelContainer

signal close_button_pressed

@export var _main_menu_scene : PackedScene

func _ready():
	print("GameSessionMenu ready")

# REQUIREMENTS:
# * SS-47
# * UIFUN-69
func _on_to_main_menu_pressed():
	get_tree().change_scene_to_packed(_main_menu_scene)

# REQUIREMENTS:
# * UIFUN-69
func _on_close_button_pressed():
	close_button_pressed.emit()
