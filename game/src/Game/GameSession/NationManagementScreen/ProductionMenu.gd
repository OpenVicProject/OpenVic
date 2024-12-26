extends GUINode

var _active: bool = false

const _screen: NationManagement.Screen = NationManagement.Screen.PRODUCTION


func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(
		_on_update_active_nation_management_screen
	)

	add_gui_element("country_production", "country_production")

	var project_listbox: GUIListBox = get_gui_listbox_from_nodepath(
		^"./country_production/good_production/project_listbox"
	)
	if project_listbox:
		project_listbox.mouse_filter = Control.MOUSE_FILTER_IGNORE
	set_click_mask_from_nodepaths([^"./country_production/main_bg"])

	var close_button: GUIIconButton = get_gui_icon_button_from_nodepath(
		^"./country_production/close_button"
	)
	if close_button:
		close_button.pressed.connect(
			Events.NationManagementScreens.close_nation_management_screen.bind(_screen)
		)

	_update_info()


func _notification(what: int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()


func _on_update_active_nation_management_screen(active_screen: NationManagement.Screen) -> void:
	_active = active_screen == _screen
	_update_info()


func _update_info() -> void:
	if _active:
		# TODO - update UI state
		show()
	else:
		hide()
