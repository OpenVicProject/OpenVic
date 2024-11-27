extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.POLITICS

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_politics", "country_politics")

	set_click_mask_from_nodepaths([^"./country_politics/main_bg"])

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_politics/close_button")
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
		# TODO - update UI state
		show()
	else:
		hide()
