extends HBoxContainer

@export
var _version_label : Button

@export
var _commit_label : Button

@export
var _checksum_label : Button

var _checksum : String = "????"

# REQUIREMENTS:
# * SS-104, SS-105, SS-106, SS-107
# * UIFUN-97, UIFUN-297, UIFUN-299
func _ready() -> void:
	_version_label.text = tr("MAIMENU_LATEST_RELEASE_NAME").format({ "release_name": _GIT_INFO_.release_name })
	_version_label.tooltip_text = _GIT_INFO_.tag
	_commit_label.text = tr("MAIMENU_COMMIT").format({ "short_hash": _GIT_INFO_.short_hash })
	_commit_label.tooltip_text = _GIT_INFO_.commit_hash
	# UI-111
	_checksum = Checksum.get_checksum_text()
	_update_checksum_label_text()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_checksum_label_text()

func _update_checksum_label_text() -> void:
	_checksum_label.tooltip_text = tr("MAINMENU_CHECKSUM_TOOLTIP").format({ "checksum": _checksum })
	_checksum_label.text = tr("MAINMENU_CHECKSUM").format({ "short_checksum": _checksum.substr(0, 4) })

func _on_version_label_pressed() -> void:
	DisplayServer.clipboard_set(_GIT_INFO_.tag)

func _on_commit_label_pressed() -> void:
	DisplayServer.clipboard_set(_GIT_INFO_.commit_hash)

func _on_checksum_label_pressed() -> void:
	DisplayServer.clipboard_set(_checksum)

func _on_game_info_button_pressed() -> void:
	var project_name := ProjectSettings.get_setting("application/config/name") as String
	var tag_name := _GIT_INFO_.tag
	var commit_sha := _GIT_INFO_.short_hash
	var godot_version := Engine.get_version_info().string as String
	var os_name := OS.get_distribution_name()
	var date_str := Time.get_datetime_string_from_system(true)
	var display_server := DisplayServer.get_name()
	var gpu_name := RenderingServer.get_video_adapter_name()
	var gpu_api_version := RenderingServer.get_video_adapter_api_version()
	var cpu_name := OS.get_processor_name()
	var cpu_processor_count := OS.get_processor_count()
	DisplayServer.clipboard_set("%s %s (%s) [Godot %s] - %s %s - %s - %s (API: %s) - %s (%s Threads)"
	% [project_name, tag_name, commit_sha, godot_version, os_name, date_str, display_server, gpu_name, gpu_api_version, cpu_name, cpu_processor_count])
