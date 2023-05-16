@tool
extends Node2D

class_name PieChart

@export var radius:float = 80
@export var num_pts:int = 32

#a data class for the pie chart
class SliceData:
	var colour:Color = Color(1.0,0.0,0.0):
		get:
			return colour
		set(value):
			colour = value
	var tooltip:String = "DEFAULT":
		get:
			return tooltip
		set(value):
			tooltip = value
	var quantity:float = -1:
		get:
			return quantity
		set(value):
			quantity = value

	func _init(quantityIn:float,tooltipIn:String,colourIn:Color):
		colour = colourIn
		tooltip = tooltipIn
		quantity = quantityIn
		

var slices: Dictionary = {
	"label1":SliceData.new(5,"Test",Color(1.0,0.0,0.0)),
	"label2":SliceData.new(3,"Test2",Color(0.0,1.0,0.0)),
	"label3":SliceData.new(1,"Test3",Color(0.0,0.0,1.0))
}

#These functions are the interface a developer will use to update the piechart
#The piechart will only redraw once one of these has been triggered
func addOrReplaceLabel(labelName:String,quantity:float,tooltip:String,colour:Color=Color(0.0,0.0,0.0)) -> void:
	slices[labelName] = SliceData.new(quantity,tooltip,colour)
	queue_redraw()

func updateLabelQuantity(labelName:String,quantity:float) -> void:
	if slices.has(labelName):
		slices[labelName].quantity = quantity
	queue_redraw()

func updateLabelColour(labelName:String,colour:Color) -> void:
	if slices.has(labelName):
		slices[labelName].colour = colour
	queue_redraw()

func updateLabelTooltip(labelName:String,tooltip:String) -> void:
	if slices.has(labelName):
		slices[labelName].tooltip = tooltip
	queue_redraw()

func RemoveLabel(labelName:String) -> bool:
	var out = slices.erase(labelName)
	queue_redraw()
	return out

#Perhaps in the future, a method to reorder the labels?

func _draw():
	var center = Vector2(radius, radius)
	
	var total:float = 0
	for slice in slices.values():
		total += slice.quantity

	var current_arc_start:int = 0
	var current_arc_finish:int = 0

	for slice in slices.values():
		var degrees_to_cover:float = (slice.quantity / total) * 360
		current_arc_finish = current_arc_start + degrees_to_cover
		draw_circle_arc_poly(center,radius,current_arc_start,current_arc_finish,slice.colour)
		current_arc_start = current_arc_finish

#taken from the godot tutorials
func draw_circle_arc_poly(center, radius, angle_from, angle_to, color):
	var points_arc = PackedVector2Array()
	points_arc.push_back(center)
	var colours = PackedColorArray([color])

	for i in range(num_pts + 1):
		var angle_point = deg_to_rad(angle_from + i * (angle_to - angle_from) / num_pts - 90)
		points_arc.push_back(center + Vector2(cos(angle_point), sin(angle_point)) * radius)
	draw_polygon(points_arc, colours)
