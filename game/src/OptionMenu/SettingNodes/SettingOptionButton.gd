extends OptionButton
class_name SettingOptionButton

@export
var section_name : String = "Setting"

@export
var setting_name : String = "SettingOptionMenu"

@export
var default_value : int = 0

func load_setting(file : ConfigFile):
	selected = file.get_value(section_name, setting_name, default_value)
	item_selected.emit(selected)

func save_setting(file : ConfigFile):
	file.set_value(section_name, setting_name, selected)

func reset_setting():
	selected = default_value
	item_selected.emit(selected)
