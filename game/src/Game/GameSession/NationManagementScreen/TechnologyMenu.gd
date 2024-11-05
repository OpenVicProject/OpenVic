extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.TECHNOLOGY

var _tech_defines : Dictionary = MenuSingleton.get_technology_menu_defines()

var _tech_folder_buttons : Array
var _tech_folder_progressbars : Array
var _tech_folder_number_discovered_labels : Array
var _selected_folder = 0

var _tech_school : GUILabel
var _tech_school_modifiers : GUIOverlappingElementsBox
var _current_research_label : GUILabel
var _current_research_cat_label : GUILabel
var _current_research_progressbar : GUIProgressBar

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_technology", "country_technology")

	_tech_school = get_gui_label_from_nodepath(^"./country_technology/administration_type")
	_tech_school_modifiers = get_gui_overlapping_elements_box_from_nodepath(^"./country_technology/school_bonus_icons")
	if _tech_school_modifiers:
		_tech_school_modifiers.set_gui_child_element_name("country_technology", "school_icon_window")

	_current_research_label = get_gui_label_from_nodepath(^"./country_technology/research_progress_name")
	_current_research_cat_label = get_gui_label_from_nodepath(^"./country_technology/research_progress_category")
	_current_research_progressbar = get_gui_progress_bar_from_nodepath(^"./country_technology/research_progress")

	var tech_folders : Array = _tech_defines.get("tech_folders")
	var tech_areas : Array = _tech_defines.get("tech_areas")
	for i in range(tech_folders.size()):
		add_gui_element("country_technology", "folder_window")

		var folder_node = get_node(^"./folder_window")
		var root_node = get_node(^"./country_technology")

		folder_node.reparent(root_node)
		folder_node.name = tech_folders[i] + "_folder"

		var pos = GUINode.get_gui_position("country_technology", "folder_offset")
		pos.x += folder_node.get_size().x * i
		folder_node.set_position(pos)

		var icon = GUINode.get_gui_icon_from_node(folder_node.get_node(^"./folder_icon"))
		if icon:
			icon.set_icon_index(i+1)

		var title = GUINode.get_gui_label_from_node(folder_node.get_node(^"./folder_category"))
		if title:
			title.set_text(tr(tech_folders[i]))

		var button = GUINode.get_gui_icon_button_from_node(folder_node.get_node(^"./folder_button"))
		var button_tooltip: String = tr("TECHNOLOGYVIEW_SHOW_FOLDER_TOOLTIP")
		var button_dict: Dictionary = { "FOLDER" : tech_folders[i] }
		if button:
			if i == 0:
				button.set_icon_index(2)
			button.pressed.connect(
				func() -> void:
					_tech_folder_buttons[_selected_folder].set_icon_index(1)
					for x in range(tech_areas[_selected_folder].size()):
						root_node.get_node("./" + tech_areas[_selected_folder][x]).visible = false
					_selected_folder = i
					button.set_icon_index(2)
					for x in range(tech_areas[_selected_folder].size()):
						root_node.get_node("./" + tech_areas[_selected_folder][x]).visible = true
			)
			button.set_tooltip_string_and_substitution_dict(button_tooltip, button_dict)
			_tech_folder_buttons.push_back(button)

		var progressbar = GUINode.get_gui_progress_bar_from_node(folder_node.get_node(^"./folder_progress"))
		if progressbar:
			progressbar.mouse_filter = Control.MOUSE_FILTER_IGNORE
			_tech_folder_progressbars.push_back(progressbar)

		var discovered = GUINode.get_gui_label_from_node(folder_node.get_node(^"folder_number_discovered"))
		if discovered:
			_tech_folder_number_discovered_labels.push_back(discovered)

		var folder_areas : Array = tech_areas[i]
		for area_index in range(folder_areas.size()):
			add_gui_element("country_technology", "tech_group")

			var area_node = get_node(^"./tech_group")

			area_node.reparent(root_node)
			area_node.name = tech_areas[i][area_index]
			if i != 0:
				area_node.set_visible(false)

			pos = GUINode.get_gui_position("country_technology", "tech_group_offset")
			pos.x += area_node.get_size().x * area_index
			area_node.set_position(pos)

			var area_title = GUINode.get_gui_label_from_node(area_node.get_node(^"./group_name"))
			if area_title:
				area_title.set_text(tr(tech_areas[i][area_index]))
			
	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_technology/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

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

		if _tech_school:
			_tech_school.set_text(info.get("tech_school"))

		if _tech_school_modifiers:
			var mod_values : Array = info.get("tech_school_mod_values")
			var mod_icons : Array = info.get("tech_school_mod_icons")
			var mod_tooltips : Array = info.get("tech_school_mod_tt")
			var mod_count = mod_values.size()
			_tech_school_modifiers.set_child_count(mod_count)
			for i in range(mod_count):
				var main_icon = get_gui_icon_from_nodepath("./country_technology/school_bonus_icons/school_icon_window_{x}/main_icon".format({"x": i}))
				main_icon.set_icon_index(mod_icons[i])
				main_icon.mouse_filter = Control.MOUSE_FILTER_PASS
				main_icon.set_tooltip_string(mod_tooltips[i])
				var plusminus_icon = get_gui_icon_from_nodepath("./country_technology/school_bonus_icons/school_icon_window_{x}/plusminus_icon".format({"x": i}))
				plusminus_icon.set_icon_index(2 if mod_values[i] > 0 else 1)
				plusminus_icon.mouse_filter = Control.MOUSE_FILTER_PASS
				plusminus_icon.set_tooltip_string(mod_tooltips[i])

		if _current_research_label:
			_current_research_label.set_text(info.get("current_research_tech"))
		
		if _current_research_cat_label:
			_current_research_cat_label.set_text(info.get("current_research_cat"))

		show()
	else:
		hide()
