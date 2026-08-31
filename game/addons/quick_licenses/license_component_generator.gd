@tool
extends EditorScript
class_name LicenseComponentGenerator

var index_to_name: PackedStringArray = []

func append_copyright_info(copyright_info: Array, component_dictionary: Dictionary[String, Dictionary]) -> void:
	for component in copyright_info:
		if component.name in index_to_name: continue
		component = { "name": component.name, "source": "" }
		if component_dictionary.has(component.name):
			component_dictionary[component.name].merge(component)
		else:
			component_dictionary[component.name] = component
		index_to_name.append(component.name)

func _run() -> void:
	var components_json := preload("res://addons/quick_licenses/components.json")

	var component_dictionary: Dictionary[String, Dictionary] = {}
	for component in components_json.data:
		component_dictionary[component.name] = component

	index_to_name.clear()

	append_copyright_info(OVGame.get_copyright_info(), component_dictionary)
	append_copyright_info(OVDataloader.get_copyright_info(), component_dictionary)
	append_copyright_info(OVLexyVDF.get_copyright_info(), component_dictionary)
	append_copyright_info(OVSimulation.get_copyright_info(), component_dictionary)
	append_copyright_info(Engine.get_copyright_info(), component_dictionary)

	components_json.data.clear()
	for index: int in range(index_to_name.size()):
		components_json.data.append(component_dictionary[index_to_name[index]])

	var components_file := FileAccess.open(components_json.resource_path, FileAccess.WRITE)
	if components_file == null:
		return

	components_file.store_string(JSON.stringify(components_json.data, "\t", false))
	print(components_json.resource_path, " successfully updated.")
