class_name TranslationMerge
extends Translation

var _translation_merges : Array[Translation] = []

func _get_message(src_message : StringName, context : StringName) -> StringName:
	var msg : StringName = &""
	for translation in _translation_merges:
		msg = translation.get_message(src_message, context)
		if not msg.is_empty(): return msg
	return &""

func _get_plural_message(src_message: StringName, src_plural_message: StringName, n: int, context: StringName) -> StringName:
	var msg : StringName = &""
	for translation in _translation_merges:
		msg = translation.get_plural_message(src_message, src_plural_message, n, context)
		if not msg.is_empty(): return msg
	return &""

func merge_with(translation : Translation):
	if translation == null or translation == self: return
	if _translation_merges.has(translation):
		return
	_translation_merges.append(translation)

	if TranslationServer.get_loaded_locales().has(locale):
		Engine.get_main_loop().notification(MainLoop.NOTIFICATION_TRANSLATION_CHANGED);
