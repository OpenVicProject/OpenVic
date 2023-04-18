extends Panel

@export var _province_name_label : Label
@export var _region_name_label : Label

var _province_identifier: String = "":
	get: return _province_identifier
	set(v):
		_province_identifier = v
		update_info()

const _name_suffix : String = "_NAME"

func _ready():
	update_info()

func update_info() -> void:
	if _province_identifier:
		_province_name_label.text = _province_identifier + _name_suffix
		var region_identifier := MapSingleton.get_region_identifier_from_province_identifier(_province_identifier)
		if region_identifier:
			_region_name_label.text = region_identifier + _name_suffix
		else:
			_region_name_label.text = "NO REGION"
		show()
	else:
		hide()

func _on_province_selected(identifier : String) -> void:
	_province_identifier = identifier

func _on_close_button_pressed() -> void:
	_province_identifier = ""
