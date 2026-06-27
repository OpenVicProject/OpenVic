@tool
extends EditorPlugin

const TranslatorPlugin := preload("uid://d0rgx3infg54m")

var _translation_plugin := TranslatorPlugin.new()

func _enable_plugin() -> void:
	add_translation_parser_plugin(_translation_plugin)

func _disable_plugin() -> void:
	remove_translation_parser_plugin(_translation_plugin)
