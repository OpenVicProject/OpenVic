extends HBoxContainer
class_name DirectConnectionTab

signal connect(ip : String, player_name : String)
signal save_ips(names : PackedStringArray, ips : PackedStringArray)
signal revert_ips()
signal player_name_changed(player_name : String)

@export var initial_focus: Control
@export var ip_edit : SecretEdit
@export var player_name_edit : LineEdit
@export var game_name_edit : LineEdit
@export var IP_grid : VBoxContainer

var connection_entry : PackedScene = preload("res://src/UI/GameMenu/MultiplayerMenu/DirectConnectionEntry.tscn")

func _notification(what : int) -> void:
	match(what):
		NOTIFICATION_VISIBILITY_CHANGED:
			if visible and is_inside_tree():
				initial_focus.grab_focus()

func _on_connect_pressed() -> void:
	var player_name : StringName = player_name_edit.text
	if player_name.is_empty():
		player_name = player_name_edit.placeholder_text
	connect.emit(ip_edit.get_text(),player_name)

func _on_connect_pressed_custom_ip(ip : String) -> void:
	var player_name : StringName = player_name_edit.text
	if player_name.is_empty():
		player_name = player_name_edit.placeholder_text
	connect.emit(ip,player_name)

func initial_setup(player_name : StringName) -> void:
	player_name_edit.text = player_name

func new_ip_entry() -> void:
	var game_name : String = game_name_edit.text
	if game_name.is_empty():
		game_name = game_name_edit.placeholder_text
	add_ip_entry_to_list(game_name,ip_edit.get_text())

func add_ip_entry_to_list(connection_name : String, ip : String) -> void:
	var entry : DirectConnectionEntry = connection_entry.instantiate()
	IP_grid.add_child(entry)
	
	entry.name_edit.text = connection_name
	entry.ip_edit.set_text(ip)
	
	entry.connect_to_ip.connect(_on_connect_pressed_custom_ip)
	entry.delete.connect(delete_entry.bind(entry))

func delete_entry(entry : DirectConnectionEntry) -> void:
	entry.queue_free()

func set_player_name(player_name : String) -> void:
	player_name_edit.text = player_name

func _on_save_ips_pressed() -> void:
	var names : PackedStringArray = []
	var ips : PackedStringArray = []
	for entry : DirectConnectionEntry in IP_grid.get_children():
		names.push_back(entry.name_edit.text)
		ips.push_back(entry.ip_edit.get_text())
	save_ips.emit(names,ips)

func _on_revert_ips_pressed() -> void:
	for child : DirectConnectionEntry in IP_grid.get_children():
		child.queue_free()
	revert_ips.emit()

func _on_player_name_changed(new_text : String) -> void:
	player_name_changed.emit(new_text)
