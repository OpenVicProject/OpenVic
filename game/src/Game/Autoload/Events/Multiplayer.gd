class_name MultiplayerEventsObject
extends RefCounted

signal save_ips(save_file: ConfigFile)
signal load_ips(load_file: ConfigFile)

const ips_file_path_setting : String = "openvic/settings/ips_file_path"
const ips_file_path_default : String = "user://ips.cfg"

var _saved_ips_file_path : String = ProjectSettings.get_setting(ips_file_path_setting, ips_file_path_default)
var _saved_ips_file := ConfigFile.new()

var config_file_loaded : bool = false

const USER : StringName = &"USER"
const SERVER_NAMES : StringName = &"SERVER_NAMES"
const SERVER_IPS : StringName = &"SERVER_IPS"
const PLAYER_NAME : StringName = &"player_name"
const HOST_GAME_NAME : StringName = &"host_game_name"
const HOST_GAME_PASSWORD : StringName = &"host_game_password"

const DEFAULT_PLAYER_NAME : StringName = &"Player1"
const DEFAULT_GAME_NAME : StringName = &"Player1's Game"
const DEFAULT_GAME_PASSWORD : StringName = &""

func get_override_path() -> String:
	var override_path : String = ProjectSettings.get_setting("application/config/project_settings_override", "")
	if override_path.is_empty():
		override_path = _saved_ips_file_path
	return override_path

func get_ips_config_file() -> ConfigFile:
	if config_file_loaded:
		return _saved_ips_file
	else:
		return load_ips_config_file()

func load_ips_config_file() -> ConfigFile:
	var override_path := get_override_path()
	if FileAccess.file_exists(override_path):
		if _saved_ips_file.load(override_path) != OK:
			push_error("Failed to load overrides from %s" % override_path)
		else:
			if !_saved_ips_file.has_section_key(USER,PLAYER_NAME):
				_saved_ips_file.set_value(USER,PLAYER_NAME,DEFAULT_PLAYER_NAME)
			if !_saved_ips_file.has_section_key(USER,HOST_GAME_NAME):
				_saved_ips_file.set_value(USER,HOST_GAME_NAME,DEFAULT_GAME_NAME)
			if !_saved_ips_file.has_section_key(USER,HOST_GAME_PASSWORD):
				_saved_ips_file.set_value(USER,HOST_GAME_PASSWORD,DEFAULT_GAME_PASSWORD)
			save_ips_config_file()
			load_ips.emit(_saved_ips_file)
			config_file_loaded = true
	else:
		_saved_ips_file.set_value(USER,PLAYER_NAME,DEFAULT_PLAYER_NAME)
		_saved_ips_file.set_value(USER,HOST_GAME_NAME,DEFAULT_GAME_NAME)
		_saved_ips_file.set_value(USER,HOST_GAME_PASSWORD,DEFAULT_GAME_PASSWORD)
		save_ips_config_file()
		
	return _saved_ips_file

func save_ips_config_file() -> void:
	var override_path := get_override_path()
	_saved_ips_file.save(_saved_ips_file_path)
	if _saved_ips_file.save(override_path) != OK:
		push_error("Failed to save ip overrides to %s" % override_path)
	else:
		save_ips.emit(_saved_ips_file)

func _init() -> void:
	load_ips_config_file()
