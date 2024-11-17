extends GUINode

signal game_session_menu_button_pressed
signal ledger_button_pressed
signal search_button_pressed
signal zoom_in_button_pressed
signal zoom_out_button_pressed
signal minimap_clicked(pos_clicked : Vector2)

var _mapmode_button_group : ButtonGroup
# We use this instead of the ButtonGroup's get_buttons() as we can add null
# entries for any missing buttons, ensuring each button is at the right index.
var _mapmode_buttons : Array[GUIIconButton]
var _minimap_icon : GUIIcon
var _viewport_points : PackedVector2Array

# REQUIREMENTS:
# * UI-550, UI-552, UI-554, UI-561, UI-562, UI-563
func _add_mapmode_button() -> void:
	var index : int = _mapmode_buttons.size()
	var button : GUIIconButton = get_gui_icon_button_from_nodepath("./menubar/mapmode_%d" % (index + 1))
	if button:
		button.tooltip_string = GameSingleton.get_mapmode_localisation_key(index)
		button.toggle_mode = true
		button.button_group = _mapmode_button_group
		button.pressed.connect(_mapmode_pressed.bind(index))
	_mapmode_buttons.push_back(button)

func _ready() -> void:
	add_gui_element("menubar", "menubar")

	hide_nodes([
		^"./menubar/messagelog_window", # TODO: implement
		^"./menubar/OPENbutton", # not quite sure what this is
		^"./menubar/menubar_plans_toggle", # TODO: implement, v low priority
		^"./menubar/menubar_plans_open", # TODO: implement, v low priority
		^"./menubar/menubar_mail_bg", # TODO: implement
		^"./menubar/menubar_msg_diplo", # TODO: implement
		^"./menubar/menubar_msg_settings", # TODO: implement
		^"./menubar/menubar_msg_combat", # TODO: implement
		^"./menubar/menubar_msg_diplo", # TODO: implement
		^"./menubar/menubar_msg_unit", # TODO: implement
		^"./menubar/menubar_msg_province", # TODO: implement
		^"./menubar/menubar_msg_event", # TODO: implement
		^"./menubar/menubar_msg_other", # TODO: implement
		^"./menubar/chat_window", # TODO: Multiplayer
	])

	var menubar : Panel = get_panel_from_nodepath(^"./menubar")
	if menubar:
		menubar.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var minimap_bg : GUIIcon = get_gui_icon_from_nodepath(^"./menubar/minimap_bg")
	if minimap_bg:
		minimap_bg.mouse_filter = Control.MOUSE_FILTER_PASS
	var menubar_bg : GUIIcon = get_gui_icon_from_nodepath(^"./menubar/menubar_bg")
	if menubar_bg:
		menubar_bg.mouse_filter = Control.MOUSE_FILTER_PASS

	# TODO: add keyboard shortcuts (and shortcut tooltips) where vanilla does by default + use key bindings in settings

	var menu_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/menu_button")
	if menu_button:
		menu_button.tooltip_string = "M_MENU_BUTTON"
		menu_button.pressed.connect(_on_game_session_menu_button_pressed)

	# TODO: implement ledger
	var ledger_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/ledger_button")
	if ledger_button:
		ledger_button.tooltip_string = "M_LEDGER_BUTTON"
		ledger_button.pressed.connect(_on_ledger_button_pressed)

	var search_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/button_goto")
	if search_button:
		search_button.tooltip_string = "M_GOTO_BUTTON"
		search_button.pressed.connect(_on_search_button_pressed)

	var zoom_in_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/map_zoom_in")
	if zoom_in_button:
		zoom_in_button.pressed.connect(_on_zoom_in_button_pressed)

	var zoom_out_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/map_zoom_out")
	if zoom_out_button:
		zoom_out_button.pressed.connect(_on_zoom_out_button_pressed)

	_minimap_icon = get_gui_icon_from_node(generate_gui_element("menubar", "minimap_pic"))
	if _minimap_icon:
		_minimap_icon.mouse_filter = Control.MOUSE_FILTER_PASS
		_minimap_icon.gui_input.connect(_minimap_gui_input)
		_minimap_icon.draw.connect(_minimap_draw)
		add_child(_minimap_icon)

	_mapmode_button_group = ButtonGroup.new()
	for index : int in GameSingleton.get_mapmode_count():
		_add_mapmode_button()

	GameSingleton.mapmode_changed.connect(_on_mapmode_changed)

	# This will set the mapmode in GameSingleton which in turn updates the buttons so that the right one is pressed
	_mapmode_pressed(0)

# REQUIREMENTS:
# * UIFUN-10
func _on_game_session_menu_button_pressed() -> void:
	game_session_menu_button_pressed.emit()

func _on_ledger_button_pressed() -> void:
	ledger_button_pressed.emit()

func _on_search_button_pressed() -> void:
	search_button_pressed.emit()

# REQUIREMENTS:
# * SS-76
# * UIFUN-129, UIFUN-131, UIFUN-133, UIFUN-140, UIFUN-141, UIFUN-142
func _mapmode_pressed(index : int) -> void:
	GameSingleton.set_mapmode(index)
	print("Mapmode set to \"%s\" (index: %d, identifier: %s)" % [
		tr(GameSingleton.get_mapmode_localisation_key(index)), index, GameSingleton.get_mapmode_identifier(index)
	])

func _on_mapmode_changed(index : int) -> void:
	var current_mapmode_button : GUIIconButton = _mapmode_button_group.get_pressed_button()
	var new_mapmode_button : GUIIconButton = _mapmode_buttons[index] if 0 <= index and index < _mapmode_buttons.size() else null

	if current_mapmode_button != new_mapmode_button:
		if new_mapmode_button:
			# This will also automatically unpress current_mapmode_button (if it isn't null)
			new_mapmode_button.button_pressed = true
		else:
			# current_mapmode_button can't be null as it isn't equal to new_mapmode_button which is null
			current_mapmode_button.button_pressed = false

# REQUIREMENTS:
# * UIFUN-269
func _on_zoom_in_button_pressed() -> void:
	zoom_in_button_pressed.emit()

# REQUIREMENTS:
# * UIFUN-270
func _on_zoom_out_button_pressed() -> void:
	zoom_out_button_pressed.emit()

# REQUIREMENTS
# * SS-80
# * UI-752
func _minimap_draw() -> void:
	if _viewport_points.size() > 1:
		_minimap_icon.draw_multiline(_viewport_points, Color.WHITE, -1)

# REQUIREMENTS
# * SS-81
# * UIFUN-127
func _minimap_gui_input(_event : InputEvent) -> void:
	const _action_click : StringName = &"map_click"
	if Input.is_action_pressed(_action_click):
		var pos_clicked : Vector2 = _minimap_icon.get_local_mouse_position() / _minimap_icon.size - Vector2(0.5, 0.5)
		if abs(pos_clicked.x) < 0.5 and abs(pos_clicked.y) < 0.5:
			minimap_clicked.emit(pos_clicked)

# Returns the point on the line going through p and q with the specific x coord
func _intersect_x(p : Vector2, q : Vector2, x : float) -> Vector2:
	if p.x == q.x:
		return Vector2(x, 0.5 * (p.y + q.y))
	var t : float = (x - q.x) / (p.x - q.x)
	return q + t * (p - q)

# Returns the point on the line going through p and q with the specific y coord
func _intersect_y(p : Vector2, q : Vector2, y : float) -> Vector2:
	if p.y == q.y:
		return Vector2(0.5 * (p.x + q.x), y)
	var t : float = (y - q.y) / (p.y - q.y)
	return q + t * (p - q)

func _add_line_looped_over_x(left : Vector2, right : Vector2) -> void:
	const _one_x : Vector2 = Vector2(1, 0)
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

	if _minimap_icon:
		for i : int in _viewport_points.size():
			_viewport_points[i] *= _minimap_icon.size
		_minimap_icon.queue_redraw()
