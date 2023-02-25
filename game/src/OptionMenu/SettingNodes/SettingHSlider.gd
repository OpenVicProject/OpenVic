extends HSlider
class_name SettingHSlider

@export
var section_name : String = "Setting"

@export
var setting_name : String = "SettingHSlider"

@export
var default_value : float = 0

func _ready():
	Events.Options.load_settings.connect(load_setting)
	Events.Options.save_settings.connect(save_setting)
	Events.Options.reset_settings.connect(reset_setting)

func load_setting(file : ConfigFile):
	if file == null: return
	var load_value = file.get_value(section_name, setting_name, default_value)
	match typeof(load_value):
		TYPE_FLOAT, TYPE_INT:
			value = load_value
			return
		TYPE_STRING, TYPE_STRING_NAME:
			var load_string := load_value as String
			if load_string.is_valid_float():
				value = load_string.to_float()
				return
	push_error("Setting value '%s' invalid for setting [%s] \"%s\"" % [load_value, section_name, setting_name])
	value = default_value

func save_setting(file : ConfigFile):
	if file == null: return
	file.set_value(section_name, setting_name, value)

func reset_setting():
	value = default_value
