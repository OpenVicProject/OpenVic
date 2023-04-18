extends PanelContainer

#UI-74 UI-75 UI-76 UI-77

@export var _longform_date_button : Button
@export var _play_pause_display_button : Button
@export var _decrease_speed_button : Button
@export var _increase_speed_button : Button

var is_game_paused : bool = true

# Called when the node enters the scene tree for the first time.
func _ready():
	_update_playpause_button()

func _update_playpause_button():
	_play_pause_display_button.text = "⏸️" if is_game_paused else "▶"
	print("Game is paused" if is_game_paused else "Game is advancing")


func _on_decrease_speed_button_pressed():
	print("Decrease speed")

func _on_increase_speed_button_pressed():
	print("Increase speed")

func _on_play_pause_display_button_pressed():
	is_game_paused = !is_game_paused
	_update_playpause_button()

func _on_longform_date_label_pressed():
	is_game_paused = !is_game_paused
	_update_playpause_button()
