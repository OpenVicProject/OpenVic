extends RefCounted

# REQUIREMENTS
# * SS-59, SS-60, SS-61
func get_default_locale() -> String:
	var locales := TranslationServer.get_loaded_locales()
	var default_locale := OS.get_locale()
	if default_locale in locales:
		return default_locale
	var default_language := OS.get_locale_language()
	for locale in locales:
		if locale.begins_with(default_language):
			return default_language
	return ProjectSettings.get_setting("internationalization/locale/fallback", "en_GB")

func load_localisation(dir_path : String) -> void:
	if LoadLocalisation.load_localisation_dir(dir_path) == OK:
		print("loaded locales: ", TranslationServer.get_loaded_locales())
	else:
		push_error("Failed to load localisation directory: ", dir_path)

# REQUIREMENTS
# * SS-57
# * FS-17
func _init():
	var localisation_dir_path : String = ProjectSettings.get_setting("internationalization/locale/localisation_path", "")
	if localisation_dir_path.is_empty():
		push_error("Missing localisation_path setting!")
	else:
		load_localisation(localisation_dir_path)
