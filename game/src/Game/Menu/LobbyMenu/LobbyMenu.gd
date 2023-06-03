extends HBoxContainer

# REQUIREMENTS:
# * 1.4 Game Lobby Menu
# * SS-12

signal back_button_pressed
signal save_game_selected(save : SaveResource)
signal start_date_selected(index : int)

@export var lobby_panel_button : PackedScene
@export var save_scene : PackedScene

@export_group("Nodes")
@export var game_select_start_date : BoxContainer
@export var game_select_save_tab : TabBar
@export var game_select_save_list : BoxContainer
@export var start_button : BaseButton
@export var session_tag_line_edit : LineEdit
@export var session_tag_dialog : ConfirmationDialog
@export var delete_dialog : ConfirmationDialog

func filter_for_tag(tag : StringName) -> void:
	for child in game_select_save_list.get_children():
		if tag == &"":
			child.show()
		else:
			if tag == child.resource.session_tag:
				child.show()
			else:
				child.hide()

func _build_date_list() -> void:
	var start_date := lobby_panel_button.instantiate()
	start_date.set_text(&"1836")
	start_date.pressed.connect(_on_start_date_panel_button_pressed.bind(start_date))
	game_select_start_date.add_child(start_date)
	start_date = lobby_panel_button.instantiate()
	start_date.set_text(&"1863")
	start_date.pressed.connect(_on_start_date_panel_button_pressed.bind(start_date))
	game_select_start_date.add_child(start_date)

var _id_to_tag : Array[StringName] = []

# Requirements
# * FS-8
func _build_save_list() -> void:
	game_select_save_tab.add_tab("GAMELOBBY_SELECT_ALL")
	for save_name in SaveManager._save_dictionary:
		var save : SaveResource = SaveManager._save_dictionary[save_name]
		var save_node := _create_save_node(save)
		game_select_save_list.add_child(save_node)
		if not _id_to_tag.has(save.session_tag):
			_id_to_tag.append(save.session_tag)
			game_select_save_tab.add_tab(save.session_tag)

func _create_save_node(resource : SaveResource) -> Control:
	var save_node = save_scene.instantiate()
	save_node.resource = resource
	save_node.pressed.connect(_on_save_node_pressed.bind(save_node))
	save_node.request_to_delete.connect(_on_save_node_delete_requested.bind(save_node))
	return save_node

func _queue_clear_lists() -> void:
	var full_list = game_select_start_date.get_children()
	full_list.append_array(game_select_save_list.get_children())
	for child in full_list:
		child.queue_free()
	game_select_save_tab.clear_tabs()
	_id_to_tag.clear()

# REQUIREMENTS:
# * SS-16
# * UIFUN-40
func _on_back_button_button_down():
	print("Returning to Main Menu.")
	SaveManager.current_session_tag = ""
	SaveManager.current_save = null
	back_button_pressed.emit()

# REQUIREMENTS:
# * SS-21
func _on_start_button_pressed():
	print("Starting new game.")
	if SaveManager.current_session_tag == "":
		# TODO: Get country tag as well
		var datetime := Time.get_datetime_dict_from_system()
		SaveManager.current_session_tag = "%s/%s/%s-%s:%s:%s" % [
			datetime["day"],
			datetime["month"],
			datetime["year"],
			datetime["hour"],
			datetime["minute"],
			datetime["second"]
		]
	if SaveManager.current_save == null and SaveManager.current_session_tag in _id_to_tag:
		session_tag_dialog.dialog_text = tr("GAMELOBBY_SESSIONTAG_DIALOG_TEXT").format({ "session_tag": SaveManager.current_session_tag })
		session_tag_dialog.title = tr("GAMELOBBY_SESSIONTAG_DIALOG_TITLE").format({ "session_tag": SaveManager.current_session_tag })
		session_tag_dialog.popup_centered()
	else:
		_on_session_tag_dialog_confirmed()

# REQUIREMENTS:
# * SS-19
func _on_game_select_list_item_selected(index):
	print("Selected save game: ", index)
	save_game_selected.emit(index)

# If the date is double-clicked, start the game!
func _on_game_select_list_item_activated(index):
	_on_game_select_list_item_selected(index)
	_on_start_button_pressed()

func _on_session_tag_edit_text_submitted(new_text):
	SaveManager.current_session_tag = new_text
	_on_start_button_pressed()

func _on_session_tag_dialog_confirmed():
	get_tree().change_scene_to_file("res://src/Game/GameSession/GameSession.tscn")

var _requested_node_to_delete : Control
func _on_save_node_delete_requested(node : Control) -> void:
	_requested_node_to_delete = node
	delete_dialog.dialog_text = tr("GAMELOBBY_DELETE_DIALOG_TEXT").format({ "file_name": _requested_node_to_delete.resource.save_name })
	delete_dialog.title = tr("GAMELOBBY_DELETE_DIALOG_TITLE").format({ "file_name": _requested_node_to_delete.resource.save_name })
	delete_dialog.popup_centered()

var _start_date_index := -1
func _on_start_date_panel_button_pressed(node : Control) -> void:
	if node is LobbyPanelButton and node.get_index(true) == _start_date_index:
		_on_start_button_pressed()
		return
	_start_date_index = node.get_index(true)
	start_button.disabled = false
	start_date_selected.emit(_start_date_index)

func _on_save_node_pressed(node : Control) -> void:
	if SaveManager.current_save != null and SaveManager.current_save == node.resource:
		SaveManager.current_session_tag = SaveManager.current_save.session_tag
		_on_start_button_pressed()
		return
	SaveManager.current_save = node.resource
	if SaveManager.current_save != null:
		session_tag_line_edit.text = SaveManager.current_save.session_tag
	else:
		session_tag_line_edit.text = ""
	start_button.disabled = false
	save_game_selected.emit(SaveManager.current_save)

func _on_game_select_save_tab_tab_changed(tab) -> void:
	if tab == 0:
		filter_for_tag(&"")
	else:
		filter_for_tag(_id_to_tag[tab - 1])

func _on_delete_dialog_confirmed():
	_requested_node_to_delete.resource.delete()
	_requested_node_to_delete.queue_free()

func _on_visibility_changed():
	if visible:
		_build_date_list()
		_build_save_list()
	else:
		_queue_clear_lists()
