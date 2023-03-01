extends OptionButton

const section_name : String = "Localization"
const setting_name : String = "Locale"

var _locales_country_rename : Dictionary
var _locales_list : Array[String]

func _ready():
	_locales_country_rename = ProjectSettings.get_setting("internationalization/locale/country_short_name", {})

	_locales_list = [TranslationServer.get_locale()]
	_locales_list.append_array(TranslationServer.get_loaded_locales())

	for locale in _locales_list:
		var locale_name := TranslationServer.get_locale_name(locale)
		var locale_first_part := locale_name.get_slice(", ", 0)
		var locale_second_part := locale_name.substr(locale_first_part.length() + 2)
		if locale_second_part in _locales_country_rename:
			locale_second_part = _locales_country_rename[locale_second_part]

		add_item("%s, %s" % [locale_first_part, locale_second_part])

	Events.Options.load_settings.connect(load_setting)
	Events.Options.save_settings.connect(save_setting)
	
func _notification(what):
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_select_locale_by_string(TranslationServer.get_locale())

func _valid_index(index : int) -> bool:
	return 0 <= index and index < _locales_list.size()

func load_setting(file : ConfigFile) -> void:
	if file == null: return
	var load_value = file.get_value(section_name, setting_name, TranslationServer.get_locale())
	match typeof(load_value):
		TYPE_STRING, TYPE_STRING_NAME:
			if _select_locale_by_string(load_value as String):
				item_selected.emit(selected)
				return
	push_error("Setting value '%s' invalid for setting [%s] %s" % [load_value, section_name, setting_name])
	reset_setting()
	
func _select_locale_by_string(locale : String) -> bool:
	var locale_index := _locales_list.find(locale)
	if locale_index != -1:
		selected = locale_index
		return true
	return false

# REQUIREMENTS:
# * UIFUN-74
func save_setting(file : ConfigFile) -> void:
	if file == null: return
	file.set_value(section_name, setting_name, _locales_list[selected])

func reset_setting() -> void:
	selected = _locales_list.find(TranslationServer.get_locale())

# REQUIREMENTS:
# * SS-58
func _on_item_selected(index : int) -> void:
	if _valid_index(index):
		TranslationServer.set_locale(_locales_list[index])
		Events.Options.save_settings_to_file.call_deferred()
	else:
		push_error("Invalid LocaleButton index: %d" % index)
		reset_setting()
