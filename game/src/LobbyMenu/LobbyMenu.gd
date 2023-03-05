extends HBoxContainer

# REQUIREMENTS:
# * 1.4 Game Lobby Menu
# * SS-12

signal back_button_pressed
signal save_game_selected

@export
var start_button : BaseButton

# REQUIREMENTS:
# * SS-16
# * UIFUN-40
func _on_back_button_button_down():
	print("Returning to Main Menu.")
	back_button_pressed.emit()


# REQUIREMENTS:
# * SS-21
func _on_start_button_button_down():
	print("Starting new game.")
	get_tree().change_scene_to_file("res://src/SampleGame.tscn")


# REQUIREMENTS:
# * SS-19
# * SS-18
func _on_game_select_list_item_selected(index):
	print("Selected save game: ", index)
	save_game_selected.emit(index)


func _on_save_game_selected(_index):
	start_button.disabled = false
