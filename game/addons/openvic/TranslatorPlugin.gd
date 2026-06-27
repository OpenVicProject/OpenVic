@tool
extends EditorTranslationParserPlugin

const AppSetting := preload("res://addons/kenyoni/app_settings/app_settings.gd")

func _customize_strings(strings: Array[PackedStringArray]) -> Array[PackedStringArray]:
	strings.append_array(generate_localization(GameSettings))
	strings.append_array(generate_localization(ModSettings))
	strings.append_array(generate_localization(Vic2Settings))

	var keychain_path: String = ControlsGlobal.get_script().resource_path
	for action_id: StringName in ControlsGlobal.actions:
		strings.append([ControlsGlobal.actions[action_id].display_name, "", "", "", keychain_path])

	for group_name: StringName in ControlsGlobal.groups:
		strings.append([group_name, "", "", "", keychain_path])

	return strings

func generate_localization(app_setting: AppSetting) -> Array[PackedStringArray]:
	var resource_path: String = app_setting.get_script().resource_path

	var result: Array[PackedStringArray] = []
	for key: String in app_setting.get_sub_sections():
		result.append([key.capitalize(), "", "", "", resource_path])

	var groups_handled: Array[GameSettings.RevertGroup] = []

	for setting: AppSetting.Setting in app_setting.get_section(""):
		if setting.has_meta(&"display_name"):
			result.append([setting.get_meta(&"display_name"), "", "", "", resource_path])
		else:
			var key := setting.key()
			result.append([key.get_slice("/", key.get_slice_count("/") - 1).capitalize(), "", "", "", resource_path])

		if setting.has_meta(&"description"):
			result.append([setting.get_meta(&"description"), "", "", "", resource_path])

		if setting.has_meta(&"display_template"):
			result.append([setting.get_meta(&"display_template"), "", "", "", resource_path])
		else:
			var display_values: PackedStringArray = setting.get_meta(&"display_values", PackedStringArray())
			for value: String in display_values:
				if value.is_empty(): continue
				result.append([value, "", "", "", resource_path])

		if setting.has_meta(&"revert_group"):
			var revert_group: GameSettings.RevertGroup = setting.get_meta(&"revert_group")
			if not groups_handled.has(revert_group):
				groups_handled.append(revert_group)
				result.append([revert_group.title, "", "", "", resource_path])
				result.append([revert_group.text, "", "", "", resource_path])

	return result
