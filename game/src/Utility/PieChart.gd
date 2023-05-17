@tool
extends Control

class_name PieChart

var min_trim_size:float = 0.5

@export var radius:float = 80
@export var num_pts:int = 32
@export var trim_size:float = 1
@export var trim_colour:Color = Color(0.1,0.1,0.1)

@export var donut:bool = false
@export var donut_inner_radius:float = 40
@export var donut_do_inner_trim:bool = false

var center = Vector2(radius, radius)

#a data class for the pie chart
class SliceData:
	#primary properties, change these to change
	#the displayed piechart
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
	#derived properties, don't set from an external
	#script
	var final_angle:float = -1:
		get:
			return final_angle
		set(value):
			final_angle = value
	var percentage:float = 0:
		get:
			return percentage
		set(value):
			percentage = clampf(value,0,1)

	func _init(quantityIn:float,tooltipIn:String,colourIn:Color):
		colour = colourIn
		tooltip = tooltipIn
		quantity = quantityIn


var slices: Dictionary = {
	"label1":SliceData.new(5,"Conservative",Color(0.0,0.0,1.0)),
	"label2":SliceData.new(3,"Liberal",Color(1.0,1.0,0.0)),
	"label3":SliceData.new(1,"Reactionary",Color(0.1,0.0,0.6))
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

func RemoveLabel(labelName:String) -> bool:
	var out = slices.erase(labelName)
	queue_redraw()
	return out

#Perhaps in the future, a method to reorder the labels?



#TODO: Stylize the pie chart so it doesn't look "flat"
#thoughts: gradient biased to be darker at edge
# second: gradient to create spotlight somewhere
# 3rd: border trim

#think of this as an update function that is only called
#when a slice or label is updated
func _draw():
	center = Vector2(radius, radius)
	size = 2*center
	
	var total:float = 0
	for slice in slices.values():
		total += slice.quantity

	var current_arc_start:int = 0
	var current_arc_finish:int = 0

	for slice in slices.values():
		slice.percentage = slice.quantity / total
		var degrees_to_cover:float = slice.percentage * 360
		current_arc_finish = current_arc_start + degrees_to_cover
		slice.final_angle = current_arc_finish
		if donut and donut_do_inner_trim:
			#inner trim
			draw_donut_arc_poly(center,donut_inner_radius,donut_inner_radius+trim_size,current_arc_start,current_arc_finish,trim_colour)
			#donut
			draw_donut_arc_poly(center,donut_inner_radius+trim_size,radius-trim_size,current_arc_start,current_arc_finish,slice.colour)
		elif donut:
			#donut
			draw_donut_arc_poly(center,donut_inner_radius,radius-trim_size,current_arc_start,current_arc_finish,slice.colour)
		else:
			#circle
			draw_circle_arc_poly(center,radius-trim_size,current_arc_start,current_arc_finish,slice.colour)
		#outer trim
		draw_donut_arc_poly(center,radius-trim_size,radius,current_arc_start,current_arc_finish,trim_colour)
		
		current_arc_start = current_arc_finish
	#if !slices.is_empty():
	#	draw_circle_arc(center,radius,0,360,trim_colour)
	
func validate() -> bool:
	if trim_size >= radius - min_trim_size:
		return false
	if donut and trim_size >= radius - 2*min_trim_size - donut_inner_radius:
		return false
	
	return true

#Process mouse to select the appropriate tooltip for the slice
func _gui_input(event:InputEvent):
	if event is InputEventMouse:
		var pos = event.position
		#is it within the circle?
		if center.distance_to(pos) <= radius:
			var angle = convertAngle(rad_to_deg(center.angle_to_point(pos)))
			for slice in slices.values():
				if angle <= slice.final_angle:
					var formatted_percent = formatpercent(slice.percentage)
					tooltip_text = "{name} {percentage}%".format({"name":slice.tooltip,"percentage":formatted_percent})
					break
		else:
			#Technically the corners of the bounding box
			#are part of the chart, but we don't want a tooltip there
			tooltip_text = ""

#angle from center.angle_to_point is measured from the +x axis
#, but the chart starts from +y
#the input angle is also -180 to 180, where we want 0 to 360
func convertAngle(angleIn:float) -> float:
	#make the angle start from +y, range is now -90 to 270
	var angle = angleIn + 90
	#adjust range to be 0 to 360
	if angle < 0:
		angle = 360 + angle
	return angle
	
func formatpercent(percentIn:float) -> float:
	return snappedf((percentIn * 100),0.1)	



#taken from the godot tutorials
#draw a filled arc
func draw_circle_arc_poly(center:Vector2, radius:float, angle_from:float, angle_to:float, color:Color) -> void:
	var points_arc = PackedVector2Array()
	points_arc.push_back(center)
	var colours = PackedColorArray([color])

	for i in range(num_pts + 1):
		var angle_point = deg_to_rad(angle_from + i * (angle_to - angle_from) / num_pts - 90)
		points_arc.push_back(center + Vector2(cos(angle_point), sin(angle_point)) * radius)
	draw_polygon(points_arc, colours)
	
#again taken from the tutorials
#draw an unfilled arc
func draw_circle_arc(center:Vector2, radius:float, angle_from:float, angle_to:float, color:Color) -> void:
	var points_arc = PackedVector2Array()

	for i in range(num_pts + 1):
		var angle_point = deg_to_rad(angle_from + i * (angle_to-angle_from) / num_pts - 90)
		points_arc.push_back(center + Vector2(cos(angle_point), sin(angle_point)) * radius)

	for index_point in range(num_pts):
		draw_line(points_arc[index_point], points_arc[index_point + 1], color)
		
#draw a filled donut arc, used for trim and donut chart
func draw_donut_arc_poly(center:Vector2, radius_inner:float, radius_outer:float, angle_from:float, angle_to:float, color:Color) -> void:
	var points_arc = PackedVector2Array()
	var colours = PackedColorArray([color])
	for i in range(num_pts + 1):
		var angle_point = deg_to_rad(angle_from + i * (angle_to - angle_from) / num_pts - 90)
		points_arc.push_back(center + Vector2(cos(angle_point), sin(angle_point)) * radius_outer)
	for i in range(num_pts + 1): 
		var j = num_pts - i
		var angle_point = deg_to_rad(angle_from + j * (angle_to - angle_from) / num_pts - 90)
		points_arc.push_back(center + Vector2(cos(angle_point), sin(angle_point)) * radius_inner)
	draw_polygon(points_arc, colours)
