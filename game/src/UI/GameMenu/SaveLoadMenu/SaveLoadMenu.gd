extends Control

@export var _save_scene : PackedScene

@export_group("Nodes")
@export var _label : Label
@export var _scroll_list : BoxContainer
@export var _save_line_edit : LineEdit
@export var _save_load_button : Button
@export var _tag_selection_tab : TabBar
@export var _delete_dialog : ConfirmationDialog
@export var _overwrite_dialog : ConfirmationDialog

var is_save_menu : bool = true
var _id_to_tag : Array[StringName] = []

func filter_for_tag(tag : StringName) -> void:
	for child : Control in _scroll_list.get_children():
		if tag.is_empty():
			child.show()
		else:
			if tag == child.resource.session_tag:
				child.show()
			else:
				child.hide()

# Requirements
# * UIFUN-78
func show_for_load() -> void:
	_label.text = "SAVELOADMENU_LOAD_TITLE"
	_save_load_button.text = "SAVELOADMENU_LOAD_BUTTON"
	_save_line_edit.editable = false
	is_save_menu = false
	show()

# Requirements
# * UIFUN-77
func show_for_save() -> void:
	_label.text = "SAVELOADMENU_SAVE_TITLE"
	_save_load_button.text = "SAVELOADMENU_SAVE_BUTTON"
	_save_line_edit.editable = true
	is_save_menu = true
	show()

func _build_save_list() -> void:
	_tag_selection_tab.add_tab("SAVELOADMENU_TABSELECTIONTABBAR_ALL")
	for save_name : StringName in SaveManager._save_dictionary:
		var save : SaveResource = SaveManager._save_dictionary[save_name]
		var save_node := _create_save_node(save)
		_scroll_list.add_child(save_node)
		if not _id_to_tag.has(save.session_tag):
			_id_to_tag.append(save.session_tag)
			_tag_selection_tab.add_tab(save.session_tag)

func _create_save_node(resource : SaveResource) -> Control:
	var save_node = _save_scene.instantiate()
	save_node.resource = resource
	save_node.pressed.connect(_on_save_node_pressed.bind(save_node))
	save_node.request_to_delete.connect(_on_save_node_delete_requested.bind(save_node))
	return save_node

func _queue_clear_scroll_list() -> void:
	for child : Node in _scroll_list.get_children():
		child.queue_free()
	_tag_selection_tab.clear_tabs()
	_id_to_tag.clear()

# REQUIREMENTS:
# * UIFUN-84
# * UIFUN-89
func _on_close_button_pressed() -> void:
	hide()

func _on_delete_dialog_confirmed() -> void:
	_requested_node_to_delete.resource.delete()
	_requested_node_to_delete.queue_free()

# REQUIREMENTS:
# * UIFUN-83
func _on_overwrite_dialog_confirmed() -> void:
	SaveManager.add_or_replace_save(SaveManager.make_new_save(_submitted_text))
	_on_close_button_pressed()

var _submitted_text : String = ""
func _on_save_line_edit_text_submitted(new_text) -> void:
	_submitted_text = new_text
	if SaveManager.has_save(new_text):
		_overwrite_dialog.dialog_text = tr("SAVELOADMENU_OVERWRITE_DIALOG_TEXT").format({ "file_name": _submitted_text })
		_overwrite_dialog.title = tr("SAVELOADMENU_OVERWRITE_DIALOG_TITLE").format({ "file_name": _submitted_text })
		_overwrite_dialog.popup_centered()
		return
	_on_overwrite_dialog_confirmed()

func _on_save_load_button_pressed() -> void:
	if is_save_menu:
		_save_line_edit.text_submitted.emit(_save_line_edit.text)

var _requested_node_to_delete : Control
func _on_save_node_delete_requested(node : Control) -> void:
	_requested_node_to_delete = node
	_delete_dialog.dialog_text = tr("SAVELOADMENU_DELETE_DIALOG_TEXT").format({ "file_name": _requested_node_to_delete.resource.save_name })
	_delete_dialog.title = tr("SAVELOADMENU_DELETE_DIALOG_TITLE").format({ "file_name": _requested_node_to_delete.resource.save_name })
	_delete_dialog.popup_centered()

# REQUIREMENTS:
# * UIFUN-81
# * UIFUN-86
func _on_save_node_pressed(node : Control) -> void:
	if is_save_menu:
		_save_line_edit.text = node.resource.save_name

func _on_tag_selection_tab_bar_tab_changed(tab) -> void:
	if tab == 0:
		filter_for_tag(&"")
	else:
		filter_for_tag(_id_to_tag[tab - 1])

func _on_visibility_changed() -> void:
	if visible:
		_build_save_list()
	else:
		_queue_clear_scroll_list()
		SaveManager.flush_save()
