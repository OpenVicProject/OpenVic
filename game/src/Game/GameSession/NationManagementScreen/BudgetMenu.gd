extends GUINode

var _active : bool = false

# Tax

# Gold
var _gold_label : GUILabel

# Total Income
var _total_income_label : GUILabel

# Funds
var _national_bank_desc_label : GUILabel
var _national_bank_value_label : GUILabel
var _total_funds_value_label : GUILabel

# Debt
var _total_debt_label : GUILabel
var _interest_label : GUILabel

# Industrial Subsidies
var _industrial_subsidies_label : GUILabel

# National Stockpile
var _military_costs_label : GUILabel
var _overeseas_maintenance_label : GUILabel
var _national_stockpile_today_label : GUILabel
var _national_stockpile_tomorrow_label : GUILabel
var _land_spending_slider : GUIScrollbar
var _naval_spending_slider : GUIScrollbar
var _construction_spending_slider : GUIScrollbar

# Education
var _education_spending_slider : GUIScrollbar
var _education_spending_value_label : GUILabel

# Administration
var _administrative_efficiency_label : GUILabel
var _administration_spending_slider : GUIScrollbar
var _administration_spending_value_label : GUILabel

# Social Spending
var _social_spending_slider : GUIScrollbar
var _social_spending_value_label : GUILabel

# Military Spending
var _military_spending_slider : GUIScrollbar
var _military_spending_value_label : GUILabel

# Total Expense
var _total_expense_label : GUILabel

# Tariffs
var _tariffs_slider : GUIScrollbar
var _tariffs_percentage_label : GUILabel
var _tariffs_value_label : GUILabel

# Diplomatic Balance
var _diplomatic_balance_label : GUILabel

# Projected Daily Balance
var _projected_daily_balance_label : GUILabel

const _screen : NationManagement.Screen = NationManagement.Screen.BUDGET

# TODO - testing function, should be replaced with calls to SIM which trigger UI updates through gamestate_updated
func _on_tax_slider_changed(slider : GUIScrollbar, label : GUILabel, tooltip : StringName, value : int) -> void:
	label.text = "%s¤" % GUINode.float_to_string_dp(value, 3 if abs(value) < 1000 else 1)
	slider.set_tooltip_string("%s: §Y%s%%" % [tr(tooltip), GUINode.float_to_string_dp(value, 1)])

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element("country_budget", "country_budget")

	set_click_mask_from_nodepaths([^"./country_budget/main_bg"])

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	const pop_sprites_by_type_key : StringName = &"pop_sprites_by_type";
	const pop_types_by_strata_key : StringName = &"pop_types_by_strata";
	const education_pop_types_key : StringName = &"education_pop_types";
	const administration_pop_types_key : StringName = &"administration_pop_types";
	# There are no social spending pop types even though an overlapping elements box for them exists in the GUI file
	# Non-administrative social_reforms reform group names
	const social_spending_subcategories_key : StringName = &"social_spending_subcategories";
	const military_spending_pop_types_key : StringName = &"military_spending_pop_types";
	# Military spending pop type names
	const military_spending_subcategories_key : StringName = &"military_spending_subcategories";

	var budget_setup_info : Dictionary = MenuSingleton.get_budget_menu_setup_info()

	# Tax

	# Gold
	var gold_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/gold_desc")
	if gold_desc_label:
		gold_desc_label.set_tooltip_string("precious_metal_desc")
	_gold_label = get_gui_label_from_nodepath(^"./country_budget/gold_inc")

	# Total Income
	_total_income_label = get_gui_label_from_nodepath(^"./country_budget/total_inc")

	# Funds
	_national_bank_desc_label = get_gui_label_from_nodepath(^"./country_budget/national_bank_desc")
	_national_bank_value_label = get_gui_label_from_nodepath(^"./country_budget/national_bank_val")
	_total_funds_value_label = get_gui_label_from_nodepath(^"./country_budget/total_funds_val")

	# Debt
	_total_debt_label = get_gui_label_from_nodepath(^"./country_budget/debt_val")
	_interest_label = get_gui_label_from_nodepath(^"./country_budget/interest_val")

	# Industrial Subsidies
	var industrial_subsidies_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/ind_sub_desc")
	if industrial_subsidies_desc_label:
		industrial_subsidies_desc_label.set_tooltip_string("IND_SUP_DESC")
	_industrial_subsidies_label = get_gui_label_from_nodepath(^"./country_budget/ind_sub_val")

	# National Stockpile
	var national_stockpile_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/nat_stock_desc")
	if national_stockpile_desc_label:
		national_stockpile_desc_label.set_tooltip_string("NAT_STOCK_DESC")
	var military_costs_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/mil_cost_desc")
	if military_costs_desc_label:
		military_costs_desc_label.set_tooltip_string("MIL_COST_DESC")
	_military_costs_label = get_gui_label_from_nodepath(^"./country_budget/mil_cost_val")
	_overeseas_maintenance_label = get_gui_label_from_nodepath(^"./country_budget/overseas_cost_val")
	_national_stockpile_today_label = get_gui_label_from_nodepath(^"./country_budget/nat_stock_val")
	_national_stockpile_tomorrow_label = get_gui_label_from_nodepath(^"./country_budget/nat_stock_est")
	_land_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/land_stockpile_slider")
	_naval_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/naval_stockpile_slider")
	_construction_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/projects_stockpile_slider")

	# Education
	var education_spending_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/education_desc")
	if education_spending_desc_label:
		education_spending_desc_label.set_tooltip_string("EDU_DESC")
	_education_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_0_slider")
	_education_spending_value_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_0")
	if _education_spending_value_label:
		_education_spending_value_label.set_tooltip_string("$DIST_EDUCATION$" + MenuSingleton.get_tooltip_separator() + "$EDU_DESC$")

	# Administration
	var administration_spending_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/admin_desc")
	if administration_spending_desc_label:
		administration_spending_desc_label.set_tooltip_string("ADM_DESC")
	_administrative_efficiency_label = get_gui_label_from_nodepath(^"./country_budget/admin_efficiency")
	_administration_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_1_slider")
	_administration_spending_value_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_1")
	if _administration_spending_value_label:
		_administration_spending_value_label.set_tooltip_string("$DIST_ADMINISTRATION$" + MenuSingleton.get_tooltip_separator() + "$ADM_DESC$")

	# Social Spending
	var social_spending_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/soc_stand_desc")
	if social_spending_desc_label:
		social_spending_desc_label.set_tooltip_string("SOCIAL_DESC2")
	_social_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_2_slider")
	_social_spending_value_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_2")
	if _social_spending_value_label:
		_social_spending_value_label.set_tooltip_string("$DIST_SOCIAL$" + MenuSingleton.get_tooltip_separator() + "$SOCIAL_DESC2$")

	# Military Spending
	var military_spending_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/mil_spend_desc")
	if military_spending_desc_label:
		military_spending_desc_label.set_tooltip_string("DEFENCE_DESC")
	_military_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_3_slider")
	_military_spending_value_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_3")
	if _military_spending_value_label:
		_military_spending_value_label.set_tooltip_string("$DIST_DEFENCE$" + MenuSingleton.get_tooltip_separator() + "$DEFENCE_DESC$")

	# Total Expense
	_total_expense_label = get_gui_label_from_nodepath(^"./country_budget/total_exp")

	# Tariffs
	var tariffs_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/tariffs")
	if tariffs_desc_label:
		tariffs_desc_label.set_tooltip_string("TARIFFS_DESC")
	_tariffs_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/tariff_slider")

	# TODO - override tariff slider min, max, step!

	_tariffs_percentage_label = get_gui_label_from_nodepath(^"./country_budget/tariffs_percent")
	_tariffs_value_label = get_gui_label_from_nodepath(^"./country_budget/tariff_val")

	# Diplomatic Balance
	var diplomatic_balance_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/diplomatic_desc")
	if diplomatic_balance_desc_label:
		diplomatic_balance_desc_label.set_tooltip_string("BUDGET_DIPL_DESC")
	_diplomatic_balance_label = get_gui_label_from_nodepath(^"./country_budget/diplomatic_balance")

	# Projected Daily Balance
	_projected_daily_balance_label = get_gui_label_from_nodepath(^"./country_budget/balance")

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

		# TODO - some values go red when they cannot be paid: national stockpile today + tomorrow, education, administration, social, military spending
		# The non-national stockpile expenses also have this added to their tooltips: "We can only afford to pay X£ of our expenses!" ("EXPENSE_NO_AFFORD", "$VAL$")

		var budget_info : Dictionary = MenuSingleton.get_budget_menu_info()

		const pop_type_needs_tooltips_key : StringName = &"pop_type_needs_tooltips";
		const pop_type_present_bools_key : StringName = &"pop_type_present_bools";

		# Tax
		const tax_info_by_strata_key : StringName = &"tax_info_by_strata";
		# Per strata:
		const strata_needs_pie_chart_key : StringName = &"strata_needs_pie_chart";
		const strata_tax_slider_key : StringName = &"strata_tax_slider";
		const strata_tax_slider_tooltip_key : StringName = &"strata_tax_slider_tooltip";
		const strata_tax_value_key : StringName = &"strata_tax_value";
		const strata_tax_value_tooltip_key : StringName = &"strata_tax_value_tooltip";

		# TODO

		# Gold
		const gold_key : StringName = &"gold";
		const gold_tooltip_key : StringName = &"gold_tooltip";

		if _gold_label:
			_gold_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(gold_key, 0), 1))
			_gold_label.set_tooltip_string(budget_info.get(gold_tooltip_key, ""))

		# Total Income
		const total_income_key : StringName = &"total_income";
		const total_income_tooltip_key : StringName = &"total_income_tooltip";

		if _total_income_label:
			_total_income_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(total_income_key, 0)))
			_total_income_label.set_tooltip_string(budget_info.get(total_income_tooltip_key, ""))

		# Funds
		const national_bank_key : StringName = &"national_bank";
		const national_bank_tooltip_key : StringName = &"national_bank_tooltip";
		const total_funds_key : StringName = &"total_funds";

		var national_bank_tooltip_string : String = budget_info.get(national_bank_tooltip_key, "")

		if _national_bank_desc_label:
			_national_bank_desc_label.set_tooltip_string(national_bank_tooltip_string)
		if _national_bank_value_label:
			_national_bank_value_label.set_text("%s¤" % GUINode.float_to_string_suffixed(budget_info.get(national_bank_key, 0)))
			_national_bank_value_label.set_tooltip_string(national_bank_tooltip_string)
		if _total_funds_value_label:
			_total_funds_value_label.set_text("%s¤" % GUINode.float_to_string_suffixed(budget_info.get(total_funds_key, 0)))

		# Debt
		const total_debt_key : StringName = &"total_debt";
		const interest_key : StringName = &"interest";
		const loans_taken_key : StringName = &"loans_taken";
		const loans_given_key : StringName = &"loans_given";

		if _total_debt_label:
			_total_debt_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(total_debt_key, 0), 1))
		if _interest_label:
			_interest_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(interest_key, 0), 2))
		# TODO - update loans pie chart and table with loans_taken_key and loans_given_key

		# Industrial Subsidies
		const industrial_subsidies_key : StringName = &"industrial_subsidies";
		const industrial_subsidies_tooltip_key : StringName = &"industrial_subsidies_tooltip";

		if _industrial_subsidies_label:
			_industrial_subsidies_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(industrial_subsidies_key, 0), 1))
			_industrial_subsidies_label.set_tooltip_string(budget_info.get(industrial_subsidies_tooltip_key, ""))

		# National Stockpile
		const military_costs_key : StringName = &"military_costs";
		const military_costs_tooltip_key : StringName = &"military_costs_tooltip";
		const overseas_maintenance_key : StringName = &"overseas_maintenance";
		const overseas_maintenance_tooltip_key : StringName = &"overseas_maintenance_tooltip";
		const national_stockpile_today_key : StringName = &"national_stockpile_today";
		const national_stockpile_today_tooltip_key : StringName = &"national_stockpile_today_tooltip";
		const national_stockpile_tomorrow_key : StringName = &"national_stockpile_tomorrow";
		const national_stockpile_tomorrow_tooltip_key : StringName = &"national_stockpile_tomorrow_tooltip";
		const land_spending_slider_key : StringName = &"land_spending_slider";
		const land_spending_slider_tooltip_key : StringName = &"land_spending_slider_tooltip";
		const naval_spending_slider_key : StringName = &"naval_spending_slider";
		const naval_spending_slider_tooltip_key : StringName = &"naval_spending_slider_tooltip";
		const construction_spending_slider_key : StringName = &"construction_spending_slider";
		const construction_spending_slider_tooltip_key : StringName = &"construction_spending_slider_tooltip";

		if _military_costs_label:
			# TODO - make this 2dp, k for over thousand
			_military_costs_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(military_costs_key, 0), 2))
			_military_costs_label.set_tooltip_string(budget_info.get(military_costs_tooltip_key, ""))
		if _overeseas_maintenance_label:
			_overeseas_maintenance_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(overseas_maintenance_key, 0), 1))
			_overeseas_maintenance_label.set_tooltip_string(budget_info.get(overseas_maintenance_tooltip_key, ""))
		if _national_stockpile_today_label:
			_national_stockpile_today_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(national_stockpile_today_key, 0)))
			_national_stockpile_today_label.set_tooltip_string(budget_info.get(national_stockpile_today_tooltip_key, ""))
		if _national_stockpile_tomorrow_label:
			_national_stockpile_tomorrow_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(national_stockpile_tomorrow_key, 0)))
			_national_stockpile_tomorrow_label.set_tooltip_string(budget_info.get(national_stockpile_tomorrow_tooltip_key, ""))
		if _land_spending_slider:
			# TODO - update _land_spending_slider with land_spending_slider_key
			_land_spending_slider.set_tooltip_string(budget_info.get(land_spending_slider_tooltip_key, ""))
		if _naval_spending_slider:
			# TODO - update _naval_spending_slider with naval_spending_slider_key
			_naval_spending_slider.set_tooltip_string(budget_info.get(naval_spending_slider_tooltip_key, ""))
		if _construction_spending_slider:
			# TODO - update _construction_spending_slider with construction_spending_slider_key
			_construction_spending_slider.set_tooltip_string(budget_info.get(construction_spending_slider_tooltip_key, ""))

		# Education
		const education_spending_slider_key : StringName = &"education_spending_slider";
		const education_spending_slider_tooltip_key : StringName = &"education_spending_slider_tooltip";
		const education_spending_value_key : StringName = &"education_spending_value";

		# TODO - education pop icons
		if _education_spending_slider:
			# TODO - update _education_spending_slider with education_spending_slider_key
			_education_spending_slider.set_tooltip_string(budget_info.get(education_spending_slider_tooltip_key, ""))
		if _education_spending_value_label:
			_education_spending_value_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(education_spending_value_key, 0)))

		# Administration
		const administrative_efficiency_key : StringName = &"administrative_efficiency";
		const administrative_efficiency_tooltip_key : StringName = &"administrative_efficiency_tooltip";
		const administration_spending_slider_key : StringName = &"administration_spending_slider";
		const administration_spending_slider_tooltip_key : StringName = &"administration_spending_slider_tooltip";
		const administration_spending_value_key : StringName = &"administration_spending_value";

		if _administrative_efficiency_label:
			_administrative_efficiency_label.set_text("%s%%"% GUINode.float_to_string_dp(budget_info.get(administrative_efficiency_key, 0), 1))
			_administrative_efficiency_label.set_tooltip_string(budget_info.get(administrative_efficiency_tooltip_key, ""))
		# TODO - administration pop icons
		if _administration_spending_slider:
			# TODO - update _administration_spending_slider with administration_spending_slider_key
			_administration_spending_slider.set_tooltip_string(budget_info.get(administration_spending_slider_tooltip_key, ""))
		if _administration_spending_value_label:
			_administration_spending_value_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(administration_spending_value_key, 0)))

		# Social Spending
		const social_spending_slider_key : StringName = &"social_spending_slider";
		const social_spending_slider_tooltip_key : StringName = &"social_spending_slider_tooltip";
		const social_spending_value_key : StringName = &"social_spending_value";
		const social_spending_subcategories_key : StringName = &"social_spending_subcategories";

		if _social_spending_slider:
			# TODO - update _social_spending_slider with social_spending_slider_key
			_social_spending_slider.set_tooltip_string(budget_info.get(social_spending_slider_tooltip_key, ""))
		if _social_spending_value_label:
			_social_spending_value_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(social_spending_value_key, 0)))
		# TODO - social spending subcategories with social_spending_subcategories_key

		# Miltiary Spending
		const military_spending_slider_key : StringName = &"military_spending_slider";
		const military_spending_slider_tooltip_key : StringName = &"military_spending_slider_tooltip";
		const military_spending_value_key : StringName = &"military_spending_value";
		const military_spending_subcategories_key : StringName = &"military_spending_subcategories";

		# TODO - military pop icons
		if _military_spending_slider:
			# TODO - update _military_spending_slider with military_spending_slider_key
			_military_spending_slider.set_tooltip_string(budget_info.get(military_spending_slider_tooltip_key, ""))
		if _military_spending_value_label:
			_military_spending_value_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(military_spending_value_key, 0)))
		# TODO - military spending subcategories with military_spending_subcategories_key

		# Total Expense
		const total_expense_key : StringName = &"total_expense";
		const total_expense_tooltip_key : StringName = &"total_expense_tooltip";

		if _total_expense_label:
			_total_expense_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(total_expense_key, 0)))
			_total_expense_label.set_tooltip_string(budget_info.get(total_expense_tooltip_key, ""))

		# Tariffs
		const tariff_slider_key : StringName = &"tariff_slider";
		const tariff_slider_tooltip_key : StringName = &"tariff_slider_tooltip";
		const tariff_value_key : StringName = &"tariff_value";

		if _tariffs_slider:
			# TODO - update _tariffs_slider with tariff_slider_key
			_tariffs_slider.set_tooltip_string(budget_info.get(tariff_slider_tooltip_key, ""))
		if _tariffs_percentage_label:
			_tariffs_percentage_label.set_text("%s%%" % GUINode.float_to_string_dp(_tariffs_slider.get_value_scaled() if _tariffs_slider else 0, 1))
		if _tariffs_value_label:
			_tariffs_value_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(tariff_value_key, 0)))

		# Diplomatic Balance
		const diplomatic_balance_key : StringName = &"diplomatic_balance";
		const diplomatic_balance_tooltip_key : StringName = &"diplomatic_balance_tooltip";

		if _diplomatic_balance_label:
			var diplomatic_balance : float = budget_info.get(diplomatic_balance_key, 0)
			_diplomatic_balance_label.set_text(
				"§%s%s¤" % [
					"G" if diplomatic_balance > 0.0 else "R" if diplomatic_balance < 0.0 else "Y",
					GUINode.float_to_string_dp(diplomatic_balance, 1)
				]
			)
			_diplomatic_balance_label.set_tooltip_string(budget_info.get(diplomatic_balance_tooltip_key, ""))

		# Projected Daily Balance
		const projected_daily_balance_key : StringName = &"projected_daily_balance";

		if _projected_daily_balance_label:
			var balance : float = budget_info.get(projected_daily_balance_key, 0)
			_projected_daily_balance_label.set_text(
				"§%s%s¤" % [
					"G+" if balance > 0.0 else "R" if balance < 0.0 else "Y+",
					GUINode.float_to_string_dp_dynamic(balance)
				]
			)

		show()
	else:
		hide()
