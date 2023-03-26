extends Node

func _ready():
	var locales := [TranslationServer.get_locale()]
	locales.append_array(TranslationServer.get_loaded_locales())

	for locale in locales:
		var translation := TranslationServer.get_translation_object(locale)
		TranslationServer.remove_translation(translation)
		var new_translation := TranslationMerge.new()
		new_translation.locale = locale
		new_translation.merge_with(translation)
		TranslationServer.add_translation(new_translation)

	var translation_path : String = ProjectSettings.get_setting("internationalization/locale/localization_path")
	if OS.has_feature("template"):
		_add_translation(OS.get_executable_path().get_base_dir().path_join(translation_path))
	_add_translation("res://" + translation_path)

func _add_translation(dir_path : String):
	var dir := DirAccess.open(dir_path)
	if dir == null: return
	for file_name in dir.get_files():
		if file_name.get_extension() == "po":
			var t: Translation = ResourceLoader.load(dir_path.path_join(file_name))
			if TranslationServer.get_loaded_locales().has(t.locale):
				TranslationServer.get_translation_object(t.locale).merge_with(t)
				continue
			var new_t : TranslationMerge = TranslationMerge.new()
			new_t.locale = t.locale
			new_t.merge_with(t)
			TranslationServer.add_translation(new_t)
