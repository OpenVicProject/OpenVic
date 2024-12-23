@tool
extends EditorPlugin

const ReleaseExportEditorPlugin := preload(
	"res://addons/openvic-plugin/ReleaseExportEditorPlugin.gd"
)
var release_export_editor_plugin := ReleaseExportEditorPlugin.new()


func _enter_tree() -> void:
	add_export_plugin(release_export_editor_plugin)


func _exit_tree() -> void:
	remove_export_plugin(release_export_editor_plugin)
