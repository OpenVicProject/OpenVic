extends OptionButton
class_name SettingOptionButton

@export
var section_name : String = "setting"

@export
var setting_name : String = "setting_optionbutton"

@export
var default_selected : int = -1:
	get: return default_selected
	set(v):
		if v < 0 or item_count == 0:
			default_selected = -1
			return
		default_selected = v % item_count

func _valid_index(index : int) -> bool:
	return 0 <= index and index < item_count

func _get_value_for_file(select_value : int):
	if _valid_index(select_value):
		return select_value
	else:
		return null

func _set_value_from_file(load_value) -> void:
	match typeof(load_value):
		TYPE_INT:
			if _valid_index(load_value):
				selected = load_value
				return
		TYPE_STRING, TYPE_STRING_NAME:
			var load_string := load_value as String
			if load_string.is_valid_int():
				var load_int := load_string.to_int()
				if _valid_index(load_int):
					selected = load_int
					return
			for item_index in item_count:
				if load_string == get_item_text(item_index):
					selected = item_index
					return
	push_error("Setting value '%s' invalid for setting [%s] \"%s\"" % [load_value, section_name, setting_name])
	selected = default_selected

func _setup_button() -> void:
	pass

func _ready():
	Events.Options.load_settings.connect(load_setting)
	Events.Options.save_settings.connect(save_setting)
	Events.Options.reset_settings.connect(reset_setting)
	_setup_button()
	if not _valid_index(default_selected) or selected == -1:
		OS.alert("Failed to generate %s %s options." % [setting_name, section_name], "%s Options Error" % section_name)
		get_tree().quit()

func load_setting(file : ConfigFile) -> void:
	if file == null: return
	_set_value_from_file(file.get_value(section_name, setting_name, _get_value_for_file(default_selected)))
	item_selected.emit(selected)

func save_setting(file : ConfigFile) -> void:
	if file == null: return
	file.set_value(section_name, setting_name, _get_value_for_file(selected))

func reset_setting() -> void:
	selected = default_selected
	item_selected.emit(selected)
