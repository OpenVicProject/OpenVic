extends Control


# Called when the node enters the scene tree for the first time.
func _ready():
	print("TODO: Load user settings!")
	pass # Replace with function body.

## Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass

func _on_resolution_selector_item_selected(index):
	print("Selected index: %d" % index)

func _on_screen_mode_selector_item_selected(index):
	print("Selected index: %d" % index)

func _on_monitor_display_selector_item_selected(index):
	print("Selected index: %d" % index)

func _on_music_volume_value_changed(value):
	print("Music: %f" % value)


func _on_sfx_volume_value_changed(value):
	print("SFX: %f" % value)


func _on_ear_exploder_toggled(button_pressed):
	print("KABOOM!!!" if button_pressed else "DEFUSED!!!")


func _on_save_settings_button_pressed():
	print("TODO: save current settings!")


func _on_back_button_pressed():
	get_tree().change_scene_to_file("res://src/MainMenu.tscn")


func _on_spin_box_value_changed(value):
	print("Spinbox: %d" % value)
