extends GUINode

enum Page {
	NATION_RANKING,
	NATIONAL_COMPARISON,
	POLITICAL_SYSTEMS,
	POLITICAL_REFORMS,
	SOCIAL_REFORMS,
	COUNTRY_POPULATION,
	PROVINCES,
	PROVINCE_POPULATION,
	PROVINCE_PRODUCTION,
	FACTORY_PRODUCTION,
	PRICE_HISTORY,
	NUMBER_OF_PAGES
}
const _page_titles : PackedStringArray = [
	"LEDGER_HEADER_RANK",
	"LEDGER_HEADER_COUNTRYCOMPARE",
	"LEDGER_HEADER_COUNTRYPARTY",
	"LEDGER_HEADER_COUNTRYPOLITICALREFORMS",
	"LEDGER_HEADER_COUNTRYSOCIALREFORMS",
	"LEDGER_HEADER_COUNTRY_POPS",
	"LEDGER_HEADER_PROVINCES",
	"LEDGER_HEADER_PROVINCE_POPS",
	"LEDGER_HEADER_PROVINCEPRODUCTION",
	"LEDGER_HEADER_FACTORYPRODUCTION",
	"LEDGER_HEADER_GOODS_PRICEHISTORY"
]
const _column_headers := [
	["Country", "Status", "Military Score", "Industrial Score", "Prestige", "Total Score"],
	["Country", "Total pop.", "Provinces", "Factories", "Literacy", "Leadership", "Brigades", "Ships"],
	["Country", "Government", "Nation Value", "Ruling party", "Party Ideology"],
	["Country", "Slavery", "Vote Franchise", "Upper House", "Voting System", "Public Meetings", "Press Rights", "Trade Unions", "Political Parties"],
	["Country", "Minimum Wage", "Max. Workhours", "Safety Regulations", "Unemployment Subsidies", "Pensions", "Health Care", "School system"],
	["Country", POP_TYPE.ARISTOCRAT, POP_TYPE.ARTISAN, POP_TYPE.BUREAUCRAT, POP_TYPE.CAPITALIST, POP_TYPE.CLERGY, POP_TYPE.CLERK, POP_TYPE.CRAFTSMAN, POP_TYPE.FARMER, POP_TYPE.LABORER, POP_TYPE.OFFICER, POP_TYPE.SLAVE, POP_TYPE.SOLDIER],
	["Province", "Total pop.", "Avg. Mil.", "Avg. Con.", "Avg. Lit.", "Religion", "Culture", "Issue", "Ideology"],
	["Province", POP_TYPE.ARISTOCRAT, POP_TYPE.ARTISAN, POP_TYPE.BUREAUCRAT, POP_TYPE.CAPITALIST, POP_TYPE.CLERGY, POP_TYPE.CLERK, POP_TYPE.CRAFTSMAN, POP_TYPE.FARMER, POP_TYPE.LABORER, POP_TYPE.OFFICER, POP_TYPE.SLAVE, POP_TYPE.SOLDIER],
	["Province", "State", "Goods", "Output", "Income", "Employed", "Level"],
	["State", "Goods", "Output", "Income", "Employed", "Level"],
	[]
]
const _column_x_positions := [
	[0, 242, 469, 578, 739, 845],
	[0, 216, 323, 434, 549, 638, 760, 886],
	[0, 324, 486, 648, 810],
	[0, 108, 216, 324, 432, 540, 648, 756, 864],
	[0, 121, 242, 363, 484, 605, 726, 847],
	[0, 143, 213, 283, 353, 423, 493, 563, 633, 703, 773, 843, 913],
	[0, 108, 224, 326, 442, 540, 648, 756, 864],
	[0, 143, 213, 283, 353, 423, 493, 563, 633, 703, 773, 843, 913],
	[0, 138, 276, 446, 580, 705, 867],
	[0, 162, 368, 526, 675, 861],
	[]
]
enum POP_TYPE {
	ARISTOCRAT,
	OFFICER,
	CLERGY,
	CAPITALIST,
	CLERK,
	CRAFTSMAN,
	SOLDIER,
	FARMER,
	LABORER,
	SLAVE,
	ARTISAN,
	BUREAUCRAT,
	POP_TYPE_COUNT
}

var _ledger : Panel
var _current_page : Page = Page.NATION_RANKING:
	get: return _current_page
	set(new_page):
		_current_page = new_page
		while _current_page < 0:
			_current_page += Page.NUMBER_OF_PAGES
		_current_page %= Page.NUMBER_OF_PAGES
		_update_info()

var _page_title_label : GUILabel
var _page_number_label : GUILabel
var _entries_listbox : GUIListBox
var _linegraph_bg : GUIIcon
var _header_row : Panel
# TODO - add variables to store any nodes you'll need to refer in more than one function call

func _ready():
	MenuSingleton.search_cache_changed.connect(_update_info)

	add_gui_element("v2ledger", "ledger")
	_ledger = get_panel_from_nodepath(^"./ledger")

	set_click_mask_from_nodepaths([^"./ledger/ledger_bg"])

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./ledger/close")
	if close_button:
		close_button.pressed.connect(hide)

	var previous_page_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./ledger/prev")
	if previous_page_button:
		previous_page_button.pressed.connect(func() -> void: _current_page -= 1)

	var next_page_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./ledger/next")
	if next_page_button:
		next_page_button.pressed.connect(func() -> void: _current_page += 1)

	_page_title_label = get_gui_label_from_nodepath(^"./ledger/ledger_header")
	_page_number_label = get_gui_label_from_nodepath(^"./ledger/page_number")

	_entries_listbox = get_gui_listbox_from_nodepath(^"./ledger/default_listbox")

	_linegraph_bg = get_gui_icon_from_nodepath(^"./ledger/ledger_linegraph_bg")

	_header_row = get_panel_from_nodepath(^"./ledger/ledger_sort_buttons")
	# TODO - get any nodes that need setting up or caching in the variables above

	hide()

func toggle_visibility() -> void:
	if is_visible():
		hide()
	else:
		show()
		_update_info()

func _generate_header_row() -> void:
	for child in _header_row.get_children():
		child.queue_free()
	var label : GUILabel
	var sort_button : GUIButton
	var pop_icon : GUIIconButton
	for i in range(_column_headers[_current_page].size()):
		sort_button = generate_gui_element("v2ledger", "ledger_default_button")
		_header_row.add_child(sort_button)
		if _column_headers[_current_page][i] is int:
			pop_icon = generate_gui_element("v2ledger", "ledger_pop_icon")
			_header_row.add_child(pop_icon)
			pop_icon.set_icon_index(_column_headers[_current_page][i]+1)
			pop_icon.position.x = _column_x_positions[_current_page][i]
		else:
			label = generate_gui_element("v2ledger", "ledger_default_button_text")
			_header_row.add_child(label)
			label.text = _column_headers[_current_page][i]
			label.position.x = _column_x_positions[_current_page][i]
			label.position.y = 9
		sort_button.position.x = _column_x_positions[_current_page][i]
		sort_button.position.y = 9

func _generate_body(sorting: int):
	_entries_listbox.clear_children()
	var label : GUILabel
	var row : Panel
	var data := MenuSingleton.get_ledger_data(_current_page)
	data.sort_custom(func(a, b): a[sorting] < b[sorting])
	for arr in data:
		row = generate_gui_element("v2ledger", "default_listbox_entry")
		for i in arr.size():
			if arr[i] is not String:
			#if false:
				label = generate_gui_element("v2ledger", "ledger_default_textbox")
				label.text = int_to_string_suffixed(arr[i]) if arr[i] is int else str(arr[i])
			else:
				label = generate_gui_element("v2ledger", "ledger_default_textbox")
				label.text = arr[i]
			row.add_child(label)
		
			label.position.x = _column_x_positions[_current_page][i]
			if str(_column_headers[_current_page][i]) == "Country":
				var flag : GUIMaskedFlag = generate_gui_element("v2ledger", "ledger_default_flag")
				flag.set_flag_country_name(arr[i])
				row.add_child(flag)
				label.position.x += flag.get_gfx_masked_flag_texture().get_width()+1
		_entries_listbox.add_child(row)

func _update_info() -> void:
	if !is_visible(): return
	if _page_title_label:
		_page_title_label.set_text(_page_titles[_current_page])
	if _page_number_label:
		# Pages are indexed from 0 in the code, but from 1 in the UI
		_page_number_label.set_text(str(_current_page + 1))
	_linegraph_bg.visible = _current_page == Page.PRICE_HISTORY

	_generate_header_row()
	_generate_body(0)
