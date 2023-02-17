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
	value = file.get_value(section_name, setting_name, default_value)

func save_setting(file : ConfigFile):
	file.set_value(section_name, setting_name, value)

func reset_setting():
	value = default_value
