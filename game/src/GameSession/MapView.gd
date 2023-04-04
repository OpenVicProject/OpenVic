extends Node3D

var ProvScene = preload("res://src/GameSession/ProvinceOverviewPanel.tscn")
var ProvinceShape = MapSingleton.get_province_shape_image()

@onready var viewport_size = get_viewport().size

const _action_north : StringName = &"map_north"
const _action_east : StringName = &"map_east"
const _action_south : StringName = &"map_south"
const _action_west : StringName = &"map_west"
const _action_zoomin : StringName = &"map_zoomin"
const _action_zoomout : StringName = &"map_zoomout"
const _action_drag : StringName = &"map_drag"
const _action_click : StringName = &"map_click"
const _shader_param_provinces : StringName = &"province_tex"
const _shader_param_mouse_pos : StringName = &"mouse_pos"



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
var _map_aspect_ratio : float = 1.0
var _map_mesh_corner : Vector2
var _map_mesh_dims : Vector2
var _mouse_pos : Vector2 = Vector2(0.5, 0.5)
func _ready():
	if _camera == null:
		push_error("MapView's _camera variable hasn't been set!")
		return
	if _map_mesh == null:
		push_error("MapView's _map_mesh variable hasn't been set!")
		return
	var province_shape_image : Image = MapSingleton.get_province_shape_image()
	if province_shape_image == null:
		push_error("Failed to get province shape image!")
		return

	# Set map mesh size and get bounds
	_map_aspect_ratio = float(province_shape_image.get_width()) / float(province_shape_image.get_height())
	if _map_mesh.mesh.get_class() != "PlaneMesh":
		push_error("Invalid map mesh class: ", _map_mesh.mesh.get_class())
	else:
		# Width is doubled so that the map appears to loop horizontally
		(_map_mesh.mesh as PlaneMesh).size = Vector2(_map_aspect_ratio * 2, 1)
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
	if map_material.get_class() != "ShaderMaterial":
		push_error("Invalid map mesh material class: ", map_material.get_class())
		return
	_map_shader_material = map_material
	var texture := ImageTexture.create_from_image(province_shape_image)
	_map_shader_material.set_shader_parameter(_shader_param_provinces, texture)


func _input(event : InputEvent):
	if Input.is_action_pressed(_action_click, true) or event is InputEventMouseMotion:
		# Check if the mouse is outside of bounds
		var _mouse_inside_flag = false
		if _mouse_pos.x > 1.0 or _mouse_pos.x < 0.0 and _mouse_pos.y > 1.0 or _mouse_pos.y < 0.0:
			_mouse_inside_flag = false
		else:
			_mouse_inside_flag = true
			
		# Convert the relative event position from 3D to 2D
		# Could do one-liner but here split for readability
		var mouse_pos2D = _mouse_pos
		mouse_pos2D.x = mouse_pos2D.x * 2.0 - 0.5
		mouse_pos2D.x = mouse_pos2D.x * ProvinceShape.get_size().x
		mouse_pos2D.y = mouse_pos2D.y * ProvinceShape.get_size().y
		
		if Input.is_action_pressed(_action_click, true) and _mouse_inside_flag == true and not event is InputEventMouseMotion:
			var pxColour = ProvinceShape.get_pixel(int(mouse_pos2D.x), int(mouse_pos2D.y))
			get_node('MapMeshInstance').material_override.set_shader_parameter("selection_hover", 1.2)
			if get_parent().has_node("ProvinceOverviewPanel"):
				get_parent().get_node("ProvinceOverviewPanel").ProvinceID = MapSingleton.get_province_id(pxColour.to_html(false))
				get_parent().get_node("ProvinceOverviewPanel").set_id()
			else:
				var Province = ProvScene.instantiate()
				Province.ProvinceID = MapSingleton.get_province_id(pxColour.to_html(false))
				get_parent().add_child(Province)
		else:
			get_node('MapMeshInstance').material_override.set_shader_parameter("selection_hover", 0.8)
#
	
	if event is InputEventMouseMotion and Input.is_action_pressed(_action_drag, true):
		_camera.position.x -= event.relative.x * _dragSensitivity
		_camera.position.z -= event.relative.y * _dragSensitivity

	if event.is_action_pressed(_action_zoomin, true):
		_zoom_target -= _zoom_target_step
	elif event.is_action_pressed(_action_zoomout, true):
		_zoom_target += _zoom_target_step

func _physics_process(delta : float):
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
	var local_mouse_pos = get_viewport().get_mouse_position()
	
	if local_mouse_pos.y < _edge_move_threshold:
		_camera.position.z -= _edge_move_speed
	elif local_mouse_pos.y > get_viewport().size.y - _edge_move_threshold:
		_camera.position.z += _edge_move_speed
	
	if local_mouse_pos.x < _edge_move_threshold:
		_camera.position.x -= _edge_move_speed
	elif local_mouse_pos.x > get_viewport().size.x - _edge_move_threshold:
		_camera.position.x += _edge_move_speed
		
func _move_process(delta : float) -> void:
	var move := Vector3(
		float(Input.is_action_pressed(_action_east)) - float(Input.is_action_pressed(_action_west)),
		0,
		float(Input.is_action_pressed(_action_south)) - float(Input.is_action_pressed(_action_north))
	)
	# Scale movement speed with height
	move *= _move_speed * _camera.transform.origin.y * delta
	_camera.global_translate(move)

func _clamp_over_map() -> void:
	var left := _map_mesh_corner.x + 0.25 * _map_mesh_dims.x
	var longitude := fposmod(_camera.transform.origin.x - left, _map_mesh_dims.x * 0.5)
	_camera.transform.origin.x = left + longitude
	_camera.transform.origin.z = clamp(_camera.transform.origin.z, _map_mesh_corner.y, _map_mesh_corner.y + _map_mesh_dims.y)

func _zoom_process(delta : float) -> void:
	var height := _camera.transform.origin.y
	var zoom := _zoom_target - height
	height += zoom * _zoom_speed * delta
	var new_zoom := _zoom_target - height
	# Set to target if height is within _zoom_epsilon of it or has overshot past it
	if abs(new_zoom) < _zoom_epsilon or sign(zoom) != sign(new_zoom):
		height = _zoom_target
	_camera.transform.origin.y = height

func _update_orientation() -> void:
	var dir := Vector3(0, -1, -exp(-_camera.transform.origin.y * 2.0 + 0.5))
	_camera.look_at(_camera.transform.origin + dir)

func _update_mouse_map_position() -> void:
	var mouse_pos_window := get_viewport().get_mouse_position()
	var ray_origin := _camera.project_ray_origin(mouse_pos_window)
	var ray_normal := _camera.project_ray_normal(mouse_pos_window)
	# Plane with normal (0,1,0) facing upwards, at a distance 0 from the origin
	var intersection = Plane(0, 1, 0, 0).intersects_ray(ray_origin, ray_normal)
	if typeof(intersection) == TYPE_VECTOR3:
		var intersection_vec := intersection as Vector3
		# This loops both horizontally (good) and vertically (bad)
		_mouse_pos = (Vector2(intersection_vec.x, intersection_vec.z) - _map_mesh_corner) / _map_mesh_dims
		_map_shader_material.set_shader_parameter(_shader_param_mouse_pos, _mouse_pos)
	else:
		# Normals parallel to the xz-plane could cause null intersections,
		# but the camera's orientation should prevent such normals
		push_error("Invalid intersection: ", intersection)
