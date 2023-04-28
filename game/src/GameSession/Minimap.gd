extends Control

signal minimap_clicked(pos_clicked : Vector2)

const _action_click : StringName = &"map_click"

@export var _minimap_texture : Control

var _viewport_points : PackedVector2Array

func _ready():
	_minimap_texture.custom_minimum_size = Vector2(GameSingleton.get_aspect_ratio(), 1.0) * 150
	if Events.ShaderManager.set_up_shader(_minimap_texture.get_material(), false) != OK:
		push_error("Failed to set up minimap shader")

# REQUIREMENTS
# * SS-80
# * UI-752
func _draw() -> void:
	if _viewport_points.size() > 1:
		draw_multiline(_viewport_points, Color.WHITE, -1)

# REQUIREMENTS
# * SS-81
# * UIFUN-127
func _unhandled_input(event : InputEvent):
	if event is InputEventMouse and Input.is_action_pressed(_action_click):
		var pos_clicked := get_local_mouse_position() / size - Vector2(0.5, 0.5)
		if abs(pos_clicked.x) < 0.5 and abs(pos_clicked.y) < 0.5:
			minimap_clicked.emit(pos_clicked)

# Returns the point on the line going through p and q with the specific x coord
func _intersect_x(p : Vector2, q : Vector2, x : float) -> Vector2:
	if p.x == q.x:
		return Vector2(x, 0.5 * (p.y + q.y))
	var t := (x - q.x) / (p.x - q.x)
	return q + t * (p - q)

# Returns the point on the line going through p and q with the specific y coord
func _intersect_y(p : Vector2, q : Vector2, y : float) -> Vector2:
	if p.y == q.y:
		return Vector2(0.5 * (p.x + q.x), y)
	var t := (y - q.y) / (p.y - q.y)
	return q + t * (p - q)

const _one_x := Vector2(1, 0)

func _add_line_looped_over_x(left : Vector2, right : Vector2) -> void:
	if left.x < 0:
		if right.x < 0:
			_viewport_points.push_back(left + _one_x)
			_viewport_points.push_back(right + _one_x)
		else:
			var mid_point := _intersect_x(left, right, 0)
			_viewport_points.push_back(mid_point)
			_viewport_points.push_back(right)
			mid_point.x = 1
			_viewport_points.push_back(left + _one_x)
			_viewport_points.push_back(mid_point)
	elif right.x > 1:
		if left.x > 1:
			_viewport_points.push_back(left - _one_x)
			_viewport_points.push_back(right - _one_x)
		else:
			var mid_point := _intersect_x(left, right, 1)
			_viewport_points.push_back(left)
			_viewport_points.push_back(mid_point)
			mid_point.x = 0
			_viewport_points.push_back(mid_point)
			_viewport_points.push_back(right - _one_x)
	else:
		_viewport_points.push_back(left)
		_viewport_points.push_back(right)

# This can break if the viewport is rotated too far!
func _on_map_view_camera_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2) -> void:
	# Bound far y coords
	if far_left.y < 0:
		far_left = _intersect_y(near_left, far_left, 0)
	if far_right.y < 0:
		far_right = _intersect_y(near_right, far_right, 0)
	# Bound near y coords
	if near_left.y > 1:
		near_left = _intersect_y(near_left, far_left, 1)
	if near_right.y > 1:
		near_right = _intersect_y(near_right, far_right, 1)

	_viewport_points.clear()
	_add_line_looped_over_x(near_left, near_right)
	_add_line_looped_over_x(far_left, far_right)
	_add_line_looped_over_x(far_left, near_left)
	_add_line_looped_over_x(near_right, far_right)

	for i in _viewport_points.size():
		_viewport_points[i] *= size
	queue_redraw()
