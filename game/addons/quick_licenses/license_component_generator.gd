@tool
extends EditorScript
class_name LicenseComponentGenerator

func _run() -> void:
	var components_json := preload("res://addons/quick_licenses/components.json")

	var skip_name: PackedStringArray = []
	for component in components_json.data:
		skip_name.append(component.name)

	for component in OVGame.get_copyright_info():
		if component.name in skip_name: continue
		components_json.data.append({ "name": component.name, "source": "" })
		skip_name.append(component.name)

	for component in OVSimulation.get_copyright_info():
		if component.name in skip_name: continue
		components_json.data.append({ "name": component.name, "source": "" })
		skip_name.append(component.name)

	for component in Engine.get_copyright_info():
		if component.name in skip_name: continue
		components_json.data.append({ "name": component.name, "source": "" })
		skip_name.append(component.name)

	var index_to_erase: PackedInt64Array = []
	for index: int in range(components_json.data.size()):
		var duplicate_index: int = components_json.data.find_custom(func(c: Dictionary) -> bool: return c.name == components_json.data[index].name, index+1)
		if duplicate_index == -1: continue
		index_to_erase.append(duplicate_index)
	index_to_erase.reverse()

	for erase_index: int in index_to_erase:
		components_json.data.remove_at(erase_index)

	var components_file := FileAccess.open(components_json.resource_path, FileAccess.WRITE)
	if components_file == null:
		return

	components_file.store_string(JSON.stringify(components_json.data, "\t", false))
	print(components_json.resource_path, " successfully updated.")
