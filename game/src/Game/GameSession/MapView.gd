class_name MapView
extends Node3D

signal map_view_camera_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2)
signal parchment_view_changed(is_parchment_view : bool)
signal detailed_view_changed(is_detailed_view : bool)

const _action_north : StringName = &"map_north"
const _action_east : StringName = &"map_east"
const _action_south : StringName = &"map_south"
const _action_west : StringName = &"map_west"
const _action_zoom_in : StringName = &"map_zoom_in"
const _action_zoom_out : StringName = &"map_zoom_out"
const _action_drag : StringName = &"map_drag"
const _action_click : StringName = &"map_click"
const _action_right_click : StringName = &"map_right_click"
const _action_select_add : StringName = &"select_add"

@export var _camera : Camera3D

@export var _cardinal_move_speed : float = 2.5
@export var _edge_move_threshold: float = 0.025
@export var _edge_move_speed: float = 2.5
var _drag_anchor : Vector2
var _drag_active : bool = false

var _mouse_over_viewport : bool = true

@export var _zoom_target_min : float = 0.075
@export var _zoom_target_max : float = 5.0
@export var _zoom_target_step : float = (_zoom_target_max - _zoom_target_min) / 64.0
@export var _zoom_epsilon : float = _zoom_target_step * 0.005
@export var _zoom_speed : float = 5.0
# _zoom_target's starting value is ignored as it is updated to the camera's height by _ready,
# hence why it is not exported and just has _zoom_target_max as a placeholder.
var _zoom_target : float = _zoom_target_max:
	get: return _zoom_target
	set(v): _zoom_target = clamp(v, _zoom_target_min, _zoom_target_max)
const _zoom_position_multiplier = 3.14159 # Horizontal movement coefficient during zoom
var _zoom_position : Vector2

# Display the parchment map above this height
@export var _zoom_parchment_threshold : float = _zoom_target_min + (_zoom_target_max - _zoom_target_min) / 4
# Display details like models and province names below this height
@export var _zoom_detailed_threshold : float = _zoom_parchment_threshold / 2

var _is_parchment_view : bool = false
var _is_detailed_view : bool = false

@export var _map_mesh_instance : MeshInstance3D
var _map_mesh : MapMesh
var _map_shader_material : ShaderMaterial
var _map_mesh_corner : Vector2
var _map_mesh_dims : Vector2

@export var _map_background_instance : MeshInstance3D

var _mouse_pos_viewport : Vector2 = Vector2(0.5, 0.5)
var _mouse_pos_map : Vector2 = Vector2(0.5, 0.5)
var _viewport_dims : Vector2 = Vector2(1, 1)

@export var _map_text : MapText

@export var validMoveMarkers : ValidMoveMarkers
@export var selectionMarkers : SelectionMarkers
var land_units_selected : Array = []
var naval_units_selected : Array = []

# ??? Strange Godot/GDExtension Bug ???
# Upon first opening a clone of this repo with the Godot Editor,
# if GameSingleton.get_province_index_image is called before MapMesh
# is referenced in the script below, then the editor will crash due
# to a failed HashMap lookup. I'm not sure if this is a bug in the
# editor, GDExtension, my own extension, or a combination of them.
# This was an absolute pain to track down. --- hop311
func _ready() -> void:
	if not _camera:
		push_error("MapView's _camera variable hasn't been set!")
		return

	# Start just under the parchment threshold
	_camera.position.y = _zoom_parchment_threshold - _zoom_target_step
	_zoom_target = _camera.position.y
	_update_view_states(true)

	if not _map_mesh_instance:
		push_error("MapView's _map_mesh_instance variable hasn't been set!")
		return

	# Shader Material
	var map_material := _map_mesh_instance.get_active_material(0)
	if GameLoader.ShaderManager.set_up_shader(map_material, true) != OK:
		push_error("Failed to set up map shader")
		return
	_map_shader_material = map_material

	if not _map_mesh_instance.mesh is MapMesh:
		push_error("Invalid map mesh class: ", _map_mesh_instance.mesh.get_class(), "(expected MapMesh)")
		return
	_map_mesh = _map_mesh_instance.mesh

	_map_mesh.set_aspect_ratio(GameSingleton.get_map_aspect_ratio())

	# Get map mesh bounds
	var map_mesh_aabb : AABB = _map_mesh.get_core_aabb() * _map_mesh_instance.transform
	_map_mesh_corner = Vector2(
		min(map_mesh_aabb.position.x, map_mesh_aabb.end.x),
		min(map_mesh_aabb.position.z, map_mesh_aabb.end.z)
	)
	_map_mesh_dims = abs(Vector2(
		map_mesh_aabb.position.x - map_mesh_aabb.end.x,
		map_mesh_aabb.position.z - map_mesh_aabb.end.z
	))

	GameSingleton.province_selected.connect(_on_province_selected)

	if not _map_background_instance:
		push_error("MapView's _map_background_instance variable hasn't been set!")
		return

	if not _map_background_instance.mesh is PlaneMesh:
		push_error("Invalid map background mesh class: ", _map_background_instance.mesh.get_class(), "(expected PlaneMesh)")
		return
	var scaled_dims : Vector3 = _map_background_instance.transform.affine_inverse() * Vector3(_map_mesh_dims.x, 0.0, _map_mesh_dims.y)
	scaled_dims.x *= 1.0 + 2.0 * _map_mesh.get_repeat_proportion()
	scaled_dims.z *= 2.0
	(_map_background_instance.mesh as PlaneMesh).set_size(Vector2(scaled_dims.x, scaled_dims.z))

	_map_text.generate_map_names()

func _notification(what: int) -> void:
	if what == NOTIFICATION_WM_MOUSE_EXIT:
		_mouse_over_viewport = false
		unset_hovered_province()

func _world_to_map_coords(pos : Vector3) -> Vector2:
	return (Vector2(pos.x, pos.z) - _map_mesh_corner) / _map_mesh_dims

func _map_to_world_coords(pos : Vector2) -> Vector3:
	pos = pos * _map_mesh_dims + _map_mesh_corner
	return Vector3(pos.x, 0, pos.y)

func _viewport_to_world_coords(pos_viewport : Vector2) -> Vector3:
	var ray_origin := _camera.project_ray_origin(pos_viewport)
	var ray_normal := _camera.project_ray_normal(pos_viewport)
	# Plane with normal (0,1,0) facing upwards, at a distance 0 from the origin
	var intersection : Variant = Plane(0, 1, 0, 0).intersects_ray(ray_origin, ray_normal)
	if typeof(intersection) == TYPE_VECTOR3:
		return intersection
	else:
		# Normals parallel to the xz-plane could cause null intersections,
		# but the camera's orientation should prevent such normals
		push_error("Invalid intersection: ", intersection)
		return _map_to_world_coords(Vector2(0.5, 0.5))

func _viewport_to_map_coords(pos_viewport : Vector2) -> Vector2:
	return _world_to_map_coords(_viewport_to_world_coords(pos_viewport))

func look_at_map_position(pos : Vector2) -> void:
	var viewport_centre : Vector2 = Vector2(0.5, 0.5) * _viewport_dims / GuiScale.get_current_guiscale()
	var pos_delta : Vector3 = _map_to_world_coords(pos) - _viewport_to_world_coords(viewport_centre)
	_camera.position.x += pos_delta.x
	_camera.position.z += pos_delta.z

func zoom_in() -> void:
	_zoom_target -= _zoom_target_step
	_zoom_position = (Vector2(0.5, 0.5) - _mouse_pos_viewport * GuiScale.get_current_guiscale() / _viewport_dims) * _zoom_position_multiplier

func zoom_out() -> void:
	_zoom_target += _zoom_target_step
	# For some reason, zooming out in the original game does not consider the
	# cursor location. I'm not sure if we want to preserve this behavior.
	_zoom_position = Vector2()

func set_hovered_province_index(hover_index : int) -> void:
	_map_shader_material.set_shader_parameter(GameLoader.ShaderManager.param_hover_index, hover_index)

func set_hovered_province_at(pos : Vector2) -> void:
	var hover_index := GameSingleton.get_province_index_from_uv_coords(pos)
	set_hovered_province_index(hover_index)

func unset_hovered_province() -> void:
	set_hovered_province_index(0)

var _province_hover_dirty := false
func queue_province_hover_update() -> void:
	if not _mouse_over_viewport: return
	_province_hover_dirty = true

func _update_province_hover() -> void:
	if not _province_hover_dirty: return
	_province_hover_dirty = false
	if _mouse_over_viewport:
		set_hovered_province_at(_viewport_to_map_coords(_mouse_pos_viewport))

func _on_province_selected(index : int) -> void:
	_map_shader_material.set_shader_parameter(GameLoader.ShaderManager.param_selected_index, index)
	print("Province selected with index: ", index)

func _input(event : InputEvent) -> void:
	if event is InputEventMouseMotion:
		_mouse_pos_viewport = get_window().get_mouse_position()
	elif _drag_active and event.is_action_released(_action_drag):
		_drag_active = false

# REQUIREMENTS
# * SS-31
# * SS-75
var _cardinal_movement_vector := Vector2.ZERO
var temp_id : int = 0
func _unhandled_input(event : InputEvent) -> void:
	if event is InputEventMouseMotion:
		_mouse_over_viewport = true
		queue_province_hover_update()

	elif event.is_action(_action_north) or event.is_action(_action_south)\
		or event.is_action(_action_east) or event.is_action(_action_west):
			_cardinal_movement_vector = Input.get_vector(
				_action_west,
				_action_east,
				_action_north,
				_action_south
			) * _cardinal_move_speed

	elif event.is_action_pressed(_action_select_add):
		if _mouse_over_viewport:
			if _map_mesh.is_valid_uv_coord(_mouse_pos_map):
				GameSingleton.set_selected_province(GameSingleton.get_province_index_from_uv_coords(_mouse_pos_map))
				var province_index : int = GameSingleton.get_province_index_from_uv_coords(_mouse_pos_map)
				
				var port_province_index : int = MapItemSingleton.get_clicked_port_province_index(_mouse_pos_map)
				if port_province_index != 0:
					var port_pos : Vector2 = MapItemSingleton.get_port_position_by_province_index(port_province_index)
					selectionMarkers.add_selection_marker(temp_id,_map_to_world_coords(port_pos))
				else:
					var unit_position : Vector2 = MapItemSingleton.get_unit_position_by_province_index(province_index)
					selectionMarkers.add_selection_marker(temp_id,_map_to_world_coords(unit_position))
				temp_id += 1

	elif event.is_action_pressed(_action_click):
		if _mouse_over_viewport:
			# Check if the mouse is outside of bounds
			if _map_mesh.is_valid_uv_coord(_mouse_pos_map):
				selectionMarkers.clear_selection_markers()
			else:
				print("Clicked outside the map!")
	elif event.is_action_pressed(_action_right_click):
		if _mouse_over_viewport:
			if _map_mesh.is_valid_uv_coord(_mouse_pos_map):
				var province_index : int = GameSingleton.get_province_index_from_uv_coords(_mouse_pos_map)
				var port_province_index : int = MapItemSingleton.get_clicked_port_province_index(_mouse_pos_map)
				if port_province_index != 0:
					var port_pos : Vector2 = MapItemSingleton.get_port_position_by_province_index(port_province_index)
					validMoveMarkers.add_move_marker(_map_to_world_coords(port_pos), randi_range(0,1))
				else:
					var unit_position : Vector2 = MapItemSingleton.get_unit_position_by_province_index(province_index)
					validMoveMarkers.add_move_marker(_map_to_world_coords(unit_position), randi_range(0,1))
				# TODO - open diplomacy screen on province owner or viewed country if province has no owner
				#Events.NationManagementScreens.open_nation_management_screen(NationManagement.Screen.DIPLOMACY)
				GameSingleton.set_viewed_country_by_province_index(province_index)
			else:
				print("Right-clicked outside the map!")
	elif event.is_action_pressed(_action_drag):
		if _drag_active:
			push_warning("Drag being activated while already active!")
		_drag_active = true
		_drag_anchor = _mouse_pos_map
	elif event.is_action_pressed(_action_zoom_in, true):
		zoom_in()
	elif event.is_action_pressed(_action_zoom_out, true):
		zoom_out()

func _process(delta : float) -> void:
	if _cardinal_movement_vector != Vector2.ZERO and get_window().gui_get_focus_owner() != null:
		_cardinal_movement_vector = Vector2.ZERO

	if _is_viewport_inactive():
		_mouse_over_viewport = false
		unset_hovered_province()

	_viewport_dims = Vector2(Resolution.get_current_resolution())
	# Process movement
	_movement_process(delta)
	# Keep within map bounds
	_clamp_over_map()
	# Process zooming
	_zoom_process(delta)
	# Orient based on height
	_update_orientation()
	# Update viewport on minimap
	_update_minimap_viewport()
	# Calculate where the mouse lies on the map
	_update_mouse_map_position()
	# Update province hover if dirty
	_update_province_hover()

# REQUIREMENTS
# * UIFUN-124
func _movement_process(delta : float) -> void:
	var direction : Vector2
	if _drag_active:
		direction = (_drag_anchor - _mouse_pos_map) * _map_mesh_dims
	else:
		direction = _edge_scrolling_vector() + _cardinal_movement_vector
		if direction != Vector2.ZERO:
			queue_province_hover_update()
		# Scale movement speed with height
		direction *= _camera.position.y * delta
	_camera.position += Vector3(direction.x, 0, direction.y)

# REQUIREMENTS
# * UIFUN-125
func _edge_scrolling_vector() -> Vector2:
	if not _mouse_over_viewport:
		return Vector2()
	var mouse_vector := _mouse_pos_viewport * GuiScale.get_current_guiscale() / _viewport_dims - Vector2(0.5, 0.5)
	# Only scroll if outside the move threshold.
	if abs(mouse_vector.x) < 0.5 - _edge_move_threshold and abs(mouse_vector.y) < 0.5 - _edge_move_threshold:
		return Vector2()
	return mouse_vector * _edge_move_speed

func _clamp_over_map() -> void:
	_camera.position.x = _map_mesh_corner.x + fposmod(_camera.position.x - _map_mesh_corner.x, _map_mesh_dims.x)
	_camera.position.z = clamp(_camera.position.z, _map_mesh_corner.y, _map_mesh_corner.y + _map_mesh_dims.y)

func _update_view_states(force_signal : bool) -> void:
	var new_is_parchment_view : bool = _camera.position.y >= _zoom_parchment_threshold - _zoom_epsilon
	if force_signal or new_is_parchment_view != _is_parchment_view:
		_is_parchment_view = new_is_parchment_view
		parchment_view_changed.emit(_is_parchment_view)

	var new_is_detailed_view : bool = _camera.position.y <= _zoom_detailed_threshold + _zoom_epsilon
	if force_signal or new_is_detailed_view != _is_detailed_view:
		_is_detailed_view = new_is_detailed_view
		detailed_view_changed.emit(_is_detailed_view)

# REQUIREMENTS
# * SS-74
# * UIFUN-123
func _zoom_process(delta : float) -> void:
	var height := _camera.position.y
	var zoom := _zoom_target - height
	var zoom_delta := zoom * _zoom_speed * delta
	# Set to target if height is within _zoom_epsilon of it or has overshot past it
	if abs(zoom - zoom_delta) < _zoom_epsilon or sign(zoom) != sign(zoom - zoom_delta):
		zoom_delta = zoom
	else:
		queue_province_hover_update()
	_camera.position += Vector3(
		_zoom_position.x * zoom_delta * int(_mouse_over_viewport),
		zoom_delta,
		_zoom_position.y * zoom_delta * int(_mouse_over_viewport)
	)
	# TODO - smooth transition similar to smooth zoom
	_update_view_states(false)
	var parchment_mapmode : bool = GameSingleton.is_parchment_mapmode_allowed() and _is_parchment_view
	_map_shader_material.set_shader_parameter(GameLoader.ShaderManager.param_parchment_mix, float(parchment_mapmode))

func _update_orientation() -> void:
	const up := Vector3(0, 0, -1)
	var dir := Vector3(0, -1, 0)
	if _is_detailed_view:
		# Zero at the transition point, increases as you zoom further in
		var delta : float = (_zoom_detailed_threshold - _camera.position.y) / _zoom_detailed_threshold
		dir.z = -(delta ** 2)
	_camera.look_at(_camera.position + dir, up)

func _update_minimap_viewport() -> void:
	var near_left := _viewport_to_map_coords(Vector2(0, _viewport_dims.y))
	var far_left := _viewport_to_map_coords(Vector2(0, 0))
	var far_right := _viewport_to_map_coords(Vector2(_viewport_dims.x, 0))
	var near_right := _viewport_to_map_coords(_viewport_dims)
	map_view_camera_changed.emit(near_left, far_left, far_right, near_right)

func _update_mouse_map_position() -> void:
	_mouse_pos_map = _viewport_to_map_coords(_mouse_pos_viewport)

func _on_minimap_clicked(pos_clicked : Vector2) -> void:
	pos_clicked *= _map_mesh_dims
	_camera.position.x = pos_clicked.x
	_camera.position.z = pos_clicked.y
	_clamp_over_map()
	queue_province_hover_update()

func _is_viewport_inactive() -> bool:
	return not get_window().has_focus() or get_window().is_input_handled()

func enable_processing() -> void:
	set_process_unhandled_input(true)
	set_process(true)

func disable_processing() -> void:
	set_process_unhandled_input(false)
	set_process(false)
