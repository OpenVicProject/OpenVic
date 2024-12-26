extends GUINode

@export var _map_view: MapView

var _search_panel: Panel
var _search_line_edit: LineEdit
var _results_list_box: GUIListBox
var _result_buttons: Array[GUIIconButton]

var _drag_active: bool = false
var _drag_anchor: Vector2


func _ready() -> void:
	MenuSingleton.search_cache_changed.connect(_update_results_base)

	add_gui_element("goto", "goto_box")

	remove_node(^"./goto_box/goto")

	_search_panel = get_panel_from_nodepath(^"./goto_box")

	set_click_mask_from_nodepaths([^"./goto_box/goto_box"])

	var close_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./goto_box/cancel")
	if close_button:
		close_button.pressed.connect(hide)

	var panel_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./goto_box/goto_box")
	if panel_button:
		panel_button.button_down.connect(_start_drag)
		panel_button.button_up.connect(_end_drag)
		if _search_panel:
			# Move to back so it's not drawn over the results list
			_search_panel.move_child(panel_button, 0)

	_search_line_edit = get_line_edit_from_nodepath(^"./goto_box/goto_edit")
	if _search_line_edit:
		_search_line_edit.text_changed.connect(_search_string_updated)
		# Restrict to desired size (by default it's a bit too tall, probably due to font size)
		_search_line_edit.set_size(_search_line_edit.get_minimum_size())

	_results_list_box = get_gui_listbox_from_nodepath(^"./goto_box/provinces")
	if _results_list_box:
		_results_list_box.scroll_index_changed.connect(_update_results_scroll)

		_results_list_box.set_position(_results_list_box.get_position() - Vector2(4, 0))

	hide()

	MenuSingleton.generate_search_cache()


func toggle_visibility() -> void:
	if is_visible():
		hide()
	else:
		show()
		if _search_line_edit:
			_search_line_edit.grab_focus()


func _start_drag() -> void:
	if _search_panel:
		_drag_anchor = _search_panel.get_position() - get_window().get_mouse_position()
		_drag_active = true


func _end_drag() -> void:
	_drag_active = false


func _input(event: InputEvent) -> void:
	if _drag_active and event is InputEventMouseMotion:
		_search_panel.set_position(_drag_anchor + get_window().get_mouse_position())


func _notification(what: int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			MenuSingleton.generate_search_cache()


func _search_string_updated(search_string: String) -> void:
	MenuSingleton.update_search_results(search_string)
	_update_results_base()


func _update_results_base() -> void:
	if not _results_list_box:
		return

	var result_count: int = MenuSingleton.get_search_result_row_count()

	var result_height: float = 0.0
	if result_count > 0 and (_results_list_box.get_child_count() > 0 or _add_result_button()):
		result_height = _results_list_box.get_child(0).get_size().y

	_results_list_box.set_fixed(result_count, result_height, false)
	_update_results_scroll()


func _add_result_button() -> bool:
	if not _results_list_box:
		return false

	var child: Panel = GUINode.generate_gui_element("menubar", "save_game_entry")
	if not child:
		return false

	var button: GUIIconButton = GUINode.get_gui_icon_button_from_node(child.get_node(^"./game"))
	if not button:
		child.queue_free()
		return false

	button.pressed.connect(_result_selected.bind(_result_buttons.size()))
	# Country/State/Province display names are already translated in the MenuSingleton
	button.auto_translate = false

	_results_list_box.add_child(child)
	_result_buttons.push_back(button)

	return true


func _update_results_scroll(scroll_index: int = -1) -> void:
	if not _results_list_box:
		return

	if scroll_index >= 0:
		_results_list_box.set_scroll_index(scroll_index, false)

	scroll_index = _results_list_box.get_scroll_index()

	var results: PackedStringArray = MenuSingleton.get_search_result_rows(
		scroll_index, _results_list_box.get_fixed_visible_items()
	)

	if results.size() < _result_buttons.size():
		_result_buttons.resize(results.size())
		_results_list_box.clear_children(results.size())
	else:
		while _result_buttons.size() < results.size() and _add_result_button():
			pass  # Button is added in the loop condition

	for index: int in min(results.size(), _result_buttons.size()):
		_result_buttons[index].set_text(results[index])


func _result_selected(index: int) -> void:
	if _map_view:
		_map_view.look_at_map_position(MenuSingleton.get_search_result_position(index))
	else:
		push_error("SearchPanel missing MapView reference!")

	if _search_line_edit:
		# This triggers a search results update, preventing further get_search_result_position(index) calls
		_search_line_edit.clear()

	hide()
