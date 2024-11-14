extends GUINode

signal game_session_menu_button_pressed
signal search_button_pressed
signal map_view_camera_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2)
signal zoom_in_button_pressed
signal zoom_out_button_pressed

var _mapmode_button_group : ButtonGroup

# REQUIREMENTS:
# * UI-550, UI-552, UI-554, UI-561, UI-562, UI-563
func _add_mapmode_button(index : int) -> void:
	var button: GUIIconButton = get_gui_icon_button_from_nodepath("./menubar/mapmode_{i}".format({"i":index+1}))
	if button:
		button.tooltip_string = tr(GameSingleton.get_mapmode_identifier(index)) #tr("MAPMODE_{i}".format({"i":index+1})) <-- TODO: use for vanilla mapmodes when that gets changed on sim
		button.toggle_mode = true
		button.button_group = _mapmode_button_group
		button.pressed.connect(
			func() -> void:
				_mapmode_pressed(index)
		)
		if _mapmode_button_group.get_pressed_button() == null:
			button.button_pressed = true

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

	var menubar: Panel = get_panel_from_nodepath(^"./menubar")
	if menubar:
		menubar.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var minimap_bg: GUIIcon = get_gui_icon_from_nodepath(^"./menubar/minimap_bg")
	if minimap_bg:
		minimap_bg.mouse_filter = Control.MOUSE_FILTER_PASS
	var menubar_bg: GUIIcon = get_gui_icon_from_nodepath(^"./menubar/menubar_bg")
	if menubar_bg:
		menubar_bg.mouse_filter = Control.MOUSE_FILTER_PASS

	# TODO: add keyboard shortcuts (and shortcut tooltips) where vanilla does by default + use key bindings in settings

	var menu_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/menu_button")
	if menu_button:
		menu_button.tooltip_string = tr("M_MENU_BUTTON")
		menu_button.pressed.connect(_on_game_session_menu_button_pressed)

	# TODO: implement ledger
	var ledger_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/ledger_button")
	if ledger_button:
		ledger_button.tooltip_string = tr("M_LEDGER_BUTTON")

	var search_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/button_goto")
	if search_button:
		search_button.tooltip_string = tr("M_GOTO_BUTTON")
		search_button.pressed.connect(_on_search_button_pressed)

	var zoom_in_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/map_zoom_in")
	if zoom_in_button:
		zoom_in_button.pressed.connect(_on_zoom_in_button_pressed)

	var zoom_out_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./menubar/map_zoom_out")
	if zoom_out_button:
		zoom_out_button.pressed.connect(_on_zoom_out_button_pressed)

	add_gui_element("menubar", "minimap_pic")
	var minimap_pic: GUIIcon = get_node(^"./minimap_pic")
	var minimap_rect: Control = get_node(^"./MinimapRect")
	minimap_rect.move_to_front()
	# TODO: make this more v2esque, project our box onto the v2 image. Low priority.
	if minimap_pic:
		minimap_rect.custom_minimum_size = minimap_pic.size
		minimap_rect.position = minimap_pic.position + minimap_pic.size
		minimap_pic.queue_free()
		
	_mapmode_button_group = ButtonGroup.new()
	for index : int in GameSingleton.get_mapmode_count():
		_add_mapmode_button(index)

# REQUIREMENTS:
# * UIFUN-10
func _on_game_session_menu_button_pressed() -> void:
	game_session_menu_button_pressed.emit()

func _on_search_button_pressed() -> void:
	search_button_pressed.emit()

# REQUIREMENTS:
# * SS-76
# * UIFUN-129, UIFUN-131, UIFUN-133, UIFUN-140, UIFUN-141, UIFUN-142
func _mapmode_pressed(index : int) -> void:
	GameSingleton.set_mapmode(GameSingleton.get_mapmode_identifier(index))

func _on_map_view_camera_changed(near_left : Vector2, far_left : Vector2, far_right : Vector2, near_right : Vector2) -> void:
	map_view_camera_changed.emit(near_left, far_left, far_right, near_right)

# REQUIREMENTS:
# * UIFUN-269
func _on_zoom_in_button_pressed() -> void:
	zoom_in_button_pressed.emit()

# REQUIREMENTS:
# * UIFUN-270
func _on_zoom_out_button_pressed() -> void:
	zoom_out_button_pressed.emit()
