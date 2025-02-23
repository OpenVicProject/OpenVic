extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.TECHNOLOGY

var _tech_defines : Dictionary = MenuSingleton.get_technology_menu_defines()

var _tech_folders : PackedStringArray
var _tech_areas : Array[PackedStringArray]
var _technologies : Array
var _folder_tech_counts : PackedInt32Array

var _tech_folder_buttons : Array[GUIIconButton]
var _tech_folder_progressbars : Array[GUIProgressBar]
var _tech_folder_number_discovered_labels : Array[GUILabel]

var _tech_school : GUILabel
var _tech_school_modifiers : GUIOverlappingElementsBox
var _current_research_label : GUILabel
var _current_research_cat_label : GUILabel
var _current_research_progressbar : GUIProgressBar

var _selected_tech_picture : GUIIcon
var _selected_tech_label : GUILabel
var _selected_tech_effects : GUILabel
var _selected_tech_research_points : GUILabel
var _selected_tech_unlock_year : GUILabel

var _start_research_button : GUIIconButton

var _selected_folder = 0
var _selected_technology : String
var _selected_technology_info : Dictionary

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_technology", "country_technology")

	set_click_mask_from_nodepaths([^"./country_technology/main_bg"])

	# TECHNOLOGY SCHOOLS
	_tech_school = get_gui_label_from_nodepath(^"./country_technology/administration_type")
	_tech_school_modifiers = get_gui_overlapping_elements_box_from_nodepath(^"./country_technology/school_bonus_icons")
	if _tech_school_modifiers:
		_tech_school_modifiers.set_gui_child_element_name("country_technology", "school_icon_window")

	# CURRENT RESEARCH PROGRESSBAR
	_current_research_label = get_gui_label_from_nodepath(^"./country_technology/research_progress_name")
	_current_research_cat_label = get_gui_label_from_nodepath(^"./country_technology/research_progress_category")
	_current_research_progressbar = get_gui_progress_bar_from_nodepath(^"./country_technology/research_progress")

	# FOLDERS, AREAS, TECHNOLOGIES
	const tech_folders_key : StringName = &"tech_folders"
	const tech_areas_key : StringName = &"tech_areas"
	const technologies_key : StringName = &"technologies"
	const folder_tech_count_key : StringName = &"folder_tech_count"

	_tech_folders = _tech_defines.get(tech_folders_key, [] as PackedStringArray)
	_tech_areas = _tech_defines.get(tech_areas_key, [] as Array[PackedStringArray])
	_technologies = _tech_defines.get(technologies_key, [])
	if _technologies.size() > 0 and _technologies[0].size() > 0 and _technologies[0][0].size() > 0 and _technologies[0][0][0]:
		_selected_technology = _technologies[0][0][0]
		_selected_technology_info = MenuSingleton.get_specific_technology_info(_selected_technology)
	_folder_tech_counts = _tech_defines.get(folder_tech_count_key, [] as PackedInt32Array)

	var root_node : Node = get_node(^"./country_technology")
	var tech_folder_pos : Vector2 = GUINode.get_gui_position("country_technology", "folder_offset")
	var tech_group_pos_base : Vector2 = GUINode.get_gui_position("country_technology", "tech_group_offset")
	var tech_pos_base : Vector2 = GUINode.get_gui_position("country_technology", "tech_offset")

	for i : int in _tech_folders.size():
		add_gui_element("country_technology", "folder_window")

		var folder_node : Node = get_node(^"./folder_window")

		if not folder_node:
			continue

		folder_node.reparent(root_node)
		folder_node.name = _tech_folders[i] + "_folder"
		folder_node.set_position(tech_folder_pos)
		tech_folder_pos.x += folder_node.get_size().x

		var icon : GUIIcon = GUINode.get_gui_icon_from_node_and_path(folder_node, ^"./folder_icon")
		if icon:
			icon.set_icon_index(i+1)

		var title : GUILabel = GUINode.get_gui_label_from_node_and_path(folder_node, ^"./folder_category")
		if title:
			title.set_text(_tech_folders[i])

		var button : GUIIconButton = GUINode.get_gui_icon_button_from_node_and_path(folder_node, ^"./folder_button")
		if button:
			if i == 0:
				button.set_icon_index(2)
			button.pressed.connect( # change selected technology area
				func() -> void:
					_tech_folder_buttons[_selected_folder].set_icon_index(1)
					for x : int in range(_tech_areas[_selected_folder].size()):
						root_node.get_node("./" + _tech_areas[_selected_folder][x]).visible = false
					_selected_folder = i
					button.set_icon_index(2)
					for x : int in range(_tech_areas[_selected_folder].size()):
						root_node.get_node("./" + _tech_areas[_selected_folder][x]).visible = true
			)
			button.set_tooltip_string_and_substitution_dict("TECHNOLOGYVIEW_SHOW_FOLDER_TOOLTIP", { "FOLDER" : _tech_folders[i] })
			_tech_folder_buttons.push_back(button)

		var progressbar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node_and_path(folder_node, ^"./folder_progress")
		if progressbar:
			progressbar.mouse_filter = Control.MOUSE_FILTER_IGNORE
			_tech_folder_progressbars.push_back(progressbar)

		var discovered : GUILabel = GUINode.get_gui_label_from_node_and_path(folder_node, ^"folder_number_discovered")
		if discovered:
			_tech_folder_number_discovered_labels.push_back(discovered)

		# areas
		var folder_areas : PackedStringArray = _tech_areas[i]
		var tech_group_pos : Vector2 = tech_group_pos_base
		for area_index : int in folder_areas.size():
			add_gui_element("country_technology", "tech_group")

			var area_node : Node = get_node(^"./tech_group")

			area_node.reparent(root_node)
			area_node.name = folder_areas[area_index]
			if i != 0:
				area_node.set_visible(false)

			area_node.set_position(tech_group_pos)
			tech_group_pos.x += area_node.get_size().x

			var area_title : GUILabel = GUINode.get_gui_label_from_node_and_path(area_node, ^"./group_name")
			if area_title:
				area_title.set_text(folder_areas[area_index])

			# technologies
			var area_technologies : PackedStringArray = _technologies[i][area_index]
			var tech_pos : Vector2 = tech_pos_base
			for tech_index : int in range(area_technologies.size()):
				add_gui_element("country_technology", "tech_window")

				var tech_node = get_node(^"./tech_window")

				tech_node.reparent(area_node)
				tech_node.name = area_technologies[tech_index]

				tech_node.set_position(tech_pos)
				tech_pos.y += tech_node.get_size().y

				var tech_name : GUILabel = GUINode.get_gui_label_from_node_and_path(tech_node, ^"./tech_name")
				if tech_name:
					tech_name.set_text(area_technologies[tech_index])

				var tech_button : GUIIconButton = GUINode.get_gui_icon_button_from_node_and_path(tech_node, ^"./start_research")
				if tech_button:
					tech_button.pressed.connect(_on_tech_selected.bind(area_technologies[tech_index]))

	# SELECTED TECH WINDOW
	_selected_tech_picture = get_gui_icon_from_nodepath(^"./country_technology/selected_tech_window/picture")
	_selected_tech_label = get_gui_label_from_nodepath(^"./country_technology/selected_tech_window/title")
	_selected_tech_effects = get_gui_label_from_nodepath(^"./country_technology/selected_tech_window/effect")
	_selected_tech_research_points = get_gui_label_from_nodepath(^"./country_technology/selected_tech_window/diff")
	_selected_tech_unlock_year = get_gui_label_from_nodepath(^"./country_technology/selected_tech_window/year")
	_start_research_button = get_gui_icon_button_from_nodepath(^"./country_technology/selected_tech_window/start")
	if _start_research_button:
		_start_research_button.pressed.connect(
			func() -> void:
				PlayerSingleton.start_research(_selected_technology)
				print("Started Research of " + _selected_technology)
				_update_info()
				get_parent()._update_info()
		)

	# CLOSE BUTTON
	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_technology/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	_update_info()

func _on_tech_selected(technology_id : String) -> void:
	if _selected_technology != technology_id:
		_selected_technology = technology_id
		_selected_technology_info = MenuSingleton.get_specific_technology_info(_selected_technology)
		_update_info()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()

func _on_update_active_nation_management_screen(active_screen : NationManagement.Screen) -> void:
	_active = active_screen == _screen
	_update_info()

func _update_info() -> void:
	if _active:
		var info : Dictionary = MenuSingleton.get_technology_menu_info()

		const tech_school_key : StringName = &"tech_school"
		const tech_school_mod_values_key : StringName = &"tech_school_mod_values"
		const tech_school_mod_icons_key : StringName = &"tech_school_mod_icons"
		const tech_school_mod_tt_key : StringName = &"tech_school_mod_tt"

		if _tech_school:
			_tech_school.set_text(info.get(tech_school_key, ""))

		if _tech_school_modifiers:
			var mod_values : PackedFloat32Array = info.get(tech_school_mod_values_key, PackedFloat32Array())
			var mod_icons : PackedInt32Array = info.get(tech_school_mod_icons_key, PackedInt32Array())
			var mod_tooltips : PackedStringArray = info.get(tech_school_mod_tt_key, PackedStringArray())
			var mod_count : int = mod_values.size()
			_tech_school_modifiers.set_child_count(mod_count)
			for i : int in range(mod_count):
				var main_icon : GUIIcon = get_gui_icon_from_nodepath("./country_technology/school_bonus_icons/school_icon_window_{x}/main_icon".format({"x": i}))
				main_icon.set_icon_index(mod_icons[i])
				main_icon.set_tooltip_string(mod_tooltips[i])
				var plusminus_icon : GUIIcon = get_gui_icon_from_nodepath("./country_technology/school_bonus_icons/school_icon_window_{x}/plusminus_icon".format({"x": i}))
				plusminus_icon.set_icon_index(2 if mod_values[i] > 0 else 1)
				plusminus_icon.set_tooltip_string(mod_tooltips[i])

		const current_research_tech_key : StringName = &"current_research_tech"
		const current_research_cat_key : StringName = &"current_research_cat"
		const current_research_finish_date_key : StringName = &"current_research_finish_date"
		const current_research_invested_key : StringName = &"current_research_invested"
		const current_research_cost_key : StringName = &"current_research_cost"
		const current_research_effect_tt_key : StringName = &"current_research_effect_tt"
		const current_research_progress_key : StringName = &"current_research_progress"

		var current_research : String = info.get(current_research_tech_key, "")
		if _current_research_label and _current_research_label.text != current_research:
			_current_research_label.set_text(current_research if current_research else "TECHNOLOGYVIEW_NO_RESEARCH")

		if _current_research_cat_label:
			_current_research_cat_label.set_text(info.get(current_research_cat_key, ""))

		if _current_research_progressbar:
			if current_research != "":
				_current_research_progressbar.value = info.get(current_research_progress_key, 0)
				_current_research_progressbar.set_tooltip_string_and_substitution_dict(
					tr(&"TECHNOLOGYVIEW_RESEARCH_TOOLTIP") + "\n" +
					tr(&"TECHNOLOGYVIEW_RESEARCH_INVESTED_TOOLTIP") +
					MenuSingleton.get_tooltip_separator() +
					info.get(current_research_effect_tt_key, ""),
					{
						"TECH": current_research,
						"DATE": info.get(current_research_finish_date_key, "1836.1.1"),
						"INVESTED": info.get(current_research_invested_key, 0),
						"COST": info.get(current_research_cost_key, 0)
					}
				)
			else:
				_current_research_progressbar.value = 0
				_current_research_progressbar.set_tooltip_string("TECHNOLOGYVIEW_NO_RESEARCH_TOOLTIP")

		const researched_technologies_key : StringName = &"researched_technologies"
		const researchable_technologies_key : StringName = &"researchable_technologies"

		var researched_techs : PackedStringArray = info.get(researched_technologies_key, PackedStringArray())
		var avail_techs : PackedStringArray = info.get(researchable_technologies_key, PackedStringArray())

		const effect_tooltip_key : StringName = &"effects_tooltip"
		const research_points_key : StringName = &"research_points"
		const start_year_key : StringName = &"start_year"
		const prerequisite_key : StringName = &"prerequisite"

		for ix : int in range(_technologies.size()):
			var folder_number_discovered = 0
			# spellchecker:off (iy is wrongly detected as misspelling of it)
			for iy : int in range(_technologies[ix].size()):
				for iz : int in range(_technologies[ix][iy].size()):
					var tech_identifier : String = _technologies[ix][iy][iz]
					var tech_info : Dictionary = MenuSingleton.get_specific_technology_info(tech_identifier)
					var tech : GUIIconButton = get_gui_icon_button_from_nodepath("./country_technology/{y}/{z}/start_research".format({"y":_tech_areas[ix][iy], "z":tech_identifier}))
					if tech:
						if researched_techs.has(tech_identifier):
							tech.set_icon_index(2)
							folder_number_discovered += 1
						elif current_research == tech_identifier:
							tech.set_icon_index(1)
						elif avail_techs.has(tech_identifier):
							tech.set_icon_index(3)
						else:
							tech.set_icon_index(4)
						tech.set_tooltip_string("§Y{tech}§W\n".format({ "tech": tr(tech_identifier) }) + tech_info.get(effect_tooltip_key, "") + MenuSingleton.get_tooltip_separator() + tr(&"TECH_INVENTIONS_TOOLTIP") + "\nTODO: Inventions")
			# spellchecker:on
			var label : GUILabel = _tech_folder_number_discovered_labels[ix]
			if label:
				label.set_text("{r}/{a}".format({"r":folder_number_discovered,"a":_folder_tech_counts[ix]}))
			var progbar : GUIProgressBar = _tech_folder_progressbars[ix]
			if progbar:
				progbar.value = float(folder_number_discovered) / float(_folder_tech_counts[ix])

		# SELECTED TECHNOLOGY PANEL
		if _selected_tech_label:
			_selected_tech_label.set_text(_selected_technology)
		if _selected_tech_picture and _selected_technology:
			_selected_tech_picture.set_texture(AssetManager.get_texture("gfx/pictures/tech/{a}.tga".format({ "a": _selected_technology })))
		if _selected_tech_effects:
			_selected_tech_effects.set_text(_selected_technology_info.get(effect_tooltip_key, ""))
		if _selected_tech_research_points:
			_selected_tech_research_points.set_text(str(_selected_technology_info.get(research_points_key, 0)))
		var start_year_string : String = str(_selected_technology_info.get(start_year_key, 0))
		if _selected_tech_unlock_year:
			_selected_tech_unlock_year.set_text(start_year_string)
		if _start_research_button:
			if current_research == _selected_technology:
				_start_research_button.disabled = true
				_start_research_button.set_tooltip_string_and_substitution_dict("TECHNOLOGYVIEW_UNDER_RESEARCH_TOOLTIP", { "TECH": _selected_technology })
			elif researched_techs.has(_selected_technology):
				_start_research_button.disabled = true
				_start_research_button.set_tooltip_string_and_substitution_dict("TECHNOLOGYVIEW_ALREADY_RESEARCHED_TOOLTIP", { "TECH": _selected_technology })
			elif avail_techs.has(_selected_technology):
				_start_research_button.disabled = false
				_start_research_button.set_tooltip_string_and_substitution_dict("TECHNOLOGYVIEW_START_RESEARCH_TOOLTIP", { "TECH": _selected_technology })
			else:
				var prerequisite : String = _selected_technology_info.get(prerequisite_key, "")
				_start_research_button.disabled = true
				_start_research_button.set_tooltip_string_and_substitution_dict(
					tr(&"TECHNOLOGYVIEW_CANNOT_RESEARCH_TOOLTIP") + "\n" +
					tr(&"TECHNOLOGYVIEW_HAVE_NOT_YEAR") + "\n" +
					(MenuSingleton.get_tooltip_condition_met() if researched_techs.has(prerequisite) else MenuSingleton.get_tooltip_condition_unmet()) +
					" " + tr(&"TECHNOLOGYVIEW_HAVE_DISCOVERED_PREREQ_TOOLTIP").replace("$TECH$", "$PREREQ$"),
					{
						"TECH": _selected_technology,
						"YEAR": start_year_string,
						"PREREQ": prerequisite
					}
				)

		show()
	else:
		hide()
