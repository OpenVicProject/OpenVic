@tool
extends TextureRect

class_name PieChart


@export var donut : bool = false
@export_range(0.0, 1.0) var donut_inner_radius : float = 0.5
@export_range(0.0, 0.5) var radius : float = 0.4
@export var shadow_displacement : Vector2 = Vector2(0.55, 0.6)
@export var shadow_tightness : float = 1.0
@export var shadow_radius : float = 0.6
@export var shadow_thickness : float = 1.0

@export var trim_colour : Color = Color(0.0, 0.0, 0.0)
@export_range(0.0, 1.0) var trim_size : float = 0.02
@export var donut_inner_trim : bool = true
@export var slice_gradient_falloff : float = 3.6
@export var slice_gradient_base : float = 3.1

@export var _rich_tooltip : RichTextLabel
var _pie_chart_image : Image

# A data class for the pie chart
class SliceData:
	extends RefCounted
	# Primary properties, change these to change
	# the displayed piechart
	var colour : Color = Color(1.0, 0.0, 0.0)
	var tooltip : String = ""
	var quantity : float = -1
	# Derived properties, don't set from an external script
	var final_angle : float = -1
	var percentage : float = 0:
		get:
			return percentage
		set(value):
			percentage = clampf(value, 0, 1)

	func _init(quantityIn : float, tooltipIn : String, colourIn : Color):
		colour = colourIn
		tooltip = tooltipIn
		quantity = quantityIn

# The key of an entry of this dictionary should be an easy to reference constant
# The tooltip label is what the user will actually read
var _slices : Dictionary = {}

# Slice keys/labels in the order they should be displayed
var _slice_order : Array = []

# Example slices:
"""
	"label1": SliceData.new(5, "Conservative", Color(0.0, 0.0, 1.0)),
	"label2": SliceData.new(3, "Liberal", Color(1.0, 1.0, 0.0)),
	"label3": SliceData.new(2, "Reactionary", Color(0.4, 0.0, 0.6))
"""

# These functions are the interface a developer will use to update the piechart
# The piechart will only redraw once one of these has been triggered
func add_or_replace_label(labelName : String, quantity : float, tooltip : String, colour : Color = Color(0.0, 0.0, 0.0)) -> void:
	_slices[labelName] = SliceData.new(quantity, tooltip, colour)
	if _slice_order.find(labelName) == -1:
		_slice_order.push_back(labelName)
	_recalculate()

func update_label_quantity(labelName : String, quantity : float) -> void:
	if _slices.has(labelName):
		_slices[labelName].quantity = quantity
	_recalculate()

func update_label_colour(labelName : String, colour : Color) -> void:
	if _slices.has(labelName):
		_slices[labelName].colour = colour
	_recalculate()

func update_label_tooltip(labelName : String, tooltip : String) -> void:
	if _slices.has(labelName):
		_slices[labelName].tooltip = tooltip

func remove_label(labelName : String) -> bool:
	if _slices.erase(labelName):
		var index := _slice_order.find(labelName)
		if index == -1:
			push_error("Slice in dictionary but not order list: ", labelName)
		else:
			_slice_order.remove_at(index)
		_recalculate()
		return true
	return false

func clear_slices() -> void:
	_slices.clear()
	_slice_order.clear()

# Distribution dictionary of the form:
# { "<label>": { "size": <quantity>, "colour": <colour> } }
func set_to_distribution(dist : Dictionary) -> void:
	clear_slices()
	for key in dist:
		var entry : Dictionary = dist[key]
		_slices[key] = SliceData.new(entry[GameSingleton.get_piechart_info_size_key()], key, entry[GameSingleton.get_piechart_info_colour_key()])
	_slice_order = _slices.keys()
	sort_slices()

# Sorted by quantity, smallest to largest, so that the smallest slice
# is to the left of a radial line straight upwards
func sort_slices() -> void:
	_slice_order.sort_custom(func (a, b): return _slices[a].quantity < _slices[b].quantity)
	_recalculate()

func _ready():
	if not Engine.is_editor_hint():
		const image_size : int = 256
		_pie_chart_image = Image.create(image_size, image_size, false, Image.FORMAT_RGBA8)
		texture = ImageTexture.create_from_image(_pie_chart_image)
		_recalculate()

# Update the slice angles based on the new slice data
func _recalculate() -> void:
	# Where the slices are the public interface, these are the actual paramters
	# which will be sent to the shader/draw function
	var angles : Array = []
	var colours : Array = []

	var total : float = 0
	for label in _slice_order:
		var quantity : float = _slices[label].quantity
		if quantity > 0:
			total += quantity

	var current_arc_start : float = 0
	var current_arc_finish : float = 0

	for label in _slice_order:
		var slice : SliceData = _slices[label]
		if slice.quantity > 0:
			slice.percentage = slice.quantity / total
			var rads_to_cover : float = slice.percentage * 2.0 * PI
			current_arc_finish = current_arc_start + rads_to_cover
			slice.final_angle = current_arc_finish
			current_arc_start = current_arc_finish
			angles.push_back(current_arc_finish)
			colours.push_back(slice.colour)

	GameSingleton.draw_pie_chart(_pie_chart_image, angles, colours, radius, shadow_displacement, shadow_tightness, shadow_radius, shadow_thickness,
		trim_colour, trim_size, slice_gradient_falloff, slice_gradient_base, donut, donut_inner_trim, donut_inner_radius / 2)
	texture.set_image(_pie_chart_image)

# Process mouse to select the appropriate tooltip for the slice
func _gui_input(event : InputEvent):
	if event is InputEventMouse:
		var pos : Vector2 = event.position
		var _handled : bool = _handle_tooltip(pos)

func _on_mouse_exited():
	_rich_tooltip.visible = false

# Takes a mouse position, and sets an appropriate tooltip for the slice the mouse
# is hovered over. Returns a boolean on whether the tooltip was handled.
func _handle_tooltip(pos : Vector2) -> bool:
	# Is it within the circle?
	var real_radius := size.x / 2.0
	var center := Vector2(real_radius, real_radius)
	var distance := center.distance_to(pos)
	var real_donut_inner_radius : float = real_radius * donut_inner_radius
	if distance <= real_radius and (not donut or distance >= real_donut_inner_radius):
		if _slice_order.is_empty():
			_rich_tooltip.text = "PIECHART_TOOLTIP_NO_DATA"
		else:
			var angle := _convert_angle(center.angle_to_point(pos))
			var selected_label : String = ""
			for label in _slice_order:
				if angle <= _slices[label].final_angle:
					if not selected_label or _slices[label].final_angle < _slices[selected_label].final_angle:
						selected_label = label
			if not selected_label:
				selected_label = _slice_order[0]
			_rich_tooltip.text = _create_tooltip(selected_label)
		_rich_tooltip.visible = true
		_rich_tooltip.position =  pos + Vector2(5, 5) + get_global_rect().position
		_rich_tooltip.reset_size()
	else:
		# Technically the corners of the bounding box
		# are part of the chart, but we don't want a tooltip there
		_rich_tooltip.visible = false
	return _rich_tooltip.visible

# Create a list of all the values and percentages
# with the hovered one highlighted
func _create_tooltip(labelHovered : String) -> String:
	var slice_tooltips : PackedStringArray = []
	for label in _slice_order:
		var slice : SliceData = _slices.get(label)
		var percent := _format_percent(slice.percentage)
		var entry : String = "%s %s%%" % [tr(label), percent]
		if label == labelHovered:
			entry = "[i][u][b]>>%s<<[/b][/u][/i]" % entry
		slice_tooltips.push_back(entry)
	# Slices are ordered smallest to largest, but here we want the opposite
	slice_tooltips.reverse()
	return "[font_size=14]%s[/font_size]" % "\n".join(slice_tooltips)

# Angle from center.angle_to_point is measured from the +x axis,
# but the chart starts from +y
# The input angle is also -180 to 180, where we want 0 to 360
func _convert_angle(angleIn : float) -> float:
	# Make the angle start from +y, range is now -90 to 270
	var angle := angleIn + PI / 2.0
	# Adjust range to be 0 to 360
	if angle < 0:
		angle += 2.0 * PI
	return angle

func _format_percent(percentIn : float) -> float:
	return snappedf((percentIn * 100), 0.1)
