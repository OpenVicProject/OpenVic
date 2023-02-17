extends OptionButton

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


func load_setting(file : ConfigFile):
	var locale_index := _locales_list.find(file.get_value("Localization", "Locale", "") as String)
	if locale_index != -1:
		selected = locale_index

func save_setting(file : ConfigFile):
	file.set_value("Localization", "Locale", _locales_list[selected])

func _on_item_selected(index):
	TranslationServer.set_locale(_locales_list[index])
	Events.Options.save_settings_from_file.call_deferred()
