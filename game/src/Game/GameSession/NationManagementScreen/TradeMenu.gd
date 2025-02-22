extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.TRADE

const _gui_file : String = "country_trade"

var _trade_detail_good_index : int = 0

# Trade details
var _trade_detail_good_icon : GUIIcon
var _trade_detail_good_name_label : GUILabel
var _trade_detail_good_price_label : GUILabel
var _trade_detail_good_price_line_chart : GUILineChart
var _trade_detail_good_price_low_label : GUILabel
var _trade_detail_good_price_high_label : GUILabel
var _trade_detail_good_chart_time_label : GUILabel
var _trade_detail_automate_checkbox : GUIIconButton
var _trade_detail_buy_sell_stockpile_checkbox : GUIIconButton
var _trade_detail_buy_sell_stockpile_label : GUILabel
var _trade_detail_stockpile_slider_description_label : GUILabel
var _trade_detail_stockpile_slider_scrollbar : GUIScrollbar
var _trade_detail_stockpile_slider_amount_label : GUILabel
var _trade_detail_confirm_trade_button : GUIIconButton
var _trade_detail_government_good_needs_label : GUILabel
var _trade_detail_factory_good_needs_label : GUILabel
var _trade_detail_pop_good_needs_label : GUILabel
var _trade_detail_good_available_label : GUILabel

# Goods tables
enum Table {
	GOVERNMENT_NEEDS,
	FACTORY_NEEDS,
	POP_NEEDS,
	MARKET_ACTIVITY,
	STOCKPILE,
	COMMON_MARKET
}
const TABLE_NAMES : PackedStringArray = [
	"government_needs", "factory_needs", "pop_needs", "market_activity", "stockpile", "common_market"
]
const TABLE_ENTRY_NAMES : PackedStringArray = [
	"goods_needs_entry", "goods_needs_entry", "goods_needs_entry", "market_activity_entry", "stockpile_entry", "common_market_entry"
]
# Nested Array contains only NodePaths
const TABLE_ITEM_PATHS : Array[Array] = [
	[^"./goods_type", ^"./value"], [^"./goods_type", ^"./value"], [^"./goods_type", ^"./value"],
	[^"./goods_type", ^"./activity", ^"./cost"],
	[^"./goods_type", ^"./value", ^"./change"],
	[^"./goods_type", ^"./total", ^"./produce_change", ^"./exported"]
]

var _table_listboxes : Array[GUIListBox]

const TABLE_UNSORTED : int = 255
const TABLE_COLUMN_KEYS : Array[StringName] = [&"COLUMN_0", &"COLUMN_1", &"COLUMN_2", &"COLUMN_3"]
var _table_sort_columns : PackedByteArray

const SORT_DESCENDING : int = 0
const SORT_ASCENDING : int = 1
var _table_sort_directions : PackedByteArray

# Good entries
var _goods_entry_offset : Vector2

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element(_gui_file, "country_trade")

	set_click_mask_from_nodepaths([^"./country_trade/main_bg"])

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	# Goods tables
	_table_listboxes.push_back(get_gui_listbox_from_nodepath(^"./country_trade/government_needs_list"))
	_table_sort_columns.push_back(TABLE_UNSORTED)
	_table_sort_directions.push_back(SORT_DESCENDING)
	var sort_government_needs_by_good_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/needs_government_sort_by_goods")
	if sort_government_needs_by_good_button:
		sort_government_needs_by_good_button.pressed.connect(_change_table_sorting.bind(Table.GOVERNMENT_NEEDS, 0))
	var sort_government_needs_by_need_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/needs_government_sort_by_value")
	if sort_government_needs_by_need_button:
		sort_government_needs_by_need_button.pressed.connect(_change_table_sorting.bind(Table.GOVERNMENT_NEEDS, 1))

	_table_listboxes.push_back(get_gui_listbox_from_nodepath(^"./country_trade/factory_needs_list"))
	_table_sort_columns.push_back(TABLE_UNSORTED)
	_table_sort_directions.push_back(SORT_DESCENDING)
	var sort_factory_needs_by_good_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/needs_factories_sort_by_goods")
	if sort_factory_needs_by_good_button:
		sort_factory_needs_by_good_button.pressed.connect(_change_table_sorting.bind(Table.FACTORY_NEEDS, 0))
	var sort_factory_needs_by_need_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/needs_factories_sort_by_value")
	if sort_factory_needs_by_need_button:
		sort_factory_needs_by_need_button.pressed.connect(_change_table_sorting.bind(Table.FACTORY_NEEDS, 1))

	_table_listboxes.push_back(get_gui_listbox_from_nodepath(^"./country_trade/pop_needs_list"))
	_table_sort_columns.push_back(TABLE_UNSORTED)
	_table_sort_directions.push_back(SORT_DESCENDING)
	var sort_pop_needs_by_good_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/needs_pops_sort_by_goods")
	if sort_pop_needs_by_good_button:
		sort_pop_needs_by_good_button.pressed.connect(_change_table_sorting.bind(Table.POP_NEEDS, 0))
	var sort_pop_needs_by_need_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/needs_pops_sort_by_value")
	if sort_pop_needs_by_need_button:
		sort_pop_needs_by_need_button.pressed.connect(_change_table_sorting.bind(Table.POP_NEEDS, 1))

	_table_listboxes.push_back(get_gui_listbox_from_nodepath(^"./country_trade/market_activity_list"))
	_table_sort_columns.push_back(TABLE_UNSORTED)
	_table_sort_directions.push_back(SORT_DESCENDING)
	var sort_market_activity_by_good_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/market_activity_sort_by_goods")
	if sort_market_activity_by_good_button:
		sort_market_activity_by_good_button.pressed.connect(_change_table_sorting.bind(Table.MARKET_ACTIVITY, 0))
	var sort_market_activity_by_activity_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/market_activity_sort_by_activity")
	if sort_market_activity_by_activity_button:
		sort_market_activity_by_activity_button.pressed.connect(_change_table_sorting.bind(Table.MARKET_ACTIVITY, 1))
	var sort_market_activity_by_cost_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/market_activity_sort_by_cost")
	if sort_market_activity_by_cost_button:
		sort_market_activity_by_cost_button.pressed.connect(_change_table_sorting.bind(Table.MARKET_ACTIVITY, 2))

	_table_listboxes.push_back(get_gui_listbox_from_nodepath(^"./country_trade/stockpile_list"))
	_table_sort_columns.push_back(TABLE_UNSORTED)
	_table_sort_directions.push_back(SORT_DESCENDING)
	var sort_stockpile_by_good_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/stockpile_sort_by_goods")
	if sort_stockpile_by_good_button:
		sort_stockpile_by_good_button.pressed.connect(_change_table_sorting.bind(Table.STOCKPILE, 0))
	var sort_stockpile_by_stock_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/stockpile_sort_by_value")
	if sort_stockpile_by_stock_button:
		sort_stockpile_by_stock_button.pressed.connect(_change_table_sorting.bind(Table.STOCKPILE, 1))
	var sort_stockpile_by_increase_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/stockpile_sort_by_change")
	if sort_stockpile_by_increase_button:
		sort_stockpile_by_increase_button.pressed.connect(_change_table_sorting.bind(Table.STOCKPILE, 2))

	_table_listboxes.push_back(get_gui_listbox_from_nodepath(^"./country_trade/common_market_list"))
	_table_sort_columns.push_back(TABLE_UNSORTED)
	_table_sort_directions.push_back(SORT_DESCENDING)
	var sort_common_market_by_good_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/common_market_sort_by_goods")
	if sort_common_market_by_good_button:
		sort_common_market_by_good_button.pressed.connect(_change_table_sorting.bind(Table.COMMON_MARKET, 0))
	var sort_common_market_by_available_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/common_market_sort_by_produced")
	if sort_common_market_by_available_button:
		sort_common_market_by_available_button.pressed.connect(_change_table_sorting.bind(Table.COMMON_MARKET, 1))
	var sort_common_market_by_increase_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/common_market_sort_by_diff")
	if sort_common_market_by_increase_button:
		sort_common_market_by_increase_button.pressed.connect(_change_table_sorting.bind(Table.COMMON_MARKET, 2))
	var sort_common_market_by_exported_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/common_market_sort_by_exported")
	if sort_common_market_by_exported_button:
		sort_common_market_by_exported_button.pressed.connect(_change_table_sorting.bind(Table.COMMON_MARKET, 3))

	# Trade details
	_trade_detail_good_icon = get_gui_icon_from_nodepath(^"./country_trade/trade_details/goods_icon")
	_trade_detail_good_name_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/goods_title")
	_trade_detail_good_price_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/goods_price")
	if _trade_detail_good_price_label:
		_trade_detail_good_price_label.set_auto_translate(false)
	_trade_detail_good_price_line_chart = get_gui_line_chart_from_nodepath(^"./country_trade/trade_details/price_linechart")
	_trade_detail_good_price_low_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/price_chart_low")
	if _trade_detail_good_price_low_label:
		_trade_detail_good_price_low_label.set_auto_translate(false)
	_trade_detail_good_price_high_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/price_chart_high")
	if _trade_detail_good_price_high_label:
		_trade_detail_good_price_high_label.set_auto_translate(false)
	_trade_detail_good_chart_time_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/price_chart_time")
	if _trade_detail_good_chart_time_label:
		_trade_detail_good_chart_time_label.set_text("PRICE_HISTORY_TIME_RANGE")
	var trade_detail_automate_label : GUILabel = get_gui_label_from_nodepath(^"./country_trade/trade_details/automate_label")
	if trade_detail_automate_label:
		trade_detail_automate_label.set_tooltip_string("AUTOMATE_TRADE_CHECK")
	_trade_detail_automate_checkbox = get_gui_icon_button_from_nodepath(^"./country_trade/trade_details/automate")
	if _trade_detail_automate_checkbox:
		_trade_detail_automate_checkbox.set_tooltip_string("AUTOMATE_TRADE_CHECK")
	_trade_detail_buy_sell_stockpile_checkbox = get_gui_icon_button_from_nodepath(^"./country_trade/trade_details/sell_stockpile")
	if _trade_detail_buy_sell_stockpile_checkbox:
		_trade_detail_buy_sell_stockpile_checkbox.toggled.connect(_update_trade_order_buy_sell)
	_trade_detail_buy_sell_stockpile_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/sell_stockpile_label")
	_trade_detail_stockpile_slider_description_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/sell_slidier_desc")
	_trade_detail_stockpile_slider_scrollbar = get_gui_scrollbar_from_nodepath(^"./country_trade/trade_details/sell_slider")
	_trade_detail_stockpile_slider_amount_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/slider_value")
	if _trade_detail_stockpile_slider_amount_label:
		_trade_detail_stockpile_slider_amount_label.set_auto_translate(false)
		if _trade_detail_stockpile_slider_scrollbar:
			_trade_detail_stockpile_slider_scrollbar.value_changed.connect(
				func(value : int) -> void:
					_update_stockpile_slider_amount_label(MenuSingleton.calculate_trade_menu_stockpile_cutoff_amount(_trade_detail_stockpile_slider_scrollbar))
			)
	_trade_detail_confirm_trade_button = get_gui_icon_button_from_nodepath(^"./country_trade/trade_details/confirm_trade")
	if _trade_detail_confirm_trade_button:
		_trade_detail_confirm_trade_button.pressed.connect(
			func() -> void:
				# TODO - implement button functionality
				print("Confirm trade!")
		)
	var good_details_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_trade/trade_details/goods_details")
	if good_details_button:
		good_details_button.pressed.connect(
			func() -> void:
				# TODO - open trade details menu
				print("Open details menu for good index ", _trade_detail_good_index)
		)

	_trade_detail_government_good_needs_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/goods_need_gov_desc")
	if _trade_detail_government_good_needs_label:
		_trade_detail_government_good_needs_label.set_text("GOV_NEED_DETAIL")
	_trade_detail_factory_good_needs_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/goods_need_factory_desc")
	if _trade_detail_factory_good_needs_label:
		_trade_detail_factory_good_needs_label.set_text("FACTORY_NEED_DETAIL")
	_trade_detail_pop_good_needs_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/produced_detail_desc") # names switched in GUI file
	if _trade_detail_pop_good_needs_label:
		_trade_detail_pop_good_needs_label.set_text("POP_NEED_DETAIL")
	_trade_detail_good_available_label = get_gui_label_from_nodepath(^"./country_trade/trade_details/goods_need_pop_desc") # names switched in GUI file
	if _trade_detail_good_available_label:
		_trade_detail_good_available_label.set_text("PRODUCED_DETAIL_REMOVE")

	# Good entries
	_goods_entry_offset = get_gui_position(_gui_file, "goods_entry_offset")
	if _goods_entry_offset.x == 0.0:
		push_error("TradeMenu: Goods entry x offset is zero, setting to 1 to avoid divide by 0 errors")
		_goods_entry_offset.x = 1.0

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

		_update_trade_details()

		const good_producers_tooltips_key : StringName = &"good_producers_tooltips"
		const good_trading_yesterday_tooltips_key : StringName = &"good_trading_yesterday_tooltips"
		const government_needs_key : StringName = &"government_needs"
		const factory_needs_key : StringName = &"factory_needs"
		const pop_needs_key : StringName = &"pop_needs"
		const market_activity_key : StringName = &"market_activity"
		const stockpile_key : StringName = &"stockpile"
		const common_market_key : StringName = &"common_market"

		var trade_info : Dictionary = MenuSingleton.get_trade_menu_tables_info()

		var good_producers_tooltips : PackedStringArray = trade_info.get(good_producers_tooltips_key, [] as PackedStringArray)

		_generate_listbox(Table.GOVERNMENT_NEEDS, trade_info.get(government_needs_key, [] as PackedVector2Array), good_producers_tooltips)
		_generate_listbox(Table.FACTORY_NEEDS, trade_info.get(factory_needs_key, [] as PackedVector2Array), good_producers_tooltips)
		_generate_listbox(Table.POP_NEEDS, trade_info.get(pop_needs_key, [] as PackedVector2Array), good_producers_tooltips)
		_generate_listbox(Table.MARKET_ACTIVITY, trade_info.get(market_activity_key, [] as PackedVector3Array), good_producers_tooltips)
		_generate_listbox(Table.STOCKPILE, trade_info.get(stockpile_key, [] as PackedVector3Array), trade_info.get(good_trading_yesterday_tooltips_key, [] as PackedStringArray))
		_generate_listbox(Table.COMMON_MARKET, trade_info.get(common_market_key, [] as PackedVector4Array), good_producers_tooltips)

		_generate_good_entries(good_producers_tooltips)

		show()
	else:
		hide()

func _update_trade_details(new_trade_detail_good_index : int = -1) -> void:
	# If the new index isn't negative, update the current index to match it
	# Even if the new index is the same as the current index, it indicates a forced refresh (including the trade order
	# buy/sell checkbox and stockpile cutoff slider, which otherwise wouldn't be refreshed)
	var force_refresh : bool = new_trade_detail_good_index >= 0
	if force_refresh:
		_trade_detail_good_index = new_trade_detail_good_index

	# Trade details
	const trade_detail_good_name_key : StringName = &"trade_detail_good_name"
	const trade_detail_good_price_key : StringName = &"trade_detail_good_price"
	const trade_detail_good_base_price_key : StringName = &"trade_detail_good_base_price"
	const trade_detail_price_history_key : StringName = &"trade_detail_price_history"
	const trade_detail_is_automated_key : StringName = &"trade_detail_is_automated"
	const trade_detail_is_selling_key : StringName = &"trade_detail_is_selling" # or buying (false)
	const trade_detail_slider_amount_key : StringName = &"trade_detail_slider_amount" # exponential good amount
	const trade_detail_government_needs_key : StringName = &"trade_detail_government_needs"
	const trade_detail_army_needs_key : StringName = &"trade_detail_army_needs"
	const trade_detail_navy_needs_key : StringName = &"trade_detail_navy_needs"
	const trade_detail_overseas_needs_key : StringName = &"trade_detail_overseas_needs"
	const trade_detail_factory_needs_key : StringName = &"trade_detail_factory_needs"
	const trade_detail_pop_needs_key : StringName = &"trade_detail_pop_needs"
	const trade_detail_available_key : StringName = &"trade_detail_available"

	var trade_info : Dictionary = MenuSingleton.get_trade_menu_trade_details_info(
		_trade_detail_good_index, _trade_detail_stockpile_slider_scrollbar if force_refresh else null
	)

	var trade_detail_good_name : String = trade_info.get(trade_detail_good_name_key, "")

	if _trade_detail_good_icon:
		_trade_detail_good_icon.set_icon_index(_trade_detail_good_index + 2)
		_trade_detail_good_icon.set_tooltip_string(trade_detail_good_name)

	if _trade_detail_good_name_label:
		_trade_detail_good_name_label.set_text(trade_detail_good_name)

	if _trade_detail_good_price_label:
		_trade_detail_good_price_label.set_text(GUINode.float_to_string_dp(trade_info.get(trade_detail_good_price_key, 0), 3) + "¤")

	var price_history : PackedFloat32Array = trade_info.get(trade_detail_price_history_key, [] as PackedFloat32Array)
	if price_history.size() == 1:
		# We cannot draw a line with just one point
		push_error("TradeMenu: Price history has only one point: ", price_history)
		price_history.clear()
	var base_price : float = trade_info.get(trade_detail_good_base_price_key, 0)
	var price_low : float = base_price
	var price_high : float = base_price

	if _trade_detail_good_price_line_chart:
		if price_history.is_empty():
			_trade_detail_good_price_line_chart.clear_lines()
		else:
			_trade_detail_good_price_line_chart.set_gradient_line(price_history, base_price, 1.0)
			price_low = _trade_detail_good_price_line_chart.get_min_value()
			price_high = _trade_detail_good_price_line_chart.get_max_value()

	if _trade_detail_good_price_low_label:
		_trade_detail_good_price_low_label.set_text(GUINode.float_to_string_dp(price_low, 1) + "¤")

	if _trade_detail_good_price_high_label:
		_trade_detail_good_price_high_label.set_text(GUINode.float_to_string_dp(price_high, 1) + "¤")

	if _trade_detail_good_chart_time_label:
		_trade_detail_good_chart_time_label.add_substitution("MONTHS", str(price_history.size()))

	var is_automated : bool = trade_info.get(trade_detail_is_automated_key, false)

	if _trade_detail_automate_checkbox:
		# Investigate whether set_pressed_no_signal can/should be used here
		_trade_detail_automate_checkbox.set_pressed(is_automated)

	if force_refresh:
		_update_trade_order_buy_sell(trade_info.get(trade_detail_is_selling_key, false))

		if _trade_detail_stockpile_slider_amount_label:
			_update_stockpile_slider_amount_label(trade_info.get(trade_detail_slider_amount_key, 0))

	if _trade_detail_confirm_trade_button:
		_trade_detail_confirm_trade_button.set_disabled(is_automated)
		_trade_detail_confirm_trade_button.set_tooltip_string("TRADE_DISABLED_AUTOMATE" if is_automated else "TRADE_CONFIRM_DESC")

	var factory_needs : float = trade_info.get(trade_detail_factory_needs_key, 0)
	var factory_needs_string : String = GUINode.float_to_string_dp(factory_needs, 2)

	if _trade_detail_government_good_needs_label:
		_trade_detail_government_good_needs_label.add_substitution("VAL", GUINode.float_to_string_dp(trade_info.get(trade_detail_government_needs_key, 0), 2))
		var government_needs_tooltip : String
		var army_needs : float = trade_info.get(trade_detail_army_needs_key, 0)
		if army_needs > 0:
			government_needs_tooltip = tr(&"TRADE_SUPPLY_NEED_A").replace("$VAL$", GUINode.float_to_string_dp(army_needs, 2))
		var navy_needs : float = trade_info.get(trade_detail_navy_needs_key, 0)
		if navy_needs > 0:
			government_needs_tooltip += tr(&"TRADE_SUPPLY_NEED_N").replace("$VAL$", GUINode.float_to_string_dp(navy_needs, 2))
		if factory_needs > 0:
			government_needs_tooltip += tr(&"TRADE_TEMP_PROD_NEED").replace("$VAL$", factory_needs_string)
		var overseas_needs : float = trade_info.get(trade_detail_overseas_needs_key, 0)
		if overseas_needs > 0:
			government_needs_tooltip += tr(&"TRADE_OVERSEAS_NEED").replace("$VAL$", GUINode.float_to_string_dp(overseas_needs, 2))
		if not government_needs_tooltip.is_empty():
			government_needs_tooltip = government_needs_tooltip.trim_suffix("\n")
		_trade_detail_government_good_needs_label.set_tooltip_string(government_needs_tooltip)

	if _trade_detail_factory_good_needs_label:
		_trade_detail_factory_good_needs_label.add_substitution("VAL", factory_needs_string)

	if _trade_detail_pop_good_needs_label:
		_trade_detail_pop_good_needs_label.add_substitution("VAL", GUINode.float_to_string_dp(trade_info.get(trade_detail_pop_needs_key, 0), 2))

	if _trade_detail_good_available_label:
		_trade_detail_good_available_label.add_substitution("VAL", GUINode.float_to_string_dp(trade_info.get(trade_detail_available_key, 0), 2))

func _update_trade_order_buy_sell(is_selling : bool) -> void:
	if _trade_detail_buy_sell_stockpile_checkbox:
		# Investigate whether set_pressed_no_signal can/should be used here
		_trade_detail_buy_sell_stockpile_checkbox.set_pressed(is_selling)

	if _trade_detail_buy_sell_stockpile_label:
		_trade_detail_buy_sell_stockpile_label.set_text("SELL" if is_selling else "BUY")

	if _trade_detail_stockpile_slider_description_label:
		_trade_detail_stockpile_slider_description_label.set_text("MINIMUM_STOCKPILE_TARGET" if is_selling else "MAXIMUM_STOCKPILE_TARGET")

func _update_stockpile_slider_amount_label(slider_amount : float) -> void:
	_trade_detail_stockpile_slider_amount_label.set_text(GUINode.float_to_string_dp(slider_amount, 3 if slider_amount < 10.0 else 2))

func _change_table_sorting(table : Table, column : int) -> void:
	if _table_sort_columns[table] != column:
		_table_sort_columns[table] = column
		_table_sort_directions[table] = SORT_DESCENDING
	else:
		_table_sort_directions[table] = SORT_ASCENDING if _table_sort_directions[table] == SORT_DESCENDING else SORT_DESCENDING

	print("Sorting table ", TABLE_NAMES[table], " by column ", column, " ", "ascending" if _table_sort_directions[table] == SORT_ASCENDING else "descending")

	_sort_table(table)

func _sort_table(table : Table) -> void:
	var column : int = _table_sort_columns[table]
	if column == TABLE_UNSORTED:
		return

	var listbox : GUIListBox = _table_listboxes[table]
	var sort_key : StringName = TABLE_COLUMN_KEYS[column]
	var descending : bool = _table_sort_directions[table] == SORT_DESCENDING

	var items : Array[Node] = listbox.get_children()

	for child : Node in items:
		listbox.remove_child(child)

	items.sort_custom(
		(func(a : Node, b : Node) -> bool: return a.get_meta(sort_key) > b.get_meta(sort_key))
		if descending else
		(func(a : Node, b : Node) -> bool: return a.get_meta(sort_key) < b.get_meta(sort_key))
	)

	for child : Node in items:
		listbox.add_child(child)

func _float_to_string_suffixed_dp(value : float, decimals : int) -> String:
	if value < 1000:
		return GUINode.float_to_string_dp(value, decimals)
	else:
		return GUINode.float_to_string_dp(value / 1000, decimals) + "k"

func _float_to_string_suffixed_dp_dynamic(value : float) -> String:
	if value < 2:
		return GUINode.float_to_string_dp(value, 3)
	elif value < 100:
		return GUINode.float_to_string_dp(value, 2)
	else:
		return _float_to_string_suffixed_dp(value, 1)

# data is either a PackedVector2Array, PackedVector3Array or PackedVector4Array
func _generate_listbox(table : Table, data : Variant, good_tooltips : PackedStringArray) -> void:
	var listbox : GUIListBox = _table_listboxes[table]
	if not listbox:
		return

	var entry_name : String = TABLE_ENTRY_NAMES[table]

	listbox.clear_children(data.size())
	while listbox.get_child_count() < data.size():
		var entry : Panel = GUINode.generate_gui_element(_gui_file, entry_name)
		if not entry:
			break
		listbox.add_child(entry)

	var item_paths : Array = TABLE_ITEM_PATHS[table]

	for index in min(listbox.get_child_count(), data.size()):
		var entry : Panel = listbox.get_child(index)
		if not entry:
			break

		# entry_data is either a Vector2, Vector3 or Vector4
		var entry_data : Variant = data[index]
		var good_index : int = int(entry_data.x)
		var good_tooltip : String = good_tooltips[good_index] if good_index < good_tooltips.size() else ""

		var good_button : GUIIconButton = GUINode.get_gui_icon_button_from_node_and_path(entry, item_paths[0])
		if good_button:
			good_button.set_icon_index(good_index + 2)
			good_button.set_tooltip_string(good_tooltip)
			good_button.pressed.connect(
				func() -> void:
					_update_trade_details(entry.get_meta(TABLE_COLUMN_KEYS[0]))
			)
		entry.set_meta(TABLE_COLUMN_KEYS[0], good_index)

		var set_tooltips : bool = table == Table.STOCKPILE

		var second_column : GUILabel = GUINode.get_gui_label_from_node_and_path(entry, item_paths[1])
		if second_column:
			second_column.set_auto_translate(false)
			second_column.set_text(
				_float_to_string_suffixed_dp_dynamic(entry_data.y)
				if table <= Table.POP_NEEDS
				else _float_to_string_suffixed_dp(abs(entry_data.y), 1)
				if table == Table.MARKET_ACTIVITY
				else _float_to_string_suffixed_dp(entry_data.y, 2)
			)
			if set_tooltips:
				second_column.set_tooltip_string(good_tooltip)
		entry.set_meta(TABLE_COLUMN_KEYS[1], entry_data.y)

		if item_paths.size() < 3:
			continue

		var third_column : GUILabel = GUINode.get_gui_label_from_node_and_path(entry, item_paths[2])
		if third_column:
			third_column.set_auto_translate(false)
			third_column.set_text(
				_float_to_string_suffixed_dp(entry_data.z, 0)
				if table == Table.MARKET_ACTIVITY
				else ("+" if entry_data.z >= 0 else "") + GUINode.float_to_string_dp(entry_data.z, 2)
				if table == Table.STOCKPILE
				else "(%s)" % GUINode.float_to_string_dp(entry_data.z, 0)
			)
			if set_tooltips:
				third_column.set_tooltip_string(good_tooltip)
		entry.set_meta(TABLE_COLUMN_KEYS[2], entry_data.z)

		if item_paths.size() < 4:
			continue

		var fourth_column : GUILabel = GUINode.get_gui_label_from_node_and_path(entry, item_paths[3])
		if fourth_column:
			fourth_column.set_auto_translate(false)
			fourth_column.set_text(_float_to_string_suffixed_dp(entry_data.w, 1))
			if set_tooltips:
				fourth_column.set_tooltip_string(good_tooltip)
		entry.set_meta(TABLE_COLUMN_KEYS[3], entry_data.w)

	_sort_table(table)

func _generate_good_entries(good_tooltips : PackedStringArray) -> void:
	var good_categories_info : Dictionary = MenuSingleton.get_trade_menu_good_categories_info()

	for good_category : String in good_categories_info:
		var good_category_panel : Panel = get_panel_from_nodepath("./country_trade/group_%s" % good_category)
		if not good_category_panel:
			continue

		# Fall back to 1 if panel width is too small to avoid division by zero
		var max_items_per_row : int = max(floor(good_category_panel.get_size().x / _goods_entry_offset.x), 1)

		var good_category_goods : Array[Dictionary] = good_categories_info[good_category]

		var child_count : int = good_category_panel.get_child_count()

		while child_count < good_category_goods.size():
			var good_entry_panel : Panel = generate_gui_element(_gui_file, "goods_entry")
			if not good_entry_panel:
				break
			good_entry_panel.set_position(
				_goods_entry_offset * Vector2(child_count % max_items_per_row, child_count / max_items_per_row)
			)
			good_category_panel.add_child(good_entry_panel)
			child_count += 1

		while child_count > good_category_goods.size():
			child_count -= 1
			good_category_panel.remove_child(good_category_panel.get_child(child_count))

		for category_index : int in min(child_count, good_category_goods.size()):
			var good_entry_panel : Panel = GUINode.get_panel_from_node(good_category_panel.get_child(category_index))
			if not good_entry_panel:
				continue

			const good_index_key : StringName = &"good_index"
			const current_price_key : StringName = &"current_price"
			const price_change_key : StringName = &"price_change"
			const demand_tooltip_key : StringName = &"demand_tooltip"
			const trade_settings_key : StringName = &"trade_settings"

			var good_dict : Dictionary = good_category_goods[category_index]

			var good_index : int = good_dict.get(good_index_key, 0)

			var entry_button : GUIIconButton = get_gui_icon_button_from_node_and_path(good_entry_panel, ^"./entry_button")
			if entry_button:
				# Connecting this way ensures the Callable is always the same, preventing errors when good_index changes,
				# while still allowing updates by changing the meta value rather than the Callable
				entry_button.pressed.connect(
					func() -> void:
						_update_trade_details(good_entry_panel.get_meta(TABLE_COLUMN_KEYS[0]))
				)
				good_entry_panel.set_meta(TABLE_COLUMN_KEYS[0], good_index)
				entry_button.set_tooltip_string(good_dict.get(demand_tooltip_key, ""))

			var good_button : GUIIconButton = get_gui_icon_button_from_node_and_path(good_entry_panel, ^"./goods_type")
			if good_button:
				good_button.set_icon_index(good_index + 2)
				good_button.set_tooltip_string(good_tooltips[good_index] if good_index < good_tooltips.size() else "")
				good_button.pressed.connect(func() -> void: print("Good button pressed with index ", good_index))

			var price_label : GUILabel = get_gui_label_from_node_and_path(good_entry_panel, ^"./price")
			if price_label:
				# TODO - change colour of text if price is very high or very low!!!
				price_label.set_auto_translate(false)
				price_label.set_text(GUINode.float_to_string_dp(good_dict.get(current_price_key, 0), 1) + "¤")

			var trend_icon : GUIIcon = get_gui_icon_from_node_and_path(good_entry_panel, ^"./trend_indicator")
			if trend_icon:
				var price_change : float = good_dict.get(price_change_key, 0)
				trend_icon.set_icon_index(1 if price_change > 0 else 3 if price_change < 0 else 2)
				trend_icon.set_tooltip_string(
					tr(&"TRADE_PRICE_TREND").replace("$VALUE$", GUINode.float_to_string_dp(price_change, 4))
					if price_change != 0.0 else "TRADE_PRICE_TREND_UNCHANGED"
				)

			var trade_settings : int = good_dict.get(trade_settings_key, 0)

			var automated_icon : GUIIcon = get_gui_icon_from_node_and_path(good_entry_panel, ^"./automation_indicator")
			if automated_icon:
				automated_icon.set_visible(trade_settings & MenuSingleton.TradeSettingBit.TRADE_SETTING_AUTOMATED)

			var buy_sell_icon : GUIIcon = get_gui_icon_from_node_and_path(good_entry_panel, ^"./selling_indicator")
			if buy_sell_icon:
				if trade_settings & MenuSingleton.TradeSettingBit.TRADE_SETTING_BUYING:
					buy_sell_icon.set_icon_index(2)
					buy_sell_icon.set_tooltip_string("TRADE_BUYING")
					buy_sell_icon.show()
				elif trade_settings & MenuSingleton.TradeSettingBit.TRADE_SETTING_SELLING:
					buy_sell_icon.set_icon_index(3)
					buy_sell_icon.set_tooltip_string("TRADE_SELLING")
					buy_sell_icon.show()
				else:
					buy_sell_icon.hide()
