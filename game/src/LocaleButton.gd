extends OptionButton

var _locales_country_rename : Dictionary
var _locales_list : Array[String]

func _ready():
	print("Loading locale button")

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


func _on_item_selected(index):
	print("Selected locale " + _locales_list[index])
	TranslationServer.set_locale(_locales_list[index])
