@tool
extends LobbyPanelButton

signal request_to_delete

@export_group("Nodes")
@export var country_flag : TextureRect
@export var date_label : Label
@export var delete_button : BaseButton

var resource : SaveResource:
	get:
		return resource
	set(value):
		if resource != null:
			resource.changed.disconnect(_resource_changed)
		resource = value
		if resource != null:
			resource.changed.connect(_resource_changed)
		_resource_changed()

func get_text() -> StringName:
	return resource.save_name

func set_text(value : StringName) -> void:
	if resource != null:
		resource.save_name = value

func _ready():
	_resource_changed()

func _is_start_date() -> bool:
	return false

func _resource_changed() -> void:
	if resource == null: return
	name_label.text = resource.save_name
	date_label.text = Time.get_datetime_string_from_unix_time(resource.get_save_file_time(), true)

func _on_delete_button_pressed() -> void:
	request_to_delete.emit()
