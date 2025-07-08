extends Control

signal back_button_pressed

@export var mod_list_box : VBoxContainer

var mod_info: Array[Dictionary]
var checkboxes = {}
var selected_mods : PackedStringArray = []
var selected_and_required_mods : PackedStringArray = []

func _ready():
	mod_info = GameSingleton.get_mod_info()
	var mod_status_file := ConfigFile.new()
	mod_status_file.load("user://mods.cfg")
	selected_mods = mod_status_file.get_value("mods", "load_list", [])
	for mod in selected_mods:
		_select_mod_dependencies(mod)

	for mod in mod_info:
		var mod_name : String = mod["mod_identifier"]
		var mod_loaded : bool = mod["mod_loaded"]

		var hbox : HBoxContainer = HBoxContainer.new()
		hbox.name = mod_name

		var checkbox : CheckBox = CheckBox.new()
		checkbox.text = mod_name
		checkbox.button_pressed = mod_loaded
		checkbox.disabled = mod_loaded and mod_name not in selected_mods
		checkbox.toggled.connect(_on_mod_toggled.bind(mod_name))
		checkboxes[mod_name] = checkbox
		hbox.add_child(checkbox)

		var status : Label = Label.new()
		status.text = "Loaded" if mod_loaded else ""
		hbox.add_child(status)

		mod_list_box.add_child(hbox)

func _select_mod_dependencies(mod_name: String):
	if mod_name not in selected_and_required_mods:
		selected_and_required_mods.push_back(mod_name)
	for dep in _get_mod_from_identifier(mod_name)["mod_dependencies"]:
		if checkboxes.has(dep):
			var dep_checkbox: CheckBox = checkboxes[dep]
			dep_checkbox.set_pressed_no_signal(true)
			dep_checkbox.disabled = true
			dep_checkbox.tooltip_text = "This mod is a dependency of another mod, and cannot be disabled."
		_select_mod_dependencies(dep)

func _deselect_mod_dependencies(mod_name: String):
	if not _is_dependency_required(mod_name):
		if mod_name in selected_and_required_mods:
			selected_and_required_mods.remove_at(selected_and_required_mods.find(mod_name))
		for dep in _get_mod_from_identifier(mod_name)["mod_dependencies"]:
			if checkboxes.has(dep):
				var dep_checkbox: CheckBox = checkboxes[dep]
				dep_checkbox.set_pressed_no_signal(false)
				dep_checkbox.disabled = false
				dep_checkbox.tooltip_text = ""
			_deselect_mod_dependencies(dep)

func _get_mod_from_identifier(mod_name: String) -> Dictionary:
	for mod in mod_info:
		if mod["mod_identifier"] == mod_name:
			return mod
	return {"mod_dependencies":[]}

func _is_dependency_required(dep_name: String) -> bool:
	for mod_name in selected_mods:
		var deps: PackedStringArray = _get_mod_from_identifier(mod_name)["mod_dependencies"]
		if dep_name in deps:
			return true
	return false

func _on_mod_toggled(checked: bool, mod_name: String) -> void:
	if checked:
		print("Selected Mod: " + mod_name)
		selected_mods.push_back(mod_name)
		_select_mod_dependencies(mod_name)
	else:
		print("Unselected Mod: " + mod_name)
		selected_mods.remove_at(selected_mods.find(mod_name))
		_deselect_mod_dependencies(mod_name)

func _on_save_button_pressed():
	var mod_status_file := ConfigFile.new()
	mod_status_file.set_value("mods", "load_list", selected_mods)
	mod_status_file.save("user://mods.cfg")
	# reload game to apply changes
	OS.set_restart_on_exit(true)
	get_tree().quit()

func _on_back_button_pressed():
	back_button_pressed.emit()
