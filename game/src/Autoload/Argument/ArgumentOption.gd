@tool
class_name ArgumentOption
extends Resource

@export var name : StringName
@export var aliases : Array[StringName] = []
@export var type : Variant.Type :
	get: return type
	set(v):
		type = v
		match v:
			TYPE_BOOL: default_value = false
			TYPE_INT: default_value = 0
			TYPE_FLOAT: default_value = 0.0
			TYPE_STRING: default_value = ""
			TYPE_STRING_NAME: default_value = &""
			TYPE_PACKED_STRING_ARRAY: default_value = PackedStringArray()
			TYPE_COLOR: default_value = Color()
			_: default_value = null
		notify_property_list_changed()
var default_value : Variant
@export var description : String

func _init(_name := "", _type := TYPE_NIL, _description := "", default : Variant = null) -> void:
	name = _name
	type = _type
	if default != null and typeof(default) == type:
		default_value = default
	description = _description

func add_alias(alias : StringName) -> ArgumentOption:
	aliases.append(alias)
	return self

func get_type_string() -> StringName:
	match type:
		TYPE_NIL: return "null"
		TYPE_BOOL: return "boolean"
		TYPE_INT: return "integer"
		TYPE_FLOAT: return "float"
		TYPE_STRING, TYPE_STRING_NAME: return "string"
		TYPE_PACKED_STRING_ARRAY: return "string array"
		TYPE_COLOR: return "color"
	return "<invalid type>"

func _get(property : StringName) -> Variant:
	if property == "default_value": return default_value
	return null

func _set(property : StringName, value : Variant) -> bool:
	if property == "default_value":
		default_value = value
		return true
	return false

func _get_property_list() -> Array[Dictionary]:
	var properties := [] as Array[Dictionary]

	properties.append({
		"name": "default_value",
		"type": type
	})

	return properties
