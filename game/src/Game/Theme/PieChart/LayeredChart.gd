extends CenterContainer

var overlapping_charts:Array = []

# Called when the node enters the scene tree for the first time.
func _ready():
	for child in get_children():
		if child is PieChart:
			overlapping_charts.push_back(child)

#Process mouse to select the appropriate tooltip for the slice
func _gui_input(event:InputEvent):
	if event is InputEventMouse:
		var pos = event.position
		var handled:bool = false
		var x = overlapping_charts.size()
		#process the charts in reverse order (overlying charts first)
		#as you can't actually make the inner chart(s) smaller with a centerContainer
		for i in range(x):
			var chart = overlapping_charts[x-(i+1)]
			if not handled:
				handled = chart.handleTooltip(pos)
			else:
				chart.RichTooltip.visible = false

func _on_mouse_exited():
	for chart in overlapping_charts:
		chart.RichTooltip.visible = false
