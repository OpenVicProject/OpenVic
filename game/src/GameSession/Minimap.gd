extends Control


var MouseInside: bool = false

func _process(delta):
	if MouseInside:
		print(get_local_mouse_position())

func _on_mouse_entered():
	MouseInside = true


func _on_mouse_exited():
	MouseInside = false
