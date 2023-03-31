extends Node3D

const _action_north : StringName = &"map_north"
const _action_east : StringName = &"map_east"
const _action_south : StringName = &"map_south"
const _action_west : StringName = &"map_west"
const _action_zoomin : StringName = &"map_zoomin"
const _action_zoomout : StringName = &"map_zoomout"

@export var _move_speed : float = 1.0

@export var _zoom_target_min : float = 0.2
@export var _zoom_target_max : float = 5.0
@export var _zoom_target_step : float = 0.1
@export var _zoom_epsilon : float = _zoom_target_step * 0.1
@export var _zoom_speed : float = 5.0
@export var _zoom_target : float = 1.0:
	get: return _zoom_target
	set(v): _zoom_target = clamp(v, _zoom_target_min, _zoom_target_max)

@export var _camera : Camera3D
@export var _map_mesh : MeshInstance3D

func _input(event : InputEvent):
	if event.is_action_pressed(_action_zoomin, true):
		_zoom_target -= _zoom_target_step
	elif event.is_action_pressed(_action_zoomout, true):
		_zoom_target += _zoom_target_step

func _physics_process(delta):
	# Process movement
	var height : float = _camera.transform.origin.y
	var move := Vector3(
		float(Input.is_action_pressed(_action_east)) - float(Input.is_action_pressed(_action_west)),
		0.0,
		float(Input.is_action_pressed(_action_south)) - float(Input.is_action_pressed(_action_north)),
	)
	move *= _move_speed * height * delta
	_camera.global_translate(move)

	# Keep within map bounds
	var bounds := _map_mesh.get_aabb() * _map_mesh.transform
	var width := bounds.end.x - bounds.position.x
	var left := bounds.position.x + 0.25 * width
	var longitude := fposmod(_camera.transform.origin.x - left, width * 0.5)
	_camera.transform.origin.x = left + longitude
	_camera.transform.origin.z = clamp(_camera.transform.origin.z, bounds.position.z, bounds.end.z)

	# Process zooming
	var zoom : float = _zoom_target - height
	height += zoom * _zoom_speed * delta
	var new_zoom : float = _zoom_target - height
	if abs(new_zoom) < _zoom_epsilon or sign(zoom) != sign(new_zoom):
		height = _zoom_target
	_camera.transform.origin.y = height

	# Orient based on height
	var dir := Vector3(0, -1, -exp(-height * 2.0 + 0.5))
	_camera.look_at(_camera.transform.origin + dir)
