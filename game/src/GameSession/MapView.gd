extends Node3D

signal province_selected(identifier : String)

const _action_north : StringName = &"map_north"
const _action_east : StringName = &"map_east"
const _action_south : StringName = &"map_south"
const _action_west : StringName = &"map_west"
const _action_zoomin : StringName = &"map_zoomin"
const _action_zoomout : StringName = &"map_zoomout"
const _action_drag : StringName = &"map_drag"
const _action_click : StringName = &"map_click"

const _shader_param_provinces : StringName = &"province_tex"
const _shader_param_hover_pos : StringName = &"hover_pos"
const _shader_param_selected_pos : StringName = &"selected_pos"

@export var _camera : Camera3D

@export var _move_speed : float = 1.0
@export var _edge_move_threshold: float = 50.0
@export var _edge_move_speed: float = 0.02
@export var _dragSensitivity: float = 0.005

@export var _zoom_target_min : float = 0.2
@export var _zoom_target_max : float = 5.0
@export var _zoom_target_step : float = 0.1
@export var _zoom_epsilon : float = _zoom_target_step * 0.1
@export var _zoom_speed : float = 5.0
@export var _zoom_target : float = 1.0:
	get: return _zoom_target
	set(v): _zoom_target = clamp(v, _zoom_target_min, _zoom_target_max)

@export var _map_mesh : MeshInstance3D
var _map_shader_material : ShaderMaterial
var _map_province_shape_image : Image
var _map_aspect_ratio : float = 1.0
var _map_mesh_corner : Vector2
var _map_mesh_dims : Vector2

var _mouse_pos_window : Vector2 = Vector2(0.5, 0.5)
var _mouse_pos_map : Vector2 = Vector2(0.5, 0.5)
var _viewport_dims : Vector2i = Vector2i(1, 1)

func _ready():
	if _camera == null:
		push_error("MapView's _camera variable hasn't been set!")
		return
	if _map_mesh == null:
		push_error("MapView's _map_mesh variable hasn't been set!")
		return
	_map_province_shape_image = MapSingleton.get_province_shape_image()
	if _map_province_shape_image == null:
		push_error("Failed to get province shape image!")
		return

	# Set map mesh size and get bounds
	_map_aspect_ratio = float(_map_province_shape_image.get_width()) / float(_map_province_shape_image.get_height())
	if _map_mesh.mesh is PlaneMesh:
		# Width is doubled so that the map appears to loop horizontally
		(_map_mesh.mesh as PlaneMesh).size = Vector2(_map_aspect_ratio * 2, 1)
	else:
		push_error("Invalid map mesh class: ", _map_mesh.mesh.get_class(), "(expected PlaneMesh)")
	var map_mesh_aabb := _map_mesh.get_aabb() * _map_mesh.transform
	_map_mesh_corner = Vector2(
		min(map_mesh_aabb.position.x, map_mesh_aabb.end.x),
		min(map_mesh_aabb.position.z, map_mesh_aabb.end.z)
	)
	_map_mesh_dims = abs(Vector2(
		map_mesh_aabb.position.x - map_mesh_aabb.end.x,
		map_mesh_aabb.position.z - map_mesh_aabb.end.z
	))

	var map_material = _map_mesh.get_active_material(0)
	if map_material == null:
		push_error("Map mesh is missing material!")
		return
	if not map_material is ShaderMaterial:
		push_error("Invalid map mesh material class: ", map_material.get_class())
		return
	_map_shader_material = map_material
	var texture := ImageTexture.create_from_image(_map_province_shape_image)
	_map_shader_material.set_shader_parameter(_shader_param_provinces, texture)

func _unhandled_input(event : InputEvent):
	if event.is_action_pressed(_action_click):
		# Check if the mouse is outside of bounds
		var mouse_inside_flag := 0 < _mouse_pos_map.x and _mouse_pos_map.x < 1 and 0 < _mouse_pos_map.y and _mouse_pos_map.y < 1
		if mouse_inside_flag:
			var mouse_pos2D := _mouse_pos_map
			mouse_pos2D.x = mouse_pos2D.x * 2.0 - 0.5
			mouse_pos2D *= Vector2(_map_province_shape_image.get_size())

			var province_colour := _map_province_shape_image.get_pixelv(Vector2i(mouse_pos2D)).to_argb32() & 0xFFFFFF
			var province_identifier := MapSingleton.get_province_identifier_from_colour(province_colour)
			_map_shader_material.set_shader_parameter(_shader_param_selected_pos, _mouse_pos_map)
			province_selected.emit(province_identifier)

	elif event is InputEventMouseMotion and Input.is_action_pressed(_action_drag):
		_camera.position.x -= event.relative.x * _dragSensitivity
		_camera.position.z -= event.relative.y * _dragSensitivity

	elif event.is_action_pressed(_action_zoomin, true):
		_zoom_target -= _zoom_target_step
	elif event.is_action_pressed(_action_zoomout, true):
		_zoom_target += _zoom_target_step

func _physics_process(delta : float):
	_mouse_pos_window = get_viewport().get_mouse_position()
	_viewport_dims = get_viewport().size
	# Process movement
	_move_process(delta)
	_edge_scrolling()
	# Keep within map bounds
	_clamp_over_map()
	# Process zooming
	_zoom_process(delta)
	# Orient based on height
	_update_orientation()
	# Calculate where the mouse lies on the map
	_update_mouse_map_position()

func _edge_scrolling() -> void:
	if _mouse_pos_window.y < _edge_move_threshold:
		_camera.position.z -= _edge_move_speed
	elif _mouse_pos_window.y > _viewport_dims.y - _edge_move_threshold:
		_camera.position.z += _edge_move_speed

	if _mouse_pos_window.x < _edge_move_threshold:
		_camera.position.x -= _edge_move_speed
	elif _mouse_pos_window.x > _viewport_dims.x - _edge_move_threshold:
		_camera.position.x += _edge_move_speed

func _move_process(delta : float) -> void:
	var move := Vector3(
		float(Input.is_action_pressed(_action_east)) - float(Input.is_action_pressed(_action_west)),
		0,
		float(Input.is_action_pressed(_action_south)) - float(Input.is_action_pressed(_action_north))
	)
	# Scale movement speed with height
	move *= _move_speed * _camera.position.y * delta
	_camera.global_translate(move)

func _clamp_over_map() -> void:
	var left := _map_mesh_corner.x + 0.25 * _map_mesh_dims.x
	var longitude := fposmod(_camera.position.x - left, _map_mesh_dims.x * 0.5)
	_camera.position.x = left + longitude
	_camera.position.z = clamp(_camera.position.z, _map_mesh_corner.y, _map_mesh_corner.y + _map_mesh_dims.y)

func _zoom_process(delta : float) -> void:
	var height := _camera.position.y
	var zoom := _zoom_target - height
	height += zoom * _zoom_speed * delta
	var new_zoom := _zoom_target - height
	# Set to target if height is within _zoom_epsilon of it or has overshot past it
	if abs(new_zoom) < _zoom_epsilon or sign(zoom) != sign(new_zoom):
		height = _zoom_target
	_camera.position.y = height

func _update_orientation() -> void:
	var dir := Vector3(0, -1, -exp(-_camera.position.y * 2.0 + 0.5))
	_camera.look_at(_camera.position + dir)

func _update_mouse_map_position() -> void:
	var ray_origin := _camera.project_ray_origin(_mouse_pos_window)
	var ray_normal := _camera.project_ray_normal(_mouse_pos_window)
	# Plane with normal (0,1,0) facing upwards, at a distance 0 from the origin
	var intersection = Plane(0, 1, 0, 0).intersects_ray(ray_origin, ray_normal)
	if typeof(intersection) == TYPE_VECTOR3:
		var intersection_vec := intersection as Vector3
		# This loops both horizontally (good) and vertically (bad)
		_mouse_pos_map = (Vector2(intersection_vec.x, intersection_vec.z) - _map_mesh_corner) / _map_mesh_dims
		_map_shader_material.set_shader_parameter(_shader_param_hover_pos, _mouse_pos_map)
	else:
		# Normals parallel to the xz-plane could cause null intersections,
		# but the camera's orientation should prevent such normals
		push_error("Invalid intersection: ", intersection)
