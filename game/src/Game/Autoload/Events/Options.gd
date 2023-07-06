class_name OptionsEventsObject
extends RefCounted

signal save_settings(save_file: ConfigFile)
signal load_settings(load_file: ConfigFile)
signal reset_settings()

func load_settings_from_file() -> void:
	load_settings.emit(_settings_file)

# REQUIREMENTS
# * SS-11
func save_settings_to_file() -> void:
	save_settings.emit(_settings_file)
	_settings_file.save(_settings_file_path)

func try_reset_settings() -> void:
	reset_settings.emit()

const settings_file_path_setting : String = "openvic/settings/settings_file_path"
const settings_file_path_default : String = "user://settings.cfg"

var _settings_file_path : String = ProjectSettings.get_setting(settings_file_path_setting, settings_file_path_default)
var _settings_file := ConfigFile.new()

# REQUIREMENTS
# * SS-9
# * UIFUN-7
func _init():
	if FileAccess.file_exists(_settings_file_path):
		_settings_file.load(_settings_file_path)
