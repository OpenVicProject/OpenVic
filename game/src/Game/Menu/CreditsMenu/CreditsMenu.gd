extends Control

signal back_button_pressed

###############
# Credits CSV format
# The project title row is the only requirement within the csv file, however
# it can be on any row, so long as it exists.
# ----------------------
# title,project-title
# role-name,person-name
# role-name,person-name
# role-name,person-name
# ...
###############

@export_file("*.csv") var core_credits_path: String

@export var godot_engine_scene: PackedScene

@export_group("Label Variants", "label_variants_")
@export var label_variants_project: StringName

@export var label_variants_role: StringName

@export var label_variants_person: StringName

@export var credits_list: VBoxContainer

const title_key: String = "TITLE"

# REQUIREMENTS:
# * 1.5 Credits Menu
# * SS-17


# REQUIREMENTS
# * FS-4
func _load_credit_file(path: String) -> Dictionary:
	var roles := {}
	var core_credits := FileAccess.open(path, FileAccess.READ)
	if core_credits == null:
		push_error(
			"Failed to open credits file %s (error code %d)" % [path, FileAccess.get_open_error()]
		)
		return roles

	while not core_credits.eof_reached():
		var line := core_credits.get_csv_line()
		var role := line[0].strip_edges().to_upper()

		# If the line does not have an identifiable role or is empty then skip it
		if role.is_empty() or line.size() < 2:
			if not (role.is_empty() and line.size() < 2):
				push_warning("Incorrectly formatted credit line %s in %s" % [line, path])
			continue

		var person := line[1].strip_edges()

		if person.is_empty():
			push_warning("Incorrectly formatted credit line %s in %s" % [line, path])
			continue
		if line.size() > 2:
			push_warning("Extra entries ignored in credit line %s in %s" % [line, path])

		if role not in roles:
			roles[role] = [person]
		else:
			if person in roles[role]:
				push_warning("Duplicate person %s for role %s in %s" % [person, role, path])
			else:
				roles[role].push_back(person)
	if title_key in roles:
		if roles[title_key].size() > 1:
			push_warning("More than one %s: %s in %s" % [title_key, roles[title_key], path])
			roles[title_key] = [roles[title_key][0]]
	else:
		push_warning("Credits file %s missing %s" % [path, title_key])
	for role_list: Array in roles.values():
		role_list.sort_custom(
			func(a: String, b: String) -> bool: return a.naturalnocasecmp_to(b) < 0
		)
	return roles


func _add_label(node: Node, text: String, type_variation: StringName) -> void:
	var label := Label.new()
	label.name = "Label" + text
	label.text = text
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.theme_type_variation = type_variation
	node.add_child(label)


# REQUIREMENTS:
# * UI-34, UI-35
func _add_project_credits(project: Dictionary) -> void:
	var project_credits_list := VBoxContainer.new()
	project_credits_list.name = "Credits"
	if title_key in project:
		var title: String = project[title_key][0]
		project_credits_list.name += title
		_add_label(project_credits_list, title, label_variants_project)
		project_credits_list.add_child(HSeparator.new())

	for role: String in project:
		if role == title_key:
			continue

		var role_parent := VBoxContainer.new()

		for person: String in project[role]:
			_add_label(role_parent, person, label_variants_person)

		_add_label(project_credits_list, role, label_variants_role)
		project_credits_list.add_child(role_parent)
		project_credits_list.add_child(HSeparator.new())

	credits_list.add_child(project_credits_list)


func _add_godot_credits() -> void:
	var godot_credits_list := VBoxContainer.new()
	godot_credits_list.name = "CreditsGodot"
	var godot_engine := godot_engine_scene.instantiate()
	godot_credits_list.add_child(godot_engine)
	godot_credits_list.add_child(HSeparator.new())

	var author_dict := Engine.get_author_info()
	_add_label(godot_credits_list, "Contributors", label_variants_role)

	for role: String in author_dict:
		var role_parent := VBoxContainer.new()

		for person: String in author_dict[role]:
			_add_label(role_parent, person, label_variants_person)

		_add_label(godot_credits_list, role.replace("_", " ").capitalize(), label_variants_role)
		godot_credits_list.add_child(role_parent)
		godot_credits_list.add_child(HSeparator.new())

	var donor_dict := Engine.get_donor_info()
	_add_label(godot_credits_list, "Donors", label_variants_role)

	for role: String in donor_dict:
		if donor_dict[role].size() == 0 or donor_dict[role][0].begins_with("None"):
			continue
		var role_parent := VBoxContainer.new()

		for person: String in donor_dict[role]:
			_add_label(role_parent, person, label_variants_person)

		_add_label(godot_credits_list, role.replace("_", " ").capitalize(), label_variants_role)
		godot_credits_list.add_child(role_parent)
		godot_credits_list.add_child(HSeparator.new())

	credits_list.add_child(godot_credits_list)


func _add_link_button(node: Node, text: String, url: String, type_variation: StringName) -> void:
	var button := LinkButton.new()
	button.name = "LinkButton" + text
	button.text = text
	button.uri = url
	button.size_flags_horizontal = SIZE_SHRINK_CENTER
	button.theme_type_variation = type_variation
	node.add_child(button)


func _add_licenses() -> void:
	var license_list := VBoxContainer.new()
	license_list.name = "Licenses"
	_add_label(license_list, "Third-Party Licenses", label_variants_project)
	license_list.add_child(HSeparator.new())

	var license_info := {
		"OpenVic": ["GPLv3", "https://github.com/OpenVicProject/OpenVic/blob/main/LICENSE.md"],
		"Godot": ["MIT", "https://github.com/godotengine/godot/blob/master/LICENSE.txt"],
		"FreeType":
		[
			"FreeType License",
			"https://gitlab.freedesktop.org/freetype/freetype/-/blob/master/docs/FTL.TXT"
		],
		"ENet": ["MIT", "http://enet.bespin.org/License.html"],
		"mbed TLS": ["APLv2", "https://github.com/Mbed-TLS/mbedtls/blob/development/LICENSE"]
	}
	# Add additional licenses required for attribution here
	# These licenses should also either be displayed or exported alongside this project

	for project: String in license_info:
		_add_label(license_list, project, label_variants_role)
		_add_link_button(
			license_list, license_info[project][0], license_info[project][1], label_variants_person
		)
		license_list.add_child(HSeparator.new())

	credits_list.add_child(license_list)


# REQUIREMENTS:
# * SS-17
func _ready() -> void:
	_add_project_credits(_load_credit_file(core_credits_path))
	_add_godot_credits()
	_add_licenses()


func _input(event: InputEvent) -> void:
	if self.is_visible_in_tree():
		if event.is_action_pressed("ui_cancel"):
			_on_back_button_pressed()


# REQUIREMENTS:
# * UI-38
# * UIFUN-37
func _on_back_button_pressed() -> void:
	back_button_pressed.emit()
