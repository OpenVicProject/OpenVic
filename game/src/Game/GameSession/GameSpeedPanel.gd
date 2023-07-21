extends PanelContainer

# REQUIREMENTS:
# * SS-37, SS-38, SS-39

@export var _longform_date_button : Button
@export var _play_pause_display_button : Button
@export var _decrease_speed_button : Button
@export var _increase_speed_button : Button

func _ready():
	GameSingleton.state_updated.connect(_update_buttons)
	_update_buttons()

func _update_buttons():
	_play_pause_display_button.text = "⏸  " if GameSingleton.is_paused() else "▶"

	_increase_speed_button.disabled = not GameSingleton.can_increase_speed()
	_decrease_speed_button.disabled = not GameSingleton.can_decrease_speed()

	_longform_date_button.text = GameSingleton.get_longform_date()

# REQUIREMENTS:
# * UIFUN-73
func _on_decrease_speed_button_pressed():
	GameSingleton.decrease_speed()
	_update_buttons()

# REQUIREMENTS:
# * UIFUN-72
func _on_increase_speed_button_pressed():
	GameSingleton.increase_speed()
	_update_buttons()

# REQUIREMENTS:
# * UIFUN-71
func _on_play_pause_display_button_pressed():
	GameSingleton.toggle_paused()
	_update_buttons()

func _on_longform_date_label_pressed():
	GameSingleton.toggle_paused()
	_update_buttons()
