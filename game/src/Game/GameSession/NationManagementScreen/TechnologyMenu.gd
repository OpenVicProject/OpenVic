extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.TECHNOLOGY

var _tech_school : GUILabel
var _tech_school_modifiers : GUIOverlappingElementsBox

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_technology", "country_technology")

	set_click_mask_from_nodepaths([^"./country_technology/main_bg"])

	_tech_school = get_gui_label_from_nodepath(^"./country_technology/administration_type")
	_tech_school_modifiers = get_gui_overlapping_elements_box_from_nodepath(^"./country_technology/school_bonus_icons")
	if _tech_school_modifiers:
		_tech_school_modifiers.set_gui_child_element_name("country_technology", "school_icon_window")

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
			var mod_names : Array = info.get("tech_school_mod_names")
			var mod_values : Array = info.get("tech_school_mod_values")
			var mod_icons : Array = info.get("tech_school_mod_icons")
			var mod_count = mod_names.size()
			_tech_school_modifiers.set_child_count(mod_count)
			for i in range(mod_count):
				get_gui_icon_from_nodepath("./country_technology/school_bonus_icons/school_icon_window_{x}/main_icon".format({"x": i})).set_icon_index(mod_icons[i])
				get_gui_icon_from_nodepath("./country_technology/school_bonus_icons/school_icon_window_{x}/plusminus_icon".format({"x": i})).set_icon_index(2 if mod_values[i] > 0 else 1)

		show()
	else:
		hide()
