extends HBoxContainer
class_name DirectConnectionEntry

@export var ip_edit : SecretEdit
@export var name_edit : LineEdit
@export var connect_button : Button
@export var delete_button : Button

# Collects connect and delete presses from its children, and emits
#  the corresponding signals for the connection controller to handle

signal connect_to_ip(ip : String)
signal delete()

func setup(name_in : StringName, ip : StringName) -> void:
	name_edit.text = name_in
	ip_edit.set_text(ip)

func _on_connect_pressed() -> void:
	connect_to_ip.emit(ip_edit.get_text())

func _on_delete_pressed() -> void:
	delete.emit()
