extends CenterContainer

signal close_button_pressed

@export var _main_menu_scene : PackedScene

func _ready():
	print("GameSessionMenu ready")

func _on_to_main_menu_pressed():
	get_tree().change_scene_to_packed(_main_menu_scene)

func _on_close_button_pressed():
	close_button_pressed.emit()
