extends "res://addons/keychain/ShortcutEdit.gd"

func _add_hotkey(action : StringName, events : Array[InputEvent]) -> void:
	Keychain.actions[action] = Keychain.InputAction.new(
			action.erase(0, "button_".length()).left(-"_hotkey".length()).capitalize(),
		"Hotkeys")
	var display_name : String = Keychain.actions[action].display_name
	if display_name.begins_with("Mapmode"):
		var mapmode_index := display_name.replace("Mapmode", "").to_int()
		display_name = tr(GameSingleton.get_mapmode_localisation_key(mapmode_index))
		if mapmode_index <= 10:
			display_name = display_name\
				.replace(" Mapmode", "")\
				.replace("Mode de carte ", "")\
				.replace("-Kartenmodus", "")\
				.replace("Modo de mapa de ", "")\
				.replace("Modo mapa de ", "")\
				.replace("Modo mapa ", "")
		display_name = tr("Mapmode %s") % display_name.capitalize()
		Keychain.actions[action].display_name = display_name
	Keychain.selected_profile.bindings[action] = events

func _ready() -> void:
	var profile_is_dirty : bool = false
	var default := (Keychain.DEFAULT_PROFILE as ShortcutProfile)

	# Setup Keychain for Hotkeys
	for action : StringName in InputMap.get_actions():
		if not Keychain.keep_binding_check.call(action):
			continue
		profile_is_dirty = true
		default.bindings[action] = InputMap.action_get_events(action)
		_add_hotkey(action, default.bindings[action])

	var has_hotkey_not_loaded_yet := false
	for action : StringName in Keychain.selected_profile.bindings:
		if InputMap.has_action(action): continue
		profile_is_dirty = true
		InputMap.add_action(action)
		default.bindings[action] = Keychain.selected_profile.bindings[action]
		_add_hotkey(action, default.bindings[action])
		has_hotkey_not_loaded_yet = true

	if has_hotkey_not_loaded_yet:
		reset_confirmation.dialog_text += "\nWarning: Hotkeys are not loaded, reset may use incorrect default values."

	if profile_is_dirty:
		Keychain.selected_profile.save()

	super._ready()
