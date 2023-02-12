extends Control

var selectedId = 0

# Called when the node enters the scene tree for the first time.
func _ready():
	updateVisibleInfo()
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass


func updateVisibleInfo():
	$CenterContainer/VBoxContainer2/GridContainer/ProvinceNumDisplay.text = str(selectedId)
	$CenterContainer/VBoxContainer2/GridContainer/ProvinceSizeDisplay.text = str(Simulation.queryProvinceSize(selectedId))


func _on_pass_time_button_pressed():
	Simulation.conductSimulationStep()
	updateVisibleInfo()


func _on_next_prov_button_pressed():
	selectedId = (selectedId + 1) % 10
	updateVisibleInfo()


func _on_prev_prov_button_pressed():
	if selectedId == 0:
		selectedId = 9
	else:
		selectedId -= 1
	updateVisibleInfo()


func _on_to_main_menu_pressed():
	get_tree().change_scene_to_file("res://src/MainMenu.tscn")
