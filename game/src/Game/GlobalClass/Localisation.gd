class_name Localisation
extends RefCounted


# REQUIREMENTS
# * SS-59, SS-60, SS-61
static func get_default_locale() -> String:
	var locales: PackedStringArray = TranslationServer.get_loaded_locales()
	var default_locale := OS.get_locale()
	if default_locale in locales:
		return default_locale
	var default_language := OS.get_locale_language()
	for locale: String in locales:
		if locale.begins_with(default_language):
			return default_language
	return ProjectSettings.get_setting("internationalization/locale/fallback", "en_GB")


static func load_localisation(dir_path: String) -> void:
	if LoadLocalisation.load_localisation_dir(dir_path) != OK:
		push_error("Error loading localisation directory: ", dir_path)
	var loaded_locales: PackedStringArray = TranslationServer.get_loaded_locales()
	print("Loaded ", loaded_locales.size(), " locales: ", loaded_locales)


# REQUIREMENTS
# * SS-57
# * FS-17
static func initialize() -> void:
	var localisation_dir_path: String = ProjectSettings.get_setting(
		"internationalization/locale/localisation_path", ""
	)
	if localisation_dir_path.is_empty():
		push_error("internationalization/locale/localisation_path setting is empty!")
	else:
		Localisation.load_localisation(localisation_dir_path)


static func tr_number(num: Variant) -> String:
	return TextServerManager.get_primary_interface().format_number(str(num))
