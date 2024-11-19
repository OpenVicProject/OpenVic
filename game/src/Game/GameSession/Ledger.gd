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
# TODO - add variables to store any nodes you'll need to refer in more than one function call

func _ready():
	MenuSingleton.search_cache_changed.connect(_update_info)

	add_gui_element("v2ledger", "ledger")

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

	# TODO - get any nodes that need setting up or caching in the variables above

	hide()

func toggle_visibility() -> void:
	if is_visible():
		hide()
	else:
		show()
		_update_info()

func _update_info() -> void:
	if is_visible():
		if _page_title_label:
			_page_title_label.set_text(_page_titles[_current_page])

		if _page_number_label:
			# Pages are indexed from 0 in the code, but from 1 in the UI
			_page_number_label.set_text(str(_current_page + 1))

		# TODO - set contents of current ledger page
