extends Control

signal back_button_pressed
signal save_game_selected

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


func _on_back_button_button_down():
	print("Returning to Main Menu.")
	back_button_pressed.emit()


func _on_start_button_button_down():
	print("Starting new game.")
	get_tree().change_scene_to_file("res://src/SampleGame.tscn")


func _on_game_select_list_item_selected(index):
	print("Selected save game: ", index)
	save_game_selected.emit(index)


func _on_save_game_selected(_index):
	$GameStartPanel/VBoxContainer/StartButton.disabled = false
