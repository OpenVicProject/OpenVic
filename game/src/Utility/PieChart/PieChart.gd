@tool
extends TextureRect

class_name PieChart


@export var donut:bool = false
@export_range(0.0,1.0) var donut_inner_radius:float = 0.5

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
	#derived properties, don't set from an external script
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

#The key of an entry of this dictionnary should be an easy to reference constant
#The tooltip label is what the user will actually read
var slices: Dictionary = {
	"label1":SliceData.new(5,"Conservative",Color(0.0,0.0,1.0)),
	"label2":SliceData.new(3,"Liberal",Color(1.0,1.0,0.0)),
	"label3":SliceData.new(2,"Reactionary",Color(0.4,0.0,0.6))
}

#These functions are the interface a developer will use to update the piechart
#The piechart will only redraw once one of these has been triggered
func addOrReplaceLabel(labelName:String,quantity:float,tooltip:String,colour:Color=Color(0.0,0.0,0.0)) -> void:
	slices[labelName] = SliceData.new(quantity,tooltip,colour)
	recalculate()

func updateLabelQuantity(labelName:String,quantity:float) -> void:
	if slices.has(labelName):
		slices[labelName].quantity = quantity
	recalculate()

func updateLabelColour(labelName:String,colour:Color) -> void:
	if slices.has(labelName):
		slices[labelName].colour = colour
	recalculate()

func updateLabelTooltip(labelName:String,tooltip:String) -> void:
	if slices.has(labelName):
		slices[labelName].tooltip = tooltip

func RemoveLabel(labelName:String) -> bool:
	var out = slices.erase(labelName)
	recalculate()
	return out

#Perhaps in the future, a method to reorder the labels?


func _draw():
	recalculate()

func _ready():
	texture = CanvasTexture.new()
	var mat_res = load("res://src/Utility/PieChart/PieChartMat.tres")
	material = mat_res.duplicate(true)
	custom_minimum_size = Vector2(50.0,50.0)
	size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	size_flags_vertical = Control.SIZE_SHRINK_CENTER
	recalculate()

#TODO:
#-make sure inputs are passed when over tranparent part
#-lock the size to be square?
#-Fix the material to be available whenever the chart is instanced
#-make sure each chart is unique when instanced
#-validate inputs
#-check the programming interface works
#-make draft pr

#Update the slice angles based on the new slice data
func recalculate() -> void:
	#center = Vector2(radius, radius)
	#size = 2*center
	
	#where the slices are the public interface, these are the actual paramters
	#which will be sent to the shader
	var angles: Array = []
	var colours: Array = []
	
	#var inner_rad_shader = donut_inner_radius / (2.0*size.x);
	material.set_shader_parameter("donut_inner_radius",donut_inner_radius/2.0)
	material.set_shader_parameter("donut",donut)
	
	var total:float = 0
	for slice in slices.values():
		total += slice.quantity

	var current_arc_start:float = 0
	var current_arc_finish:float = 0

	for slice in slices.values():
		slice.percentage = slice.quantity / total
		var rads_to_cover:float = slice.percentage * 2.0*PI
		current_arc_finish = current_arc_start + rads_to_cover
		slice.final_angle = current_arc_finish
		current_arc_start = current_arc_finish
		angles.push_back(current_arc_finish)
		colours.push_back(Vector3(slice.colour.r,slice.colour.g,slice.colour.b) )
	material.set_shader_parameter("stopAngles",angles)
	material.set_shader_parameter("colours",colours)
	
"""func validate() -> bool:
	if trim_size >= radius - min_trim_size:
		return false
	if donut and trim_size >= radius - 2*min_trim_size - donut_inner_radius:
		return false
	
	return true
"""
#Process mouse to select the appropriate tooltip for the slice
func _gui_input(event:InputEvent):
	if event is InputEventMouse:
		var pos = event.position
		
		#is it within the circle?
		var center = Vector2(size.x/2.0, size.x/2.0)
		var radius = size.x/2.0
		var distance = center.distance_to(pos)
		#print(distance >= donut_inner_radius/2.0)
		var real_donut_inner_radius:float = radius * donut_inner_radius
		if distance <= radius and (not donut or distance >= real_donut_inner_radius):
			var angle = convertAngle(center.angle_to_point(pos))
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
	var angle = angleIn + PI/2.0
	#adjust range to be 0 to 360
	if angle < 0:
		angle = 2.0*PI + angle
	return angle
	
func formatpercent(percentIn:float) -> float:
	return snappedf((percentIn * 100),0.1)	
