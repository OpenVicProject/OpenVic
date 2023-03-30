extends Panel

@export var _province_name_label : Label

@export var province_identifier: String = "":
	get: return province_identifier
	set(v):
		province_identifier = v
		update_info()

func _ready():
	update_info()

func update_info() -> void:
	_province_name_label.text = province_identifier + "_NAME"
	visible = not province_identifier.is_empty()

func _on_province_selected(identifier : String) -> void:
	province_identifier = identifier

func _on_button_pressed() -> void:
	province_identifier = ""
