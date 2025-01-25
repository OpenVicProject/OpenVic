extends HBoxContainer
class_name HostTab

@export var initial_focus: Control

@export var player_name_entry : LineEdit
@export var game_name_entry : LineEdit
@export var password_entry : SecretEdit
@export var is_lan : CheckButton

signal host(player_name : String, game_name : String, password : String, lan_only : bool)
signal player_name_changed(player_name : String)

func _notification(what : int) -> void:
	match(what):
		NOTIFICATION_VISIBILITY_CHANGED:
			if visible and is_inside_tree():
				initial_focus.grab_focus()

func set_user_values(player_name : StringName, game_name : StringName, game_password : StringName) -> void:
	player_name_entry.text = player_name
	game_name_entry.text = game_name
	password_entry.set_text(game_password)

func _on_host_pressed() -> void:
	var player_name := get_player_name()
	var game_name := game_name_entry.text
	host.emit(player_name,game_name,password_entry.get_text(), is_lan.button_pressed)

func get_player_name() -> StringName:
	var player_name := player_name_entry.text
	if player_name.is_empty():
		player_name = player_name_entry.placeholder_text
	return player_name

func set_player_name(player_name : String) -> void:
	player_name_entry.text = player_name

func set_game_name(game_name : String) -> void:
	game_name_entry.text = game_name

func set_game_password(game_password : String) -> void:
	password_entry.set_text(game_password)

func get_game_name() -> StringName:
	var game_name := game_name_entry.text
	if game_name.is_empty():
		game_name = game_name_entry.placeholder_text
	return game_name

func get_password() -> StringName:
	return password_entry.get_text()

func _on_player_name_entry_text_changed(new_text: String) -> void:
	player_name_changed.emit(new_text)
