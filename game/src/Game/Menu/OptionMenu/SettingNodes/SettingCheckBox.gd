extends CheckBox
class_name SettingCheckBox

signal option_selected(pressed : bool, by_user : bool)

@export
var section_name : String = "setting"

@export
var setting_name : String = "setting_checkbox"

@export
var default_pressed : bool = true

func _setup_button() -> void:
	pass

func _ready() -> void:
	Events.Options.load_settings.connect(load_setting)
	Events.Options.save_settings.connect(save_setting)
	Events.Options.reset_settings.connect(reset_setting)
	toggled.connect(func(p : bool) -> void: option_selected.emit(p, true))
	_setup_button()

func _set_value_from_file(load_value : Variant) -> void:
	match typeof(load_value):
		TYPE_BOOL, TYPE_INT:
			set_pressed_no_signal(load_value as bool)
			return
		TYPE_STRING, TYPE_STRING_NAME:
			var load_str := (load_value as String).to_lower()
			if load_str.is_empty() or load_str.begins_with("f") or load_str.begins_with("n"):
				set_pressed_no_signal(false)
				return
			if load_str.begins_with("t") or load_str.begins_with("y"):
				set_pressed_no_signal(true)
				return
	push_error("Setting value '%s' invalid for setting [%s] \"%s\"" % [load_value, section_name, setting_name])
	set_pressed_no_signal(default_pressed)

func load_setting(file : ConfigFile) -> void:
	if file == null: return
	_set_value_from_file(file.get_value(section_name, setting_name, default_pressed))
	option_selected.emit(button_pressed, false)

func save_setting(file : ConfigFile) -> void:
	if file == null: return
	file.set_value(section_name, setting_name, button_pressed)

func reset_setting(no_emit : bool = false) -> void:
	set_pressed_no_signal(default_pressed)
	if not no_emit:
		option_selected.emit(button_pressed, false)
