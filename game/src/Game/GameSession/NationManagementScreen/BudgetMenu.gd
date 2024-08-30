extends GUINode

var _active : bool = false
var _incVal : int = 0 # incremental value to see the UI update, replace later by real values

# income
var _lower_class_label : GUILabel
var _middle_class_label : GUILabel
var _upper_class_label : GUILabel
var _gold_label : GUILabel
var _total_inc_label : GUILabel

# debt
var _national_bank_label : GUILabel
var _total_funds_label : GUILabel
var _debt_val_label : GUILabel
var _interest_val_label : GUILabel

# costs
var _nat_stock_val_label : GUILabel
var _nat_stock_exp_label : GUILabel
var _mil_cost_val_label : GUILabel
var _overseas_cost_val_label : GUILabel
var _ind_sub_val_label : GUILabel
var _admin_efficiency_label : GUILabel
var _education_exp_label : GUILabel
var _administration_exp_label : GUILabel
var _social_exp_label : GUILabel
var _military_exp_label : GUILabel
var _total_exp_label : GUILabel

# others
var _tariffs_percent_label : GUILabel
var _tariff_val_label : GUILabel
var _diplomatic_balance_label : GUILabel
var _balance_label : GUILabel

var _lower_class_chart : GUIPieChart
var _middle_class_chart : GUIPieChart
var _upper_class_chart : GUIPieChart
var _debt_chart : GUIPieChart

const _screen : NationManagement.Screen = NationManagement.Screen.BUDGET

# TODO - testing function, should be replaced with calls to SIM which trigger UI updates through gamestate_updated
func _on_tax_slider_changed(slider : GUIScrollbar, label : GUILabel, tooltip : String, value : int) -> void:
	label.text = "%s¤" % GUINode.float_to_string_dp(value, 3 if abs(value) < 1000 else 1)
	slider.set_tooltip_string("%s: §Y%s%%" % [tr(tooltip), GUINode.float_to_string_dp(value, 1)])


func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_budget", "country_budget")

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	# labels
	# income
	_lower_class_label = get_gui_label_from_nodepath(^"./country_budget/tax_0_inc")
	_middle_class_label = get_gui_label_from_nodepath(^"./country_budget/tax_1_inc")
	_upper_class_label = get_gui_label_from_nodepath(^"./country_budget/tax_2_inc")
	_gold_label = get_gui_label_from_nodepath(^"./country_budget/gold_inc")
	_total_inc_label = get_gui_label_from_nodepath(^"./country_budget/total_inc")
	# debt
	_national_bank_label = get_gui_label_from_nodepath(^"./country_budget/national_bank_val")
	_total_funds_label = get_gui_label_from_nodepath(^"./country_budget/total_funds_val")
	_debt_val_label = get_gui_label_from_nodepath(^"./country_budget/debt_val")
	_interest_val_label = get_gui_label_from_nodepath(^"./country_budget/interest_val")
	# costs
	_nat_stock_val_label = get_gui_label_from_nodepath(^"./country_budget/nat_stock_val")
	_nat_stock_exp_label = get_gui_label_from_nodepath(^"./country_budget/nat_stock_est")
	_mil_cost_val_label = get_gui_label_from_nodepath(^"./country_budget/mil_cost_val")
	_overseas_cost_val_label = get_gui_label_from_nodepath(^"./country_budget/overseas_cost_val")
	_ind_sub_val_label = get_gui_label_from_nodepath(^"./country_budget/ind_sub_val")
	_admin_efficiency_label = get_gui_label_from_nodepath(^"./country_budget/admin_efficiency")
	_education_exp_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_0")
	_administration_exp_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_1")
	_social_exp_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_2")
	_military_exp_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_3")
	_total_exp_label = get_gui_label_from_nodepath(^"./country_budget/total_exp")
	# others
	_tariffs_percent_label = get_gui_label_from_nodepath(^"./country_budget/tariffs_percent")
	_tariff_val_label = get_gui_label_from_nodepath(^"./country_budget/tariff_val")
	_diplomatic_balance_label = get_gui_label_from_nodepath(^"./country_budget/diplomatic_balance")
	_balance_label = get_gui_label_from_nodepath(^"./country_budget/balance")

	# sliders
	# income
	var _lower_class_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/tax_0_slider")
	if _lower_class_slider and _lower_class_label:
		_lower_class_slider.value_changed.connect(
			func (value : int) -> void:
				_on_tax_slider_changed(_lower_class_slider, _lower_class_label, "BUDGET_TAX_POOR", value)
		)
		_lower_class_slider.emit_value_changed()
	var _middle_class_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/tax_1_slider")
	if _middle_class_slider and _middle_class_label:
		_middle_class_slider.value_changed.connect(
			func (value : int) -> void:
				_on_tax_slider_changed(_middle_class_slider, _middle_class_label, "BUDGET_TAX_MIDDLE", value)
		)
		_middle_class_slider.emit_value_changed()
	var _upper_class_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/tax_2_slider")
	if _upper_class_slider and _upper_class_label:
		_upper_class_slider.value_changed.connect(
			func (value : int) -> void:
				_on_tax_slider_changed(_upper_class_slider, _upper_class_label, "BUDGET_TAX_RICH", value)
		)
		_upper_class_slider.emit_value_changed()

	# costs
	var _land_stockpile_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/land_stockpile_slider")
	if _land_stockpile_slider and _mil_cost_val_label:
		_land_stockpile_slider.value_changed.connect(func(value : int) -> void: _mil_cost_val_label.text = "%s¤" % GUINode.float_to_string_dp(value, 2))
		_land_stockpile_slider.emit_value_changed()
	var _naval_stockpile_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/naval_stockpile_slider")
	if _naval_stockpile_slider and _overseas_cost_val_label:
		_naval_stockpile_slider.value_changed.connect(func(value : int) -> void: _overseas_cost_val_label.text = "%s¤" % GUINode.float_to_string_dp(value, 1))
		_naval_stockpile_slider.emit_value_changed()
	var _projects_stockpile_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/projects_stockpile_slider")
	if _projects_stockpile_slider:
		if _nat_stock_val_label:
			_projects_stockpile_slider.value_changed.connect(func(value : int) -> void: _nat_stock_val_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(value))
		if _nat_stock_exp_label:
			_projects_stockpile_slider.value_changed.connect(func(value : int) -> void: _nat_stock_exp_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(value))
		_projects_stockpile_slider.emit_value_changed()
	var _exp_0_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_0_slider")
	if _exp_0_slider and _education_exp_label:
		_exp_0_slider.value_changed.connect(func(value : int) -> void: _education_exp_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(value))
		_exp_0_slider.emit_value_changed()
	var _exp_1_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_1_slider")
	if _exp_1_slider:
		if _administration_exp_label:
			_exp_1_slider.value_changed.connect(func(value : int) -> void: _administration_exp_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(value))
		if _admin_efficiency_label:
			_exp_1_slider.value_changed.connect(func(value : int) -> void: _admin_efficiency_label.text = "%s%%" % GUINode.float_to_string_dp(value, 1))
		_exp_1_slider.emit_value_changed()
	var _exp_2_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_2_slider")
	if _exp_2_slider and _social_exp_label:
		_exp_2_slider.value_changed.connect(func(value : int) -> void: _social_exp_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(value))
		_exp_2_slider.emit_value_changed()
	var _exp_3_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_3_slider")
	if _exp_3_slider and _military_exp_label:
		_exp_3_slider.value_changed.connect(func(value : int) -> void: _military_exp_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(value))
		_exp_3_slider.emit_value_changed()

	# others
	var _tariff_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath(^"./country_budget/tariff_slider")
	if _tariff_slider:
		if _tariff_val_label:
			_tariff_slider.value_changed.connect(func(value : int) -> void: _tariff_val_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(value))
		if _tariffs_percent_label:
			_tariff_slider.value_changed.connect(func(value : int) -> void: _tariffs_percent_label.text = "%s%%" % GUINode.float_to_string_dp(value, 1))
		_tariff_slider.emit_value_changed()

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
	_active = active_screen == _screen
	_update_info()

func _update_info() -> void:
	# TODO - remove _incVal and link the true data with the UI
	_incVal += 1

	if _active:
		if _gold_label:
			_gold_label.text  = "%s¤" % GUINode.float_to_string_dp(_incVal - (_incVal % 7), 1)

		if _total_inc_label:
			_total_inc_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(_incVal)

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

		if _total_exp_label:
			_total_exp_label.text = "%s¤" % GUINode.float_to_string_dp_dynamic(_incVal + 1)

		if _balance_label:
			var balance : float = _incVal * 2.5
			_balance_label.text = "§%s%s¤" % ["G+" if balance > 0.0 else "R" if balance < 0.0 else "Y+", GUINode.float_to_string_dp_dynamic(balance)]

		# TODO - set strata tax and debt charts
		# TODO - update sliders to reflect changes in limits
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
