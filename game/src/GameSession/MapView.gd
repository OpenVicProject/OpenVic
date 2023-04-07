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

const _shader_param_province_index : StringName = &"province_index_tex"
const _shader_param_province_colour : StringName = &"province_colour_tex"
const _shader_param_hover_pos : StringName = &"hover_pos"
const _shader_param_selected_pos : StringName = &"selected_pos"

@export var _camera : Camera3D

@export var _cardinal_move_speed : float = 1.0
@export var _edge_move_threshold: float = 0.15
@export var _edge_move_speed: float = 2.5
var _drag_anchor : Vector2
var _drag_active : bool = false

@export var _zoom_target_min : float = 0.2
@export var _zoom_target_max : float = 5.0
@export var _zoom_target_step : float = 0.1
@export var _zoom_epsilon : float = _zoom_target_step * 0.1
@export var _zoom_speed : float = 5.0
@export var _zoom_target : float = 1.0:
	get: return _zoom_target
	set(v): _zoom_target = clamp(v, _zoom_target_min, _zoom_target_max)

@export var _map_mesh_instance : MeshInstance3D
var _map_mesh : MapMesh
var _map_shader_material : ShaderMaterial
var _map_image_size : Vector2
var _map_province_index_image : Image
var _map_mesh_corner : Vector2
var _map_mesh_dims : Vector2

var _mouse_pos_viewport : Vector2 = Vector2(0.5, 0.5)
var _mouse_pos_map : Vector2 = Vector2(0.5, 0.5)

func _ready():
	if _camera == null:
		push_error("MapView's _camera variable hasn't been set!")
		return
	if _map_mesh_instance == null:
		push_error("MapView's _map_mesh variable hasn't been set!")
		return
	if not _map_mesh_instance.mesh is MapMesh:
		push_error("Invalid map mesh class: ", _map_mesh_instance.mesh.get_class(), "(expected MapMesh)")
		return
	_map_mesh = _map_mesh_instance.mesh

	# Set map mesh size and get bounds
	_map_image_size = Vector2(Vector2i(MapSingleton.get_width(), MapSingleton.get_height()))
	_map_mesh.aspect_ratio = _map_image_size.x / _map_image_size.y
	var map_mesh_aabb := _map_mesh.get_core_aabb() * _map_mesh_instance.transform
	_map_mesh_corner = Vector2(
		min(map_mesh_aabb.position.x, map_mesh_aabb.end.x),
		min(map_mesh_aabb.position.z, map_mesh_aabb.end.z)
	)
	_map_mesh_dims = abs(Vector2(
		map_mesh_aabb.position.x - map_mesh_aabb.end.x,
		map_mesh_aabb.position.z - map_mesh_aabb.end.z
	))

	var map_material = _map_mesh_instance.get_active_material(0)
	if map_material == null:
		push_error("Map mesh is missing material!")
		return
	if not map_material is ShaderMaterial:
		push_error("Invalid map mesh material class: ", map_material.get_class())
		return
	_map_shader_material = map_material
	# Province index texture
	_map_province_index_image = MapSingleton.get_province_index_image()
	if _map_province_index_image == null:
		push_error("Failed to get province index image!")
		return
	var province_index_texture := ImageTexture.create_from_image(_map_province_index_image)
	_map_shader_material.set_shader_parameter(_shader_param_province_index, province_index_texture)
	# Province colour texture
	var province_colour_image = MapSingleton.get_province_colour_image()
	if province_colour_image == null:
		push_error("Failed to get province colour image!")
		return
	var province_colour_texture := ImageTexture.create_from_image(province_colour_image)
	_map_shader_material.set_shader_parameter(_shader_param_province_colour, province_colour_texture)

func _unhandled_input(event : InputEvent):
	if event.is_action_pressed(_action_click):
		# Check if the mouse is outside of bounds
		if _map_mesh.is_valid_uv_coord(_mouse_pos_map):
			_map_shader_material.set_shader_parameter(_shader_param_selected_pos, _mouse_pos_map)
			var mouse_pixel_pos := Vector2i(_mouse_pos_map * _map_image_size)
			var province_identifier := MapSingleton.get_province_identifier_from_pixel_coords(mouse_pixel_pos)
			province_selected.emit(province_identifier)
	elif event.is_action_pressed(_action_drag):
		if _drag_active:
			push_warning("Drag being activated while already active!")
		_drag_active = true
		_drag_anchor = _mouse_pos_map
	elif event.is_action_released(_action_drag):
		if not _drag_active:
			push_warning("Drag being deactivated while already not active!")
		_drag_active = false
	elif event.is_action_pressed(_action_zoomin, true):
		_zoom_target -= _zoom_target_step
	elif event.is_action_pressed(_action_zoomout, true):
		_zoom_target += _zoom_target_step

func _physics_process(delta : float):
	_mouse_pos_viewport = get_viewport().get_mouse_position()
	# Process movement
	_movement_process(delta)
	# Keep within map bounds
	_clamp_over_map()
	# Process zooming
	_zoom_process(delta)
	# Orient based on height
	_update_orientation()
	# Calculate where the mouse lies on the map
	_update_mouse_map_position()

func _movement_process(delta : float) -> void:
	var direction : Vector2
	if _drag_active:
		direction = (_drag_anchor - _mouse_pos_map) * _map_mesh_dims
	else:
		direction = _edge_scrolling_vector() + _cardinal_movement_vector()
		# Scale movement speed with height
		direction *= _camera.position.y * delta
	_camera.position += Vector3(direction.x, 0, direction.y)

func _edge_scrolling_vector() -> Vector2:
	var viewport_dims := Vector2(Resolution.get_current_resolution())
	var mouse_vector := _mouse_pos_viewport / viewport_dims - Vector2(0.5, 0.5);
	if pow(mouse_vector.x, 4) + pow(mouse_vector.y, 4) < pow(0.5 - _edge_move_threshold, 4):
		mouse_vector *= 0
	return mouse_vector * _edge_move_speed

func _cardinal_movement_vector() -> Vector2:
	var move := Vector2(
		float(Input.is_action_pressed(_action_east)) - float(Input.is_action_pressed(_action_west)),
		float(Input.is_action_pressed(_action_south)) - float(Input.is_action_pressed(_action_north))
	)
	return move * _cardinal_move_speed

func _clamp_over_map() -> void:
	_camera.position.x = _map_mesh_corner.x + fposmod(_camera.position.x - _map_mesh_corner.x, _map_mesh_dims.x)
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
	var ray_origin := _camera.project_ray_origin(_mouse_pos_viewport)
	var ray_normal := _camera.project_ray_normal(_mouse_pos_viewport)
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
