extends HSlider
class_name SettingHSlider

@export
var section_name : String = "setting"

@export
var setting_name : String = "setting_hslider"

@export
var default_value : float = 0

var settings_path := "user://settings.cfg"

func _enter_tree() -> void:
	var settings := GameSettings.load_from_file(settings_path)
	settings.changed.connect(load_setting.bind(settings))
	value_changed.connect(set_setting.bind(settings))

func load_setting(menu : GameSettings) -> void:
	var load_value : Variant = menu.get_value(section_name, setting_name, default_value)
	match typeof(load_value):
		TYPE_FLOAT, TYPE_INT:
			if value == load_value:
				value_changed.emit(value)
			value = load_value
			return
		TYPE_STRING, TYPE_STRING_NAME:
			var load_string := load_value as String
			if load_string.is_valid_float():
				load_value = load_string.to_float()
				if value == load_value: value_changed.emit(value)
				value = load_value
				return
	push_error("Setting value '%s' invalid for setting [%s] \"%s\"" % [load_value, section_name, setting_name])
	value = default_value

func set_setting(val : float, menu : GameSettings) -> void:
	menu.set_value(section_name, setting_name, val)

func reset_setting() -> void:
	value = default_value
