extends "res://addons/keychain/ShortcutEdit.gd"

func _ready() -> void:
	# Setup Keychain for Hotkeys
	for action : StringName in InputMap.get_actions():
		if not Keychain.keep_binding_check.call(action):
			continue
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
		Keychain.profiles[0].bindings[action] = InputMap.action_get_events(action)

	super._ready()
