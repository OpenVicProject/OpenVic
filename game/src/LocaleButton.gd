extends OptionButton

const section_name : String = "localisation"
const setting_name : String = "locale"

var _default_locale_index : int

func _ready():
	var locales_country_rename : Dictionary = ProjectSettings.get_setting("internationalization/locale/country_short_name", {})

	var locales_list = TranslationServer.get_loaded_locales()
	var default_locale := Events.Localisation.get_default_locale()
	if default_locale not in locales_list:
		locales_list.push_back(default_locale)

	for locale in locales_list:
		# locale_name consists of a compulsory language name and optional script
		# and country names, in the format: "<language>[ (script)][, country]"
		var locale_name := TranslationServer.get_locale_name(locale)
		var comma_idx := locale_name.find(", ")
		if comma_idx != -1:
			var locale_country_name := locale_name.substr(comma_idx + 2)
			locale_country_name = locales_country_rename.get(locale_country_name, "")
			if not locale_country_name.is_empty():
				locale_name = locale_name.left(comma_idx + 2) + locale_country_name

		add_item(locale_name)
		set_item_metadata(item_count - 1, locale)

		if locale == default_locale:
			_default_locale_index = item_count - 1

	Events.Options.load_settings.connect(load_setting)
	Events.Options.save_settings.connect(save_setting)

func _notification(what : int):
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_select_locale_by_string(TranslationServer.get_locale())

func _valid_index(index : int) -> bool:
	return 0 <= index and index < item_count

func load_setting(file : ConfigFile) -> void:
	if file == null: return
	var load_value = file.get_value(section_name, setting_name, Events.Localisation.get_default_locale())
	match typeof(load_value):
		TYPE_STRING, TYPE_STRING_NAME:
			if _select_locale_by_string(load_value as String):
				item_selected.emit(selected)
				return
	push_error("Setting value '%s' invalid for setting [%s] %s" % [load_value, section_name, setting_name])
	reset_setting()

func _select_locale_by_string(locale : String) -> bool:
	for idx in item_count:
		if get_item_metadata(idx) == locale:
			selected = idx
			return true
	selected = _default_locale_index
	return false

# REQUIREMENTS:
# * UIFUN-74
func save_setting(file : ConfigFile) -> void:
	if file == null: return
	file.set_value(section_name, setting_name, get_item_metadata(selected))

func reset_setting() -> void:
	_select_locale_by_string(TranslationServer.get_locale())

# REQUIREMENTS:
# * SS-58
func _on_item_selected(index : int) -> void:
	if _valid_index(index):
		TranslationServer.set_locale(get_item_metadata(index))
		Events.Options.save_settings_to_file.call_deferred()
	else:
		push_error("Invalid LocaleButton index: %d" % index)
		reset_setting()
