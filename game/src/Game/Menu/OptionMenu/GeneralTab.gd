extends HBoxContainer

@export var initial_focus: Control


func _notification(what: int) -> void:
	match what:
		NOTIFICATION_VISIBILITY_CHANGED:
			if visible and is_inside_tree():
				initial_focus.grab_focus()
