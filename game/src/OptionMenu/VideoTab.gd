extends HBoxContainer

@export var initial_focus: Button

func _notification(what : int) -> void:
	match(what):
		NOTIFICATION_VISIBILITY_CHANGED:
			if visible: initial_focus.grab_focus()
