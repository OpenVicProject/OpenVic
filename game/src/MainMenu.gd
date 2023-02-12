extends Control

signal options_button_pressed

func _ready():
	print("From GDScript")
	TestSingleton.hello_singleton()
	$VBox/Center/VBox/NewGameButton.grab_focus()


func _on_new_game_button_pressed():
	print("Start a new game!")


func _on_continue_button_pressed():
	print("Continue last game!")


func _on_multi_player_button_pressed():
	print("Have fun with friends!")


func _on_options_button_pressed():
	print("Check out some options!")
	options_button_pressed.emit()


func _on_exit_button_pressed():
	print("See you later!")
	get_tree().quit()
