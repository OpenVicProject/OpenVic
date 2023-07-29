@tool
extends TextureRect

class_name PieChart


@export var donut : bool = false
@export_range(0.0,1.0) var donut_inner_radius : float = 0.5
@export_range(0.0,0.5) var radius : float = 0.4
@export var shadow_displacement : Vector2 = Vector2(0.6,0.6)
@export var shadow_focus : float = 1.0
@export var shadow_radius : float = 0.6
@export var shadow_thickness : float = 1.0

@export var trim_colour : Color = Color(0.0,0.0,0.0)
@export_range(0.0,1.0) var trim_size : float = 0.02
@export var donut_inner_trim : bool = true
@export var slice_gradient_falloff : float = 3.6
@export var slice_gradient_base : float = 3.1

#@onready
@export var RichTooltip : RichTextLabel# = $RichToolTip

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

}
#example slices:
"""
	"label1":SliceData.new(5,"Conservative",Color(0.0,0.0,1.0)),
	"label2":SliceData.new(3,"Liberal",Color(1.0,1.0,0.0)),
	"label3":SliceData.new(2,"Reactionary",Color(0.4,0.0,0.6))
"""

#These functions are the interface a developer will use to update the piechart
#The piechart will only redraw once one of these has been triggered
func addOrReplaceLabel(labelName:String,quantity:float,tooltip:String,colour:Color=Color(0.0,0.0,0.0)) -> void:
	slices[labelName] = SliceData.new(quantity,tooltip,colour)
	_recalculate()

func updateLabelQuantity(labelName:String,quantity:float) -> void:
	if slices.has(labelName):
		slices[labelName].quantity = quantity
	_recalculate()

func updateLabelColour(labelName:String,colour:Color) -> void:
	if slices.has(labelName):
		slices[labelName].colour = colour
	_recalculate()

func updateLabelTooltip(labelName:String,tooltip:String) -> void:
	if slices.has(labelName):
		slices[labelName].tooltip = tooltip

func RemoveLabel(labelName:String) -> bool:
	var out = slices.erase(labelName)
	_recalculate()
	return out

#Perhaps in the future, a method to reorder the labels?


#In editor only, force the shader parameters to update whenever _draw
#is called so developers can see their changes
#otherwise, for performance, reduce the number of material resets
func _draw():
	if Engine.is_editor_hint():
		if not material:
			_reset_material()
		_setShaderParams()
		_recalculate()

func _ready():
	_reset_material()
	_setShaderParams()

func _reset_material():
	texture = CanvasTexture.new()
	var mat_res = load("res://src/Game/Theme/PieChart/PieChartMat.tres")
	material = mat_res.duplicate(true)
	custom_minimum_size = Vector2(50.0,50.0)
	size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	size_flags_vertical = Control.SIZE_SHRINK_CENTER
	_recalculate()

func _setShaderParams():
	material.set_shader_parameter("donut",donut)
	material.set_shader_parameter("donut_inner_trim",donut_inner_trim)
	material.set_shader_parameter("radius",radius)
	material.set_shader_parameter("donut_inner_radius",donut_inner_radius/2.0)
	
	material.set_shader_parameter("trim_colour",Vector3(trim_colour.r,trim_colour.g,trim_colour.b))
	material.set_shader_parameter("trim_size",trim_size)
	material.set_shader_parameter("gradient_falloff",slice_gradient_falloff)
	material.set_shader_parameter("gradient_base",slice_gradient_base)
	
	material.set_shader_parameter("shadow_displacement",shadow_displacement)
	material.set_shader_parameter("shadow_tightness",shadow_focus)
	material.set_shader_parameter("shadow_radius",shadow_radius)
	material.set_shader_parameter("shadow_thickness",shadow_thickness)


#Update the slice angles based on the new slice data
func _recalculate() -> void:
	#where the slices are the public interface, these are the actual paramters
	#which will be sent to the shader
	var angles: Array = []
	var colours: Array = []
	
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
	

#Process mouse to select the appropriate tooltip for the slice
func _gui_input(event:InputEvent):
	if event is InputEventMouse:
		var pos = event.position
		var _handled:bool = _handleTooltip(pos)

func _on_mouse_exited():
	RichTooltip.visible = false

#takes a mouse position, and sets an appropriate tooltip for the slice the mouse
#is hovered over. Returns a boolean on whether the tooltip was handled.
func _handleTooltip(pos:Vector2) -> bool:
	#is it within the circle?
	var center = Vector2(size.x/2.0, size.x/2.0)
	var radius = size.x/2.0
	var distance = center.distance_to(pos)
	#print(distance >= donut_inner_radius/2.0)
	var real_donut_inner_radius:float = radius * donut_inner_radius
	if distance <= radius and (not donut or distance >= real_donut_inner_radius):
		var angle = _convertAngle(center.angle_to_point(pos))
		for label in slices.keys():
			var slice = slices.get(label)
			if angle <= slice.final_angle:
				RichTooltip.visible = true
				RichTooltip.text = _createTooltip(label)
				RichTooltip.position =  pos + Vector2(5,5) + get_global_rect().position #get_global_rect().position +
				RichTooltip.reset_size()
				
				return true
	else:
		#Technically the corners of the bounding box
		#are part of the chart, but we don't want a tooltip there
		RichTooltip.visible = false
	return false

#create a list of all the values and percentages
# but with the hovered one on top and highlighted
func _createTooltip(labelHovered:String) -> String:
	var tooltip:String = ""
	var hoveredSlice = slices.get(labelHovered)
	var formatted_percent = _formatpercent(hoveredSlice.percentage)
	#TOOD: perhaps this is a bit much, but final feedback should determine this
	tooltip += "[font_size=10][i][u][b]>> {name} {percentage}% <<[/b][/u][/i]".format(
		{"name":hoveredSlice.tooltip,"percentage":formatted_percent})
	
	for label in slices.keys():
		if label == labelHovered: continue
		var slice = slices.get(label)
		var percent = _formatpercent(slice.percentage)
		tooltip += "\n{name} {percentage}%".format(
			{"name":slice.tooltip,"percentage":percent})
	tooltip += "[/font_size]"
	return tooltip

#angle from center.angle_to_point is measured from the +x axis
#, but the chart starts from +y
#the input angle is also -180 to 180, where we want 0 to 360
func _convertAngle(angleIn:float) -> float:
	#make the angle start from +y, range is now -90 to 270
	var angle = angleIn + PI/2.0
	#adjust range to be 0 to 360
	if angle < 0:
		angle = 2.0*PI + angle
	return angle
	
func _formatpercent(percentIn:float) -> float:
	return snappedf((percentIn * 100),0.1)



