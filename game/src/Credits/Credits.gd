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

# TODO: implement for theme instead
# waiting for https://github.com/OpenVic2Project/OpenVic2/pull/48
@export_group("Label Settings", "label_settings_")
@export
var label_settings_project : LabelSettings

@export
var label_settings_role : LabelSettings

@export
var label_settings_personel : LabelSettings

@export
var credits_list: VBoxContainer

# REQUIREMENTS:
# * 1.5 Credits Menu
# * SS-17

# REQUIREMENTS
# * FS-4
func load_credit_file(path):
	var roles = {}
	var core_credits = FileAccess.open(path, FileAccess.READ)

	while not core_credits.eof_reached():
		var line = core_credits.get_csv_line()

		# If the line does not have an identifiable role or is empty then skip it
		if line[0].is_empty():
			continue

		var role := line[0].to_upper()
		var person : String = line[1] # Cannot infer with := as line[1] does not have a set type

		if role not in roles.keys():
			roles[role] = []
			roles[role].push_back(person)
		else:
			roles[role].push_back(person)

	return roles

# REQUIREMENTS:
# * UI-34, UI-35
func make_project_credits(project):
	var project_credits_list = VBoxContainer.new()
	var project_credits_label = Label.new()


	# Spartan has some suggestions here but for now its good enough. Refer
	# to PR 16 resolved comments for further details
	project_credits_list.name = 'Credits' + project['TITLE'][0]
	project_credits_label.name = 'Label' + project['TITLE'][0]
	project_credits_label.text = project['TITLE'][0]
	project_credits_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	project_credits_label.label_settings = label_settings_project

	for role in project.keys():
		if role == 'TITLE':
			continue

		var role_parent = VBoxContainer.new()
		var role_label = Label.new()

		role_parent.name = 'Role' + role
		role_label.name = 'Label' + role
		role_label.text = role
		role_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		role_label.label_settings = label_settings_role

		for person in project[role]:
			var person_label = Label.new()

			person_label.name = 'Label' + person
			person_label.text = person
			person_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
			person_label.label_settings = label_settings_personel
			role_parent.add_child(person_label)

		project_credits_list.add_child(role_label)
		project_credits_list.add_child(role_parent)
		project_credits_list.add_child(HSeparator.new())

	credits_list.add_child(project_credits_list)

# REQUIREMENTS:
# * SS-17
func _ready():
	var core_credits = load_credit_file(core_credits_path)
	make_project_credits(core_credits)

# REQUIREMENTS:
# * UI-38
# * UIFUN-37
func _on_back_button_pressed():
	back_button_pressed.emit()
