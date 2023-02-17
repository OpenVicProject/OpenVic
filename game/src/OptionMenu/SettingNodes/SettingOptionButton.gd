extends OptionButton
class_name SettingOptionButton

@export
var section_name : String = "Setting"

@export
var setting_name : String = "SettingOptionMenu"

@export
var default_selected : int = -1:
	get: return default_selected
	set(v):
		if v == -1:
			default_selected = -1
			return
		default_selected = v % item_count

@export
var add_nonstandard_value := false

func _get_value_for_file(select_value : int):
	if select_value > -1:
		return get_item_text(select_value)
	else:
		return null

func _set_value_from_file(load_value) -> void:
	selected = -1
	for item_index in range(item_count):
		if load_value == get_item_text(item_index):
			selected = item_index
	if selected == -1:
		if add_nonstandard_value:
			add_item(load_value)
			selected = item_count - 1
		else: push_error("Setting value '%s' invalid for setting [%s] \"%s\"" % [load_value, section_name, setting_name])

func _setup_button() -> void:
	pass

func _ready():
	Events.Options.load_settings.connect(load_setting)
	Events.Options.save_settings.connect(save_setting)
	Events.Options.reset_settings.connect(reset_setting)
	_setup_button()

func load_setting(file : ConfigFile) -> void:
	_set_value_from_file(file.get_value(section_name, setting_name, _get_value_for_file(default_selected)))
	item_selected.emit(selected)

func save_setting(file : ConfigFile) -> void:
	file.set_value(section_name, setting_name, _get_value_for_file(selected))

func reset_setting() -> void:
	selected = default_selected
	item_selected.emit(selected)
