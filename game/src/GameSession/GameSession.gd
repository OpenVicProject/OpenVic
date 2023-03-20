extends Control

var selectedId = 0

@export
var _province_num_display : Label

@export
var _province_size_display : Label

@export
var _main_menu_scene : PackedScene

# Called when the node enters the scene tree for the first time.
func _ready():
	updateVisibleInfo()
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass


func updateVisibleInfo():
	_province_num_display.text = str(selectedId)
	_province_size_display.text = str(Simulation.queryProvinceSize(selectedId))


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
	get_tree().change_scene_to_packed(_main_menu_scene)
