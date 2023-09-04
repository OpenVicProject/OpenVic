extends Control

var PanelName: String = "TEST"
var _mouse_over_viewport : bool = true
const _action_click : StringName = &"map_click"

func _on_close_button_pressed():
	self.queue_free()

func _on_top_panel_gui_input(event):
	var far_point1 : Vector2 = Vector2(self.get_global_position().x,self.get_global_position().y)
	var far_point2 : Vector2 = Vector2(self.get_global_position().x+self.size.x,self.get_global_position().y+self.size.y)

	if Input.is_action_pressed(_action_click) and event is InputEventMouseMotion and _mouse_over_viewport: 
		if far_point1.x+event.relative.x > 0 and far_point1.y+event.relative.y > 0 and far_point2.x+event.relative.x < get_viewport().size.x and far_point2.y+event.relative.y < get_viewport().size.y and get_global_mouse_position().x > 0 and get_global_mouse_position().y > 0 and get_global_mouse_position().x < get_viewport().size.x and get_global_mouse_position().y < get_viewport().size.y:
			set_position(self.get_screen_position()+event.relative)
		else:
			pass

func _on_mouse_entered_viewport():
	_mouse_over_viewport = true

func _on_mouse_exited_viewport():
	_mouse_over_viewport = false
