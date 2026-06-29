extends Control

const AppSettings := preload("res://addons/kenyoni/app_settings/app_settings.gd")

# REQUIREMENTS
# * SS-13

signal back_button_pressed

@export var _tab_container : TabContainer
@export var _settings_container: PackedScene

func _ready() -> void:
	GameSettings.settings_applied.connect(_on_settings_applied)

	# Prepare options menu before loading user settings
	var tab_bar : TabBar = _tab_container.get_tab_bar()

	# This ends up easier to manage then trying to manually recreate the TabContainer's behavior
	# These buttons can be accessed regardless of the tab
	var button_list := HBoxContainer.new()
	button_list.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	button_list.alignment = BoxContainer.ALIGNMENT_END
	tab_bar.add_child(button_list)

	# REQUIREMENTS
	# * UI-11
	# * UIFUN-17
	var back_button := Button.new()
	back_button.text = "OPTIONS_BACK"
	back_button.shortcut_feedback = false
	back_button.pressed.connect(_on_back_button_pressed)
	button_list.add_child(back_button)

	var cancel_action := InputEventAction.new()
	cancel_action.action = &"ui_cancel"
	cancel_action.pressed = true
	back_button.shortcut = Shortcut.new()
	back_button.shortcut.events = [cancel_action]

	_setup_settings()

func _setup_settings() -> void:
	_iterate_settings_sections(GameSettings)
	_iterate_settings_sections(ModSettings)
	_iterate_settings_sections(Vic2Settings)

	_tab_container.move_child(_tab_container.get_child(0), 3)
	_tab_container.set_tab_title(2, "OPTIONS_SOUND")
	_tab_container.set_tab_title(3, "OPTIONS_CONTROLS")
	_tab_container.current_tab = 0

func _iterate_settings_sections(app_setting: AppSettings) -> void:
	var tab_count := _tab_container.get_tab_count() - 1

	var sections: PackedStringArray = app_setting.get_sub_sections()
	for idx: int in range(sections.size()):
		var section: String = sections[idx]
		_setup_settings_section(app_setting, tab_count + idx, section)

func _setup_settings_section(app_setting: AppSettings, section_index: int, section: String) -> void:
	var all_internal := true
	for setting: AppSettings.Setting in app_setting.get_section(section):
		if setting.is_internal(): continue
		all_internal = false
		break
	if all_internal: return

	var container: SettingsContainer = _settings_container.instantiate()
	container.name = section.capitalize().validate_node_name()
	container.section_key = section
	_tab_container.add_child(container)
	_tab_container.set_tab_title(section_index + 1, "OPTIONS_" + section.to_upper())

	# all settings in section without a sub section
	for setting: AppSettings.Setting in app_setting.get_section(section, 1):
		container.add_setting(setting)
	# add all sub sections and their settings
	for sub_section: String in app_setting.get_sub_sections(section):
		container.add_section(sub_section.capitalize())
		for setting: AppSettings.Setting in app_setting.get_section(section.path_join(sub_section)):
			container.add_setting(setting)

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_CRASH, NOTIFICATION_WM_CLOSE_REQUEST:
			_on_window_close_requested()

func _on_back_button_pressed() -> void:
	back_button_pressed.emit()
	GameSettings.apply_staged_values()

func _on_window_close_requested() -> void:
	GameSettings.apply_staged_values()

func _on_settings_applied() -> void:
	GameSettings.save()
