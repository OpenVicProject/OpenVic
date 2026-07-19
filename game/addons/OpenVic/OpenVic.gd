@tool
extends EditorPlugin

const TranslatorPlugin := preload("uid://d0rgx3infg54m")

var _translation_plugin := TranslatorPlugin.new()


func _enter_tree() -> void:
	add_translation_parser_plugin(_translation_plugin)


func _exit_tree() -> void:
	remove_translation_parser_plugin(_translation_plugin)
