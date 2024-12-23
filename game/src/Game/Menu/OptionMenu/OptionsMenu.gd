extends Control

# REQUIREMENTS
# * SS-13

signal back_button_pressed

@export var _tab_container: TabContainer


func _ready() -> void:
	_tab_container.set_tab_title(0, "OPTIONS_GENERAL")
	_tab_container.set_tab_title(1, "OPTIONS_VIDEO")
	_tab_container.set_tab_title(2, "OPTIONS_SOUND")
	_tab_container.set_tab_title(3, "OPTIONS_CONTROLS")
	_tab_container.set_tab_title(4, "OPTIONS_OTHER")

	# Setup Keychain for Hotkeys
	for action: StringName in InputMap.get_actions():
		if not Keychain.keep_binding_check.call(action):
			continue
		Keychain.actions[action] = Keychain.InputAction.new(
			action.erase(0, "button_".length()).left(-"_hotkey".length()).capitalize(), "Hotkeys"
		)
		var display_name: String = Keychain.actions[action].display_name
		if display_name.begins_with("Mapmode"):
			var mapmode_index := display_name.replace("Mapmode", "").to_int()
			display_name = tr(GameSingleton.get_mapmode_localisation_key(mapmode_index))
			if mapmode_index <= 10:
				display_name = (
					display_name
					. replace(" Mapmode", "")
					. replace("Mode de carte ", "")
					. replace("-Kartenmodus", "")
					. replace("Modo de mapa de ", "")
					. replace("Modo mapa de ", "")
					. replace("Modo mapa ", "")
				)
			display_name = tr("Mapmode %s") % display_name.capitalize()
			Keychain.actions[action].display_name = display_name
		Keychain.profiles[0].bindings[action] = InputMap.action_get_events(action)

	# Prepare options menu before loading user settings
	var tab_bar: TabBar = _tab_container.get_child(0, true)

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
	reset_button.pressed.connect(Events.Options.try_reset_settings)
	button_list.add_child(reset_button)

	# REQUIREMENTS
	# * UI-11
	# * UIFUN-17
	var back_button := Button.new()
	back_button.text = "OPTIONS_BACK"
	back_button.pressed.connect(_on_back_button_pressed)
	button_list.add_child(back_button)
	get_viewport().get_window().close_requested.connect(_on_window_close_requested)
	_save_overrides.call_deferred()
	Events.Options.save_settings.connect(
		func(_f: ConfigFile) -> void: self._save_overrides.call_deferred()
	)


func _notification(what: int) -> void:
	match what:
		NOTIFICATION_CRASH:
			_on_window_close_requested()


func _input(event: InputEvent) -> void:
	if self.is_visible_in_tree():
		if event.is_action_pressed("ui_cancel"):
			_on_back_button_pressed()


func _on_back_button_pressed() -> void:
	Events.Options.save_settings_to_file()
	back_button_pressed.emit()


func _on_window_close_requested() -> void:
	if visible:
		Events.Options.save_settings_to_file()


func _save_overrides() -> void:
	var override_path: String = ProjectSettings.get_setting(
		"application/config/project_settings_override", ""
	)
	if override_path.is_empty():
		override_path = ProjectSettings.get_setting(
			Events.Options.settings_file_path_setting, Events.Options.settings_file_path_default
		)
	var file := ConfigFile.new()
	if FileAccess.file_exists(override_path):
		if file.load(override_path) != OK:
			push_error("Failed to load overrides from %s" % override_path)
	file.set_value("display", "window/size/mode", Resolution.get_current_window_mode())
	var resolution: Vector2i = Resolution.get_current_resolution()
	file.set_value("display", "window/size/viewport_width", resolution.x)
	file.set_value("display", "window/size/viewport_height", resolution.y)
	if file.save(override_path) != OK:
		push_error("Failed to save overrides to %s" % override_path)
