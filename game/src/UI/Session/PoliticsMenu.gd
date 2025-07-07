extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.POLITICS

const _gui_file : String = "country_politics"

# Main Window
var _country_modifiers_array : Array[GUIIconButton]
var _government_name : GUILabel
var _national_value : GUIIcon
var _government_desc : GUILabel
var _plurality_value : GUILabel
var _revanchism_value : GUILabel
var _hold_election_button : GUIIconButton
var _voters_ideology_chart : GUIPieChart
var _peoples_ideology_chart : GUIPieChart

#	Upper House
var _upperhouse_chart : GUIPieChart
var _upperhouse_listbox : GUIListBox
var _social_reform_allowed : GUILabel
var _political_reform_allowed: GUILabel
var _s_reform_allowed_icon : GUIIcon
var _p_reform_allowed_icon : GUIIcon

#	Issues
var _sort_issues : GUIIconButton
var _sort_voters : GUIIconButton
var _sort_people : GUIIconButton
var _issue_listbox : GUIListBox

#Tabs
var _reforms_tab : GUIIconButton
var _movements_tab : GUIIconButton
var _decisions_tab : GUIIconButton
var _release_nations_tab : GUIIconButton

# Decisions
var _decisions_listbox : GUIListBox

# Movements
var _suppression_value : GUILabel
var _sortby_size_button : GUIIconButton
var _sortby_radical_button: GUIIconButton
var _sortby_name_button : GUIIconButton
var _movements_listbox : GUIListBox
var _rebels_listbox : GUIListBox

# Reforms

# Unciv Reforms
var _research_points_value : GUILabel
var _unciv_progress : GUIProgressBar
var _westernize_button : GUIIconButton

# Release Nations
var _release_nation_listbox : GUIListBox

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_politics", "country_politics")

	set_click_mask_from_nodepaths([^"./country_politics/main_bg"])

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_politics/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	var politics_menu : Panel = get_panel_from_nodepath(^"./country_politics")
	if not politics_menu:
		return

	# Main Window
	_government_name = GUINode.get_gui_label_from_node(politics_menu.get_node(^"./government_name"))
	_national_value = GUINode.get_gui_icon_from_node(politics_menu.get_node(^"./national_value"))
	_government_desc = GUINode.get_gui_label_from_node(politics_menu.get_node(^"./government_desc"))
	_plurality_value = GUINode.get_gui_label_from_node(politics_menu.get_node(^"./plurality_value"))
	_revanchism_value = GUINode.get_gui_label_from_node(politics_menu.get_node(^"./revanchism_value"))
	_hold_election_button = GUINode.get_gui_icon_button_from_node(politics_menu.get_node(^"./hold_election"))
	if _hold_election_button :
		_hold_election_button.pressed.connect(func() -> void: print("ELECTION TO BE HELD"))
	_voters_ideology_chart = GUINode.get_gui_pie_chart_from_node(politics_menu.get_node(^"./chart_voters_ideologies"))
	_peoples_ideology_chart = GUINode.get_gui_pie_chart_from_node(politics_menu.get_node(^"./chart_people_ideologies"))

	# Upper House
	_upperhouse_chart = GUINode.get_gui_pie_chart_from_node(politics_menu.get_node(^"./chart_upper_house"))
	_upperhouse_listbox = GUINode.get_gui_listbox_from_node(politics_menu.get_node(^"./upperhouse_ideology_listbox"))
	_social_reform_allowed = GUINode.get_gui_label_from_node(politics_menu.get_node(^"./can_do_social_reforms"))
	_political_reform_allowed = GUINode.get_gui_label_from_node(politics_menu.get_node(^"./can_do_political_reforms"))
	_s_reform_allowed_icon = GUINode.get_gui_icon_from_node(politics_menu.get_node(^"./social_reforms_bock"))
	_p_reform_allowed_icon = GUINode.get_gui_icon_from_node(politics_menu.get_node(^"./political_reforms_bock"))

	#Issues
	_sort_issues = GUINode.get_gui_icon_button_from_node(politics_menu.get_node(^"./sort_by_issue_name"))
	if _sort_issues :
		_sort_issues.pressed.connect(func() -> void: print("ISSUES LIST SORTED BY ISSUE NAME"))
	_sort_voters = GUINode.get_gui_icon_button_from_node(politics_menu.get_node(^"./sort_by_voters"))
	if _sort_voters :
		_sort_voters.pressed.connect(func() -> void: print("ISSUES LIST SORTED BY VOTER PREFERENCE"))
	_sort_people = GUINode.get_gui_icon_button_from_node(politics_menu.get_node(^"./sort_by_people"))
	if _sort_people :
		_sort_people.pressed.connect(func() -> void: print("ISSUES LIST SORTED BY PEOPLES PREFERENCE"))
	_issue_listbox = GUINode.get_gui_listbox_from_node(politics_menu.get_node(^"./issue_listbox"))

	#Tabs
	_reforms_tab = GUINode.get_gui_icon_button_from_node(politics_menu.get_node(^"./reforms_tab"))
	_movements_tab = GUINode.get_gui_icon_button_from_node(politics_menu.get_node(^"./movements_tab"))
	_decisions_tab = GUINode.get_gui_icon_button_from_node(politics_menu.get_node(^"./decisions_tab"))
	_release_nations_tab = GUINode.get_gui_icon_button_from_node(politics_menu.get_node(^"./release_nations_tab"))

	# Decisions
	var decisions_window : Panel = GUINode.get_panel_from_node(politics_menu.get_node(^"./decision_window"))
	if decisions_window :
		_decisions_listbox = GUINode.get_gui_listbox_from_node(decisions_window.get_node(^"./decision_listbox"))

	# Movements
	var movements_window : Panel = GUINode.get_panel_from_node(politics_menu.get_node(^"./movements_window"))
	if movements_window :
		_suppression_value = GUINode.get_gui_label_from_node(movements_window.get_node(^"./suppression_value"))
		_sortby_size_button = GUINode.get_gui_icon_button_from_node(movements_window.get_node(^"./sortby_size_button"))
		if _sortby_size_button  :
			_sortby_size_button.pressed.connect(func() -> void: print("MOVEMENTS LIST SORTED BY MOVEMENT SIZE"))
		_sortby_radical_button = GUINode.get_gui_icon_button_from_node(movements_window.get_node(^"./sortby_radical_button"))
		if _sortby_radical_button  :
			_sortby_radical_button.pressed.connect(func() -> void: print("MOVEMENTS LIST SORTED BY MOVEMENT RADICALISM"))
		_sortby_name_button = GUINode.get_gui_icon_button_from_node(movements_window.get_node(^"./sortby_name_button"))
		if _sortby_name_button  :
			_sortby_name_button.pressed.connect(func() -> void: print("MOVEMENTS LIST SORTED BY MOVEMENT NAME"))
		_movements_listbox = GUINode.get_gui_listbox_from_node(movements_window.get_node(^"./movements_listbox"))
		_rebels_listbox = GUINode.get_gui_listbox_from_node(movements_window.get_node(^"./rebel_listbox"))


	var reforms_window : Panel  = GUINode.get_panel_from_node(politics_menu.get_node(^"./reforms_window"))

	# Unciv Reforms
	var unciv_reforms_window : Panel = GUINode.get_panel_from_node(politics_menu.get_node(^"./unciv_reforms_window"))
	if unciv_reforms_window :
		_research_points_value = GUINode.get_gui_label_from_node(unciv_reforms_window.get_node(^"./research_points_val"))
		_unciv_progress = GUINode.get_gui_progress_bar_from_node(unciv_reforms_window.get_node(^"./civ_progress"))
		_westernize_button = GUINode.get_gui_icon_button_from_node(unciv_reforms_window.get_node(^"./westernize_button"))
		if _westernize_button :
			_westernize_button.pressed.connect(func() -> void: print("COUNTRY HAS WESTERNIZED"))

	# Release Nations
	var release_nations_window : Panel = GUINode.get_panel_from_node(politics_menu.get_node(^"./release_nation"))
	if release_nations_window : 
		_release_nation_listbox = GUINode.get_gui_listbox_from_node(release_nations_window.get_node(^"./nations"))

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
