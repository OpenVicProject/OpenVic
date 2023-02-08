extends Control


# Called when the node enters the scene tree for the first time.
func _ready():
	print("From GDScript")
	TestSingleton.hello_singleton()
	$CenterContainer/VBoxContainer/NewGameButton.grab_focus()
	pass # Replace with function body.


## Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass


func _on_new_game_button_pressed():
	print("Start a new game!")


func _on_continue_button_pressed():
	print("Continue last game!")


func _on_multi_player_button_pressed():
	print("Have fun with friends!")


func _on_options_button_pressed():
	print("Check out some options!")
	get_tree().change_scene_to_file("res://src/OptionsMenu.tscn")


func _on_exit_button_pressed():
	print("See you later!")
	get_tree().quit()
