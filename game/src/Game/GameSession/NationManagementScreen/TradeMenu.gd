extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.TRADE

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_trade", "country_trade")

	set_click_mask_from_nodepaths([^"./country_trade/main_bg"])

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	var good_price_line_chart : GUILineChart = get_gui_line_chart_from_nodepath(^"./country_trade/trade_details/price_linechart")

	if good_price_line_chart:
		# TEST COLOURED LINES
		var colours : PackedColorArray = [
			Color.RED, Color.GREEN, Color.BLUE, Color.MAGENTA,
			Color.CYAN, Color.ORANGE, Color.CRIMSON, Color.FOREST_GREEN
		]
		for n : int in colours.size():
			const point_count : int = 36
			var values : PackedFloat32Array
			for x : int in point_count:
				values.push_back(1000 * sin((float(x) / (point_count - 1) + float(n) / (colours.size() * 2)) * 4 * PI) + 4000)
			good_price_line_chart.add_coloured_line(values, colours[n])
		good_price_line_chart.scale_coloured_lines()

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
