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

@export_file("*.csv")
var core_credits_path : String

@export_group("Label Variants", "label_variants_")
@export
var label_variants_project : StringName

@export
var label_variants_role : StringName

@export
var label_variants_person : StringName

@export
var credits_list: VBoxContainer

const title_key : String = "TITLE"

# REQUIREMENTS:
# * 1.5 Credits Menu
# * SS-17

# REQUIREMENTS
# * FS-4
func _load_credit_file(path : String) -> Dictionary:
	var roles := {}
	var core_credits = FileAccess.open(path, FileAccess.READ)
	if core_credits == null:
		push_error("Failed to open credits file %s (error code %d)" % [path, FileAccess.get_open_error()])
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
	for role_list in roles.values():
		role_list.sort_custom(func(a : String, b : String) -> bool: return a.naturalnocasecmp_to(b) < 0)
	return roles

func _add_label(node : Node, text : String, type_variation : StringName) -> void:
	var label := Label.new()
	label.name = 'Label' + text
	label.text = text
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.theme_type_variation = type_variation
	node.add_child(label)

# REQUIREMENTS:
# * UI-34, UI-35
func _add_project_credits(project : Dictionary) -> void:
	var project_credits_list = VBoxContainer.new()
	project_credits_list.name = 'Credits'
	if title_key in project:
		var title : String = project[title_key][0]
		project_credits_list.name += title
		_add_label(project_credits_list, title, label_variants_project)
		project_credits_list.add_child(HSeparator.new())

	for role in project:
		if role == title_key:
			continue

		var role_parent = VBoxContainer.new()

		for person in project[role]:
			_add_label(role_parent, person, label_variants_person)

		_add_label(project_credits_list, role, label_variants_role)
		project_credits_list.add_child(role_parent)
		project_credits_list.add_child(HSeparator.new())

	credits_list.add_child(project_credits_list)

# REQUIREMENTS:
# * SS-17
func _ready():
	_add_project_credits(_load_credit_file(core_credits_path))

# REQUIREMENTS:
# * UI-38
# * UIFUN-37
func _on_back_button_pressed() -> void:
	back_button_pressed.emit()
