extends Control

# REQUIREMENTS
# * SS-13

signal back_button_pressed

@export var _tab_container : TabContainer

@onready
var _settings := GameSettings.load_from_file("user://settings.cfg")

func _ready() -> void:
	_tab_container.set_tab_title(0, "OPTIONS_GENERAL")
	_tab_container.set_tab_title(1, "OPTIONS_VIDEO")
	_tab_container.set_tab_title(2, "OPTIONS_SOUND")
	_tab_container.set_tab_title(3, "OPTIONS_CONTROLS")
	_tab_container.set_tab_title(4, "OPTIONS_OTHER")

	# Prepare options menu before loading user settings
	var tab_bar : TabBar = _tab_container.get_child(0, true)

	# This ends up easier to manage then trying to manually recreate the TabContainer's behavior
	# These buttons can be accessed regardless of the tab
	var button_list := HBoxContainer.new()
	button_list.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	button_list.alignment = BoxContainer.ALIGNMENT_END
	tab_bar.add_child(button_list)

	# REQUIREMENTS
	# * UI-12
	# * UIFUN-14
	var reset_button := Button.new()
	reset_button.text = "OPTIONS_RESET"
	reset_button.pressed.connect(_settings.reset_settings)
	button_list.add_child(reset_button)

	# REQUIREMENTS
	# * UI-11
	# * UIFUN-17
	var back_button := Button.new()
	back_button.text = "OPTIONS_BACK"
	back_button.pressed.connect(_on_back_button_pressed)
	button_list.add_child(back_button)
	_save_overrides.call_deferred()

	_settings.changed.emit()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_CRASH, NOTIFICATION_WM_CLOSE_REQUEST:
			_on_window_close_requested()

func _input(event : InputEvent) -> void:
	if self.is_visible_in_tree():
		if event.is_action_pressed("ui_cancel"):
			_on_back_button_pressed()

func _on_back_button_pressed() -> void:
	back_button_pressed.emit()
	_settings.save()
	_save_overrides()

func _on_window_close_requested() -> void:
	_settings.save()
	_save_overrides()

func _save_overrides() -> void:
	var override_path : String = ProjectSettings.get_setting_with_override("application/config/project_settings_override")
	if override_path.is_empty():
		return
	var override_settings := GameSettings.load_from_file(override_path)
	override_settings.set_block_signals(true)
	if FileAccess.file_exists(override_path):
		if override_settings.load(override_path) != OK:
			push_error("Failed to load overrides from %s" % override_path)
	override_settings.set_value("display", "window/size/mode", Resolution.get_current_window_mode())
	var resolution : Vector2i = Resolution.get_current_resolution()
	override_settings.set_value("display", "window/size/viewport_width", resolution.x)
	override_settings.set_value("display", "window/size/viewport_height", resolution.y)
	if override_settings.save(override_path) != OK:
		push_error("Failed to save overrides to %s" % override_path)
	override_settings.set_block_signals(false)
