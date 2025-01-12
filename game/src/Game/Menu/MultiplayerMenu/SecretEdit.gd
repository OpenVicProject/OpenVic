extends HBoxContainer
class_name SecretEdit

@export var editable : bool = true
@export var start_hidden : bool = true

@export var line : LineEdit
@export var check : CheckButton

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	line.editable = editable
	
	check.button_pressed = start_hidden
	line.secret = start_hidden

func get_text() -> StringName:
	return line.text

func set_editable(value : bool) -> void:
	editable = value
	line.editable = editable

func set_hidden(value : bool) -> void:
	check.button_pressed = value
	line.secret = value

func set_text(value : StringName) -> void:
	line.text = value
