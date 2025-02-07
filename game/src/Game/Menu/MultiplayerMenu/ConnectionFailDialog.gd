extends AcceptDialog
class_name ConnectionFailDialog

enum FAIL_REASONS {NO_SERVER, PASSWORD, BAD_IP}

@export var reason_text : String
@export var no_server_fail_text : String
@export var bad_password_fail_text : String
@export var bad_ip_text : String

@onready var reason_text_map : Dictionary = {
	FAIL_REASONS.NO_SERVER : no_server_fail_text,
	FAIL_REASONS.PASSWORD : bad_password_fail_text,
	FAIL_REASONS.BAD_IP : bad_ip_text
}

@export var label : Label

func display(reason : FAIL_REASONS) -> void:
	label.text = "%s %s" % [tr(reason_text), tr(reason_text_map[reason])]
	popup_centered()
