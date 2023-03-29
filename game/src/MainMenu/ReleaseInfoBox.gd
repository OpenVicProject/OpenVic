extends HBoxContainer

@export
var _version_label : Button

@export
var _commit_label : Button

@export
var _checksum_label : Button

var _checksum : String = "????"

# REQUIREMENTS:
# * UIFUN-97
func _ready():
	_version_label.text = _GIT_INFO_.release_name
	_version_label.tooltip_text = _GIT_INFO_.tag
	_commit_label.text = _GIT_INFO_.short_hash
	_commit_label.tooltip_text = _GIT_INFO_.commit_hash
	# UI-111
	_checksum = Checksum.get_checksum_text()
	_update_checksum_label_text()

func _notification(what : int):
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_checksum_label_text()

func _update_checksum_label_text() -> void:
	_checksum_label.tooltip_text = tr("MAINMENU_CHECKSUM") % _checksum
	_checksum_label.text = "(%s)" % _checksum.substr(0, 4)

func _on_version_label_pressed():
	DisplayServer.clipboard_set(_GIT_INFO_.tag)

func _on_commit_label_pressed():
	DisplayServer.clipboard_set(_GIT_INFO_.commit_hash)

func _on_checksum_label_pressed():
	DisplayServer.clipboard_set(_checksum)
