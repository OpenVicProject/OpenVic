extends Control

signal back_button_pressed

func _ready():
	# Prepare options menu before loading user settings
	var tab_bar : TabBar = $Margin/Tab.get_child(0, true)

	# This ends up easier to manage then trying to manually recreate the TabContainer's behavior
	# These buttons can be accessed regardless of the tab
	var button_list := HBoxContainer.new()
	button_list.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	button_list.alignment = BoxContainer.ALIGNMENT_END
	tab_bar.add_child(button_list)

	var reset_button := Button.new()
	reset_button.text = "R"
	reset_button.pressed.connect(Events.Options.try_reset_settings)
	button_list.add_child(reset_button)

	var back_button := Button.new()
	back_button.text = "X"
	back_button.pressed.connect(_on_back_button_pressed)
	button_list.add_child(back_button)

	get_viewport().get_window().close_requested.connect(_on_window_close_requested)
	_save_overrides.call_deferred()
	Events.Options.save_settings.connect(func(_f): self._save_overrides.call_deferred())

func _notification(what):
	match what:
		NOTIFICATION_CRASH:
			_on_window_close_requested()

# Could pass the LocaleButton between the MainMenu and OptionsMenu
# but that seems a bit excessive
func toggle_locale_button_visibility(locale_visible : bool):
	$LocaleVBox/LocaleHBox/LocaleButton.visible = locale_visible

func _on_back_button_pressed():
	Events.Options.save_settings_from_file()
	back_button_pressed.emit()

func _on_window_close_requested() -> void:
	if visible:
		Events.Options.save_settings_from_file()

func _save_overrides() -> void:
	var file := ConfigFile.new()
	var override_path := ProjectSettings.get_setting("application/config/project_settings_override") as String
	file.load(override_path)
	file.set_value("display", "window/size/mode", get_viewport().get_window().mode)
	var resolution : Vector2i = Resolution.get_current_resolution()
	file.set_value("display", "window/size/viewport_width", resolution.x)
	file.set_value("display", "window/size/viewport_height", resolution.y)
	file.save(override_path)
