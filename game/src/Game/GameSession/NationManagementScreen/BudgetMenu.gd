extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.BUDGET

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_budget", "country_budget")

	var close_button : Button = get_button_from_nodepath(^"./country_budget/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	# Scrollbar test code
	var test_scrollbar : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/tax_0_slider")
	var test_label : Label = get_label_from_nodepath(^"./country_budget/tax_0_inc")
	test_scrollbar.value_changed.connect(func(value : int) -> void: test_label.text = str(value))
	test_scrollbar.set_range_limits(20, 80)
	test_scrollbar.emit_value_changed()

	var tariff_scrollbar : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/tariff_slider")
	var tariff_label : Label = get_label_from_nodepath(^"./country_budget/tariffs_percent")
	tariff_scrollbar.value_changed.connect(func(value : int) -> void: tariff_label.text = "%s%%" % GUINode.float_to_formatted_string(value, 1))
	tariff_scrollbar.set_limits(-100, 100)
	tariff_scrollbar.set_range_limits(-45, 80)
	tariff_scrollbar.emit_value_changed()

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
