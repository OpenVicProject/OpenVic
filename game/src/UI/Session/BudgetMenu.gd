extends GUINode

var _active : bool = false
var _incVal : int = 0 # incremental value to see the UI update, replace later by real values

# debt
var _national_bank_label : GUILabel
var _total_funds_label : GUILabel
var _debt_val_label : GUILabel
var _interest_val_label : GUILabel

# costs
var _overseas_cost_val_label : GUILabel
var _ind_sub_val_label : GUILabel

# others
var _diplomatic_balance_label : GUILabel

var _lower_class_chart : GUIPieChart
var _middle_class_chart : GUIPieChart
var _upper_class_chart : GUIPieChart
var _debt_chart : GUIPieChart

const SCREEN := NationManagement.Screen.BUDGET

# TODO - testing function, should be replaced with calls to SIM which trigger UI updates through gamestate_updated
#	slider.set_tooltip_string("%s: §Y%s%%" % [tr(tooltip), GUINode.float_to_string_dp(value, 1)])

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_budget", "country_budget")
	MenuSingleton.link_budget_menu_to_cpp(self)

	set_click_mask_from_nodepaths([^"./country_budget/main_bg"])

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(SCREEN))

	# labels
	# debt
	_national_bank_label = get_gui_label_from_nodepath(^"./country_budget/national_bank_val")
	_total_funds_label = get_gui_label_from_nodepath(^"./country_budget/total_funds_val")
	_debt_val_label = get_gui_label_from_nodepath(^"./country_budget/debt_val")
	_interest_val_label = get_gui_label_from_nodepath(^"./country_budget/interest_val")
	# costs
	_ind_sub_val_label = get_gui_label_from_nodepath(^"./country_budget/ind_sub_val")
	# others
	_diplomatic_balance_label = get_gui_label_from_nodepath(^"./country_budget/diplomatic_balance")

	# debt buttons
	var _tab_takenloans_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/tab_takenloans")
	if _tab_takenloans_button:
		_tab_takenloans_button.pressed.connect(_switch_loans_tab.bind(true))
	var _tab_givenloans_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/tab_givenloans")
	if _tab_givenloans_button:
		_tab_givenloans_button.pressed.connect(_switch_loans_tab.bind(false))
	var _debt_sort_country_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/debt_sort_country")
	if _debt_sort_country_button:
		_debt_sort_country_button.pressed.connect(_sort_loans.bind(true))
	var _debt_sort_amount_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/debt_sort_amount")
	if _debt_sort_amount_button:
		_debt_sort_amount_button.pressed.connect(_sort_loans.bind(false))
	var _take_loan_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/take_loan")
	if _take_loan_button:
		_take_loan_button.pressed.connect(_take_loan)
	var _repay_loan_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/repay_loan")
	if _repay_loan_button:
		_repay_loan_button.pressed.connect(_repay_loan)

	# charts
	_lower_class_chart = get_gui_pie_chart_from_nodepath(^"./country_budget/chart_0")
	_middle_class_chart = get_gui_pie_chart_from_nodepath(^"./country_budget/chart_1")
	_upper_class_chart = get_gui_pie_chart_from_nodepath(^"./country_budget/chart_2")
	_debt_chart = get_gui_pie_chart_from_nodepath(^"./country_budget/chart_debt")

	# TODO - generate strata pop type icons

	_update_info()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()

func _on_update_active_nation_management_screen(active_screen : NationManagement.Screen) -> void:
	_active = active_screen == SCREEN
	_update_info()

func _update_info() -> void:
	# TODO - remove _incVal and link the true data with the UI
	_incVal += 1

	if _active:
		if _national_bank_label:
			_national_bank_label.text = "%s¤" % GUINode.float_to_string_suffixed(_incVal * 2)

		if _total_funds_label:
			_total_funds_label.text = "%s¤" % GUINode.float_to_string_suffixed(_incVal * 3)

		if _debt_val_label:
			_debt_val_label.text = "%s¤" % GUINode.float_to_string_dp(_incVal * 4, 1)

		if _interest_val_label:
			_interest_val_label.text = "%s¤" % GUINode.float_to_string_dp(_incVal * 5, 2)

		if _ind_sub_val_label:
			_ind_sub_val_label.text = "%s¤" % GUINode.float_to_string_dp(_incVal * 6, 1)

		if _diplomatic_balance_label:
			# TODO - check colours and +/- when non-zero
			_diplomatic_balance_label.text = "§Y%s¤" % GUINode.float_to_string_dp(_incVal * 8, 1)

		# TODO - set strata tax and debt charts
		# TODO - update loans taken/given list and enable/disable take/give loan buttons

		show()
	else:
		hide()

func _switch_loans_tab(taken_loans : bool) -> void:
	# TODO - code the necessary logic
	#if taken_loans:
	#else: # given loans
	pass

func _sort_loans(sort_by_country : bool) -> void:
	# TODO - code the necessary logic
	#if sort_by_country:
	#else: # sort by amount
	pass

func _take_loan() -> void:
	# TODO - code the necessary logic
	pass

func _repay_loan() -> void:
	# TODO - code the necessary logic
	pass
