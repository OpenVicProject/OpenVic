extends CheckBox
class_name SettingCheckBox

signal option_selected(pressed : bool, by_user : bool)

@export
var section_name : String = "setting"

@export
var setting_name : String = "setting_checkbox"

@export
var default_pressed : bool = true

var settings_path := "user://settings.cfg"

func _setup_button() -> void:
	pass

func _enter_tree() -> void:
	var settings := GameSettings.load_from_file(settings_path)
	settings.changed.connect(load_setting.bind(settings))
	toggled.connect(set_setting.bind(settings))

func _ready() -> void:
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

func load_setting(menu : GameSettings) -> void:
	_set_value_from_file(menu.get_value(section_name, setting_name, default_pressed))
	option_selected.emit(button_pressed, false)

func set_setting(is_toggled : bool, menu : GameSettings) -> void:
	menu.set_value(section_name, setting_name, is_toggled)
