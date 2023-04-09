extends Control

@export var _background : TextureRect

var _viewport_points : PackedVector2Array

func _draw() -> void:
	if _viewport_points.size() > 1:
		draw_polyline(_viewport_points, Color.WHITE, -1)

func _on_camera_view_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2) -> void:
	_viewport_points.clear()
	_viewport_points.push_back(near_left * _background.size)
	_viewport_points.push_back(far_left * _background.size)
	_viewport_points.push_back(far_right * _background.size)
	_viewport_points.push_back(near_right * _background.size)
	_viewport_points.push_back(_viewport_points[0])
	
	# Cutting out of Frame part of camera polyline
	for i in range(0,_viewport_points.size()):
		for j in range(0,2):
			if _viewport_points[i][j] > _background.size[j]:
				_viewport_points[i][j] = _background.size[j]
			elif _viewport_points[i][j] < 0:
				_viewport_points[i][j] = 1

	queue_redraw()
