extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.BUDGET

const _gui_file : String = "country_budget"

# Tax
var _strata_tax_piecharts : Array[GUIPieChart]
var _strata_tax_pop_icon_buttons : Array[Array] # Array of Array of GUIIconButtons
var _strata_tax_sliders : Array[GUIScrollbar]
var _strata_tax_labels : Array[GUILabel]

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
var _repaying_debts_icon : GUIIcon

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
var _education_spending_pop_icon_buttons : Array[GUIIconButton]
var _education_spending_slider : GUIScrollbar
var _education_spending_value_label : GUILabel

# Administration
var _administrative_efficiency_label : GUILabel
var _administration_spending_pop_icon_buttons : Array[GUIIconButton]
var _administration_spending_slider : GUIScrollbar
var _administration_spending_value_label : GUILabel

# Social Spending
var _social_spending_slider : GUIScrollbar
var _social_spending_value_label : GUILabel
var _social_spending_subcategory_labels : Array[GUILabel]

# Military Spending
var _military_spending_pop_icon_buttons : Array[GUIIconButton]
var _military_spending_slider : GUIScrollbar
var _military_spending_value_label : GUILabel
var _military_spending_subcategory_labels : Array[GUILabel]

# Total Expense
var _total_expense_label : GUILabel

# Tariffs
var _tariff_slider : GUIScrollbar
var _tariff_percentage_label : GUILabel
var _tariff_value_label : GUILabel

# Diplomatic Balance
var _diplomatic_balance_label : GUILabel

# Projected Daily Balance
var _projected_daily_balance_label : GUILabel

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element(_gui_file, "country_budget")

	set_click_mask_from_nodepaths([^"./country_budget/main_bg"])

	var country_budget_panel : Panel = get_panel_from_nodepath(^"./country_budget")
	if not country_budget_panel:
		return

	var close_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_budget/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	const stratas_key : StringName = &"stratas"
	const pop_types_by_strata_key : StringName = &"pop_types_by_strata"
	const pop_sprites_by_type_key : StringName = &"pop_sprites_by_type"
	const education_spending_pop_types_key : StringName = &"education_spending_pop_types"
	const administration_spending_pop_types_key : StringName = &"administration_spending_pop_types"
	# There are no social spending pop types even though an overlapping elements box for them exists in the GUI file
	# Non-administrative social_reforms reform group names
	const social_spending_subcategories_key : StringName = &"social_spending_subcategories"
	const military_spending_pop_types_key : StringName = &"military_spending_pop_types"
	# Military spending pop type names
	const military_spending_subcategories_key : StringName = &"military_spending_subcategories"

	_tariff_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/tariff_slider")

	var budget_setup_info : Dictionary = MenuSingleton.get_budget_menu_setup_info(_tariff_slider)

	var pop_sprites_by_type : PackedByteArray = budget_setup_info.get(pop_sprites_by_type_key, [] as PackedByteArray)

	# Tax
	var strata_identifiers : PackedStringArray = budget_setup_info.get(stratas_key, [] as PackedStringArray)
	var pop_types_by_strata : Array[PackedByteArray] = budget_setup_info.get(pop_types_by_strata_key, [] as Array[PackedByteArray])

	for index : int in strata_identifiers.size():
		var strata_tooltip_string : String = "TAX_%s_DESC" % strata_identifiers[index].to_upper()

		var strata_desc_icon : GUIIcon = get_gui_icon_from_nodepath("./country_budget/icon_%d" % index)
		if strata_desc_icon:
			strata_desc_icon.set_tooltip_string(strata_tooltip_string)

		var strata_desc_label : GUILabel = get_gui_label_from_nodepath("./country_budget/tax_%d_desc" % index)
		if strata_desc_label:
			strata_desc_label.set_tooltip_string(strata_tooltip_string)

		_strata_tax_piecharts.push_back(get_gui_pie_chart_from_nodepath("./country_budget/chart_%d" % index))

		_strata_tax_pop_icon_buttons.push_back(_setup_pop_icon_buttons(
			"./country_budget/tax_%d_pops" % index,
			"pop_listitem",
			pop_types_by_strata[index] if index < pop_types_by_strata.size() else [] as PackedByteArray,
			pop_sprites_by_type
		))

		var strata_tax_slider : GUIScrollbar = get_gui_scrollbar_from_nodepath("./country_budget/tax_%d_slider" % index)
		if strata_tax_slider:
			strata_tax_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_strata_tax_slider_value(index, strata_tax_slider))
		_strata_tax_sliders.push_back(strata_tax_slider)
		_strata_tax_labels.push_back(get_gui_label_from_nodepath("./country_budget/tax_%d_inc" % index))

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
	_repaying_debts_icon = get_gui_icon_from_nodepath(^"./country_budget/gunboat_alert")

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
	if _land_spending_slider:
		_land_spending_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_land_spending_slider_value(_land_spending_slider))
	_naval_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/naval_stockpile_slider")
	if _naval_spending_slider:
		_naval_spending_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_naval_spending_slider_value(_naval_spending_slider))
	_construction_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/projects_stockpile_slider")
	if _construction_spending_slider:
		_construction_spending_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_construction_spending_slider_value(_construction_spending_slider))

	# Education
	var education_spending_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/education_desc")
	if education_spending_desc_label:
		education_spending_desc_label.set_tooltip_string("EDU_DESC")
	_education_spending_pop_icon_buttons = _setup_pop_icon_buttons(
		^"./country_budget/exp_0_pops",
		"pop_listitem_small",
		budget_setup_info.get(education_spending_pop_types_key, [] as PackedByteArray),
		pop_sprites_by_type
	)
	_education_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_0_slider")
	if _education_spending_slider:
		_education_spending_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_education_spending_slider_value(_education_spending_slider))
	_education_spending_value_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_0")
	if _education_spending_value_label:
		_education_spending_value_label.set_tooltip_string("$DIST_EDUCATION$" + MenuSingleton.get_tooltip_separator() + "$EDU_DESC$")

	# Administration
	var administration_spending_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/admin_desc")
	if administration_spending_desc_label:
		administration_spending_desc_label.set_tooltip_string("ADM_DESC")
	_administrative_efficiency_label = get_gui_label_from_nodepath(^"./country_budget/admin_efficiency")
	_administration_spending_pop_icon_buttons = _setup_pop_icon_buttons(
		^"./country_budget/exp_1_pops",
		"pop_listitem_small",
		budget_setup_info.get(administration_spending_pop_types_key, [] as PackedByteArray),
		pop_sprites_by_type
	)
	_administration_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_1_slider")
	if _administration_spending_slider:
		_administration_spending_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_administration_spending_slider_value(_administration_spending_slider))
	_administration_spending_value_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_1")
	if _administration_spending_value_label:
		_administration_spending_value_label.set_tooltip_string("$DIST_ADMINISTRATION$" + MenuSingleton.get_tooltip_separator() + "$ADM_DESC$")

	# Social Spending
	var social_spending_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/soc_stand_desc")
	if social_spending_desc_label:
		social_spending_desc_label.set_tooltip_string("SOCIAL_DESC2")
	hide_node(^"./country_budget/exp_2_pops")
	_social_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_2_slider")
	if _social_spending_slider:
		_social_spending_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_social_spending_slider_value(_social_spending_slider))
	_social_spending_value_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_2")
	if _social_spending_value_label:
		_social_spending_value_label.set_tooltip_string("$DIST_SOCIAL$" + MenuSingleton.get_tooltip_separator() + "$SOCIAL_DESC2$")
	_social_spending_subcategory_labels = _setup_spending_subcategories(
		country_budget_panel,
		GUINode.get_gui_position(_gui_file, "budget_social_spending_pos"),
		budget_setup_info.get(social_spending_subcategories_key, [] as PackedStringArray),
		GUINode.get_gui_position(_gui_file, "budget_social_spending_wrap_count").x
	)

	# Military Spending
	var military_spending_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/mil_spend_desc")
	if military_spending_desc_label:
		military_spending_desc_label.set_tooltip_string("DEFENCE_DESC")
	_military_spending_pop_icon_buttons = _setup_pop_icon_buttons(
		^"./country_budget/exp_3_pops",
		"pop_listitem",
		budget_setup_info.get(military_spending_pop_types_key, [] as PackedByteArray),
		pop_sprites_by_type
	)
	_military_spending_slider = get_gui_scrollbar_from_nodepath(^"./country_budget/exp_3_slider")
	if _military_spending_slider:
		_military_spending_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_military_spending_slider_value(_military_spending_slider))
	_military_spending_value_label = get_gui_label_from_nodepath(^"./country_budget/exp_val_3")
	if _military_spending_value_label:
		_military_spending_value_label.set_tooltip_string("$DIST_DEFENCE$" + MenuSingleton.get_tooltip_separator() + "$DEFENCE_DESC$")
	_military_spending_subcategory_labels = _setup_spending_subcategories(
		country_budget_panel,
		GUINode.get_gui_position(_gui_file, "budget_military_spending_pos"),
		budget_setup_info.get(military_spending_subcategories_key, [] as PackedStringArray)
	)

	# Total Expense
	_total_expense_label = get_gui_label_from_nodepath(^"./country_budget/total_exp")

	# Tariffs
	var tariff_desc_label : GUILabel = get_gui_label_from_nodepath(^"./country_budget/tariffs")
	if tariff_desc_label:
		tariff_desc_label.set_tooltip_string("TARIFFS_DESC")
	# _tariff_slider already loaded above, as it's needed for get_budget_menu_setup_info which updates its step size and min/max values
	if _tariff_slider:
		_tariff_slider.value_changed.connect(func(value : int) -> void: PlayerSingleton.set_tariff_rate_slider_value(_tariff_slider))

	_tariff_percentage_label = get_gui_label_from_nodepath(^"./country_budget/tariffs_percent")
	_tariff_value_label = get_gui_label_from_nodepath(^"./country_budget/tariff_val")

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

const pop_icon_type_meta_key : StringName = &"pop_type"

func _setup_pop_icon_buttons(overlapping_elements_box_path : NodePath, pop_icon_panel_name : String, pop_types : PackedByteArray, pop_sprites_by_type : PackedByteArray) -> Array[GUIIconButton]:
	var pop_icon_buttons : Array[GUIIconButton]

	var pop_icons_overlapping_elements_box : GUIOverlappingElementsBox = get_gui_overlapping_elements_box_from_nodepath(overlapping_elements_box_path)
	if not pop_icons_overlapping_elements_box:
		return pop_icon_buttons

	pop_icons_overlapping_elements_box.set_gui_child_element_name(_gui_file, pop_icon_panel_name)
	pop_icons_overlapping_elements_box.set_child_count(pop_types.size())

	for index : int in pop_types.size():
		var pop_icon_panel : Panel = get_panel_from_node(pop_icons_overlapping_elements_box.get_child(index))
		if not pop_icon_panel:
			continue

		var pop_icon_button : GUIIconButton = get_gui_icon_button_from_node_and_path(pop_icon_panel, ^"./pop")
		if not pop_icon_button:
			continue

		var pop_type : int = pop_types[index]
		pop_icon_button.set_meta(pop_icon_type_meta_key, pop_type)

		if pop_type < pop_sprites_by_type.size():
			pop_icon_button.set_icon_index(pop_sprites_by_type[pop_type])

		pop_icon_buttons.push_back(pop_icon_button)

	return pop_icon_buttons

func _update_pop_icon_buttons(pop_icon_buttons : Array[GUIIconButton], pop_type_tooltips : PackedStringArray, pop_type_present_bools : PackedByteArray = [] as PackedByteArray) -> void:
	for pop_icon_button : GUIIconButton in pop_icon_buttons:
		# Default to tooltips array size so it'll default to having no tooltip
		var pop_type : int = pop_icon_button.get_meta(pop_icon_type_meta_key, pop_type_tooltips.size())
		pop_icon_button.set_tooltip_string(
			pop_type_tooltips[pop_type] if pop_type < pop_type_tooltips.size() else ""
		)
		pop_icon_button.set_disabled(not pop_type_present_bools[pop_type] if pop_type < pop_type_present_bools.size() else false)

func _setup_spending_subcategories(country_budget_panel : Panel, starting_pos : Vector2, spending_subcategories : PackedStringArray, wrap_x : int = 0) -> Array[GUILabel]:
	var spending_subcategory_labels : Array[GUILabel]

	if wrap_x < 1:
		wrap_x = spending_subcategories.size()

	var current_pos : Vector2 = starting_pos
	var max_width : float = 0

	for index : int in spending_subcategories.size():
		var subcategory_panel : Panel = GUINode.generate_gui_element(_gui_file, "exp_subcategory_window")
		if not subcategory_panel:
			break

		var subcategory_desc_label : GUILabel = get_gui_label_from_node_and_path(subcategory_panel, ^"./desc")
		if subcategory_desc_label:
			subcategory_desc_label.set_text(spending_subcategories[index])

		spending_subcategory_labels.push_back(get_gui_label_from_node_and_path(subcategory_panel, ^"./value"))

		subcategory_panel.set_anchors_and_offsets_preset(PRESET_TOP_LEFT)

		if index % wrap_x == 0:
			starting_pos.x += max_width
			max_width = 0
			current_pos = starting_pos

		subcategory_panel.set_position(current_pos)
		var subcategory_panel_size : Vector2 = subcategory_panel.get_custom_minimum_size()
		max_width = max(max_width, subcategory_panel_size.x)
		current_pos.y += subcategory_panel_size.y

		country_budget_panel.add_child(subcategory_panel)

	return spending_subcategory_labels

func _update_spending_subcategories(spending_subcategory_labels : Array[GUILabel], spending_subcategory_values : PackedFloat32Array) -> void:
	for index : int in spending_subcategory_labels.size():
		var spending_subcategory_label : GUILabel = spending_subcategory_labels[index]
		if not spending_subcategory_label:
			continue
		spending_subcategory_label.set_text("%s¤" % GUINode.float_to_string_dp(
			spending_subcategory_values[index] if index < spending_subcategory_values.size() else 0, 1
		))

func _update_info() -> void:
	if _active:

		# TODO - some values go red when they cannot be paid: national stockpile today + tomorrow, education, administration, social, military spending
		# The non-national stockpile expenses also have this added to their tooltips: "We can only afford to pay X£ of our expenses!" ("EXPENSE_NO_AFFORD", "$VAL$")

		var budget_info : Dictionary = MenuSingleton.get_budget_menu_info(
			_strata_tax_sliders,
			_land_spending_slider,
			_naval_spending_slider,
			_construction_spending_slider,
			_education_spending_slider,
			_administration_spending_slider,
			_social_spending_slider,
			_military_spending_slider,
			_tariff_slider
		)

		const pop_type_needs_tooltips_key : StringName = &"pop_type_needs_tooltips"
		const pop_type_present_bools_key : StringName = &"pop_type_present_bools"

		var pop_type_needs_tooltips : PackedStringArray = budget_info.get(pop_type_needs_tooltips_key, [] as PackedStringArray)
		var pop_type_present_bools : PackedByteArray = budget_info.get(pop_type_present_bools_key, [] as PackedByteArray)

		# Tax
		const tax_info_by_strata_key : StringName = &"tax_info_by_strata"
		# Per strata:
		const strata_needs_pie_chart_key : StringName = &"strata_needs_pie_chart"
		const strata_tax_value_key : StringName = &"strata_tax_value"
		const strata_tax_value_tooltip_key : StringName = &"strata_tax_value_tooltip"

		var tax_info_by_strata : Array[Dictionary] = budget_info.get(tax_info_by_strata_key, [] as Array[Dictionary])

		for index : int in _strata_tax_piecharts.size():
			var strata_tax_info : Dictionary = tax_info_by_strata[index] if index < tax_info_by_strata.size() else {}

			var strata_tax_piechart : GUIPieChart = _strata_tax_piecharts[index]
			if strata_tax_piechart:
				strata_tax_piechart.set_slices_array(strata_tax_info.get(strata_needs_pie_chart_key, []))
			_update_pop_icon_buttons(_strata_tax_pop_icon_buttons[index], pop_type_needs_tooltips, pop_type_present_bools)
			var strata_tax_label : GUILabel = _strata_tax_labels[index]
			if strata_tax_label:
				var value : float = strata_tax_info.get(strata_tax_value_key, 0)
				strata_tax_label.set_text("%s¤" % GUINode.float_to_string_dp(value, 3 if abs(value) < 1000 else 1))
				strata_tax_label.set_tooltip_string(strata_tax_info.get(strata_tax_value_tooltip_key, ""))

		# Gold
		const gold_key : StringName = &"gold"
		const gold_tooltip_key : StringName = &"gold_tooltip"

		if _gold_label:
			_gold_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(gold_key, 0), 1))
			_gold_label.set_tooltip_string(budget_info.get(gold_tooltip_key, ""))

		# Total Income
		const total_income_key : StringName = &"total_income"
		const total_income_tooltip_key : StringName = &"total_income_tooltip"

		if _total_income_label:
			_total_income_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(total_income_key, 0)))
			_total_income_label.set_tooltip_string(budget_info.get(total_income_tooltip_key, ""))

		# Funds
		const national_bank_key : StringName = &"national_bank"
		const national_bank_tooltip_key : StringName = &"national_bank_tooltip"
		const total_funds_key : StringName = &"total_funds"

		var national_bank_tooltip_string : String = budget_info.get(national_bank_tooltip_key, "")

		if _national_bank_desc_label:
			_national_bank_desc_label.set_tooltip_string(national_bank_tooltip_string)
		if _national_bank_value_label:
			_national_bank_value_label.set_text("%s¤" % GUINode.float_to_string_suffixed(budget_info.get(national_bank_key, 0)))
			_national_bank_value_label.set_tooltip_string(national_bank_tooltip_string)
		if _total_funds_value_label:
			_total_funds_value_label.set_text("%s¤" % GUINode.float_to_string_suffixed(budget_info.get(total_funds_key, 0)))

		# Debt
		const total_debt_key : StringName = &"total_debt"
		const interest_key : StringName = &"interest"
		const repaying_debts_key : StringName = &"repaying_debts"
		const loans_taken_key : StringName = &"loans_taken"
		const loans_given_key : StringName = &"loans_given"

		if _total_debt_label:
			_total_debt_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(total_debt_key, 0), 1))
		if _interest_label:
			_interest_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(interest_key, 0), 2))
		if _repaying_debts_icon:
			# TODO - _repaying_debts_icon tooltip
			_repaying_debts_icon.set_visible(budget_info.get(repaying_debts_key, false))
		# TODO - update loans pie chart and table with loans_taken_key and loans_given_key

		# Industrial Subsidies
		const industrial_subsidies_key : StringName = &"industrial_subsidies"
		const industrial_subsidies_tooltip_key : StringName = &"industrial_subsidies_tooltip"

		if _industrial_subsidies_label:
			_industrial_subsidies_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(industrial_subsidies_key, 0), 1))
			_industrial_subsidies_label.set_tooltip_string(budget_info.get(industrial_subsidies_tooltip_key, ""))

		# National Stockpile
		const military_costs_key : StringName = &"military_costs"
		const military_costs_tooltip_key : StringName = &"military_costs_tooltip"
		const overseas_maintenance_key : StringName = &"overseas_maintenance"
		const overseas_maintenance_tooltip_key : StringName = &"overseas_maintenance_tooltip"
		const national_stockpile_today_key : StringName = &"national_stockpile_today"
		const national_stockpile_today_tooltip_key : StringName = &"national_stockpile_today_tooltip"
		const national_stockpile_tomorrow_key : StringName = &"national_stockpile_tomorrow"
		const national_stockpile_tomorrow_tooltip_key : StringName = &"national_stockpile_tomorrow_tooltip"

		if _military_costs_label:
			# TODO - make this 2dp, k for over thousand
			_military_costs_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(military_costs_key, 0), 2))
			_military_costs_label.set_tooltip_string(budget_info.get(military_costs_tooltip_key, ""))
		if _overeseas_maintenance_label:
			_overeseas_maintenance_label.set_text("%s¤" % GUINode.float_to_string_dp(budget_info.get(overseas_maintenance_key, 0), 1))
			_overeseas_maintenance_label.set_tooltip_string(budget_info.get(overseas_maintenance_tooltip_key, ""))
		if _national_stockpile_today_label:
			_national_stockpile_today_label.set_text(budget_info.get(national_stockpile_today_key, "0.000¤"))
			_national_stockpile_today_label.set_tooltip_string(budget_info.get(national_stockpile_today_tooltip_key, ""))
		if _national_stockpile_tomorrow_label:
			_national_stockpile_tomorrow_label.set_text(budget_info.get(national_stockpile_tomorrow_key, "0.000¤"))
			_national_stockpile_tomorrow_label.set_tooltip_string(budget_info.get(national_stockpile_tomorrow_tooltip_key, ""))

		# Education
		const education_spending_value_key : StringName = &"education_spending"

		_update_pop_icon_buttons(_education_spending_pop_icon_buttons, pop_type_needs_tooltips)
		if _education_spending_value_label:
			_education_spending_value_label.set_text(budget_info.get(education_spending_value_key, "0.000¤"))

		# Administration
		const administrative_efficiency_key : StringName = &"administrative_efficiency"
		const administrative_efficiency_tooltip_key : StringName = &"administrative_efficiency_tooltip"
		const administration_spending_value_key : StringName = &"administration_spending_value"

		if _administrative_efficiency_label:
			_administrative_efficiency_label.set_text("%s%%"% GUINode.float_to_string_dp(budget_info.get(administrative_efficiency_key, 0), 1))
			_administrative_efficiency_label.set_tooltip_string(budget_info.get(administrative_efficiency_tooltip_key, ""))
		_update_pop_icon_buttons(_administration_spending_pop_icon_buttons, pop_type_needs_tooltips)
		if _administration_spending_value_label:
			_administration_spending_value_label.set_text(budget_info.get(administration_spending_value_key, "0.000¤"))

		# Social Spending
		const social_spending_value_key : StringName = &"social_spending_value"
		const social_spending_subcategories_key : StringName = &"social_spending_subcategories"

		if _social_spending_value_label:
			_social_spending_value_label.set_text(budget_info.get(social_spending_value_key, "0.000¤"))
		_update_spending_subcategories(_social_spending_subcategory_labels, budget_info.get(social_spending_subcategories_key, [] as PackedFloat32Array))

		# Miltiary Spending
		const military_spending_value_key : StringName = &"military_spending_value"
		const military_spending_subcategories_key : StringName = &"military_spending_subcategories"

		_update_pop_icon_buttons(_military_spending_pop_icon_buttons, pop_type_needs_tooltips)
		if _military_spending_value_label:
			_military_spending_value_label.set_text(budget_info.get(military_spending_value_key, "0.000¤"))
		_update_spending_subcategories(_military_spending_subcategory_labels, budget_info.get(military_spending_subcategories_key, [] as PackedFloat32Array))

		# Total Expense
		const total_expense_key : StringName = &"total_expense"
		const total_expense_tooltip_key : StringName = &"total_expense_tooltip"

		if _total_expense_label:
			_total_expense_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(total_expense_key, 0)))
			_total_expense_label.set_tooltip_string(budget_info.get(total_expense_tooltip_key, ""))

		# Tariffs
		const tariff_value_key : StringName = &"tariff_value"

		if _tariff_percentage_label:
			_tariff_percentage_label.set_text("%s%%" % GUINode.float_to_string_dp(_tariff_slider.get_value_scaled() if _tariff_slider else 0, 1))
		if _tariff_value_label:
			_tariff_value_label.set_text("%s¤" % GUINode.float_to_string_dp_dynamic(budget_info.get(tariff_value_key, 0)))

		# Diplomatic Balance
		const diplomatic_balance_key : StringName = &"diplomatic_balance"
		const diplomatic_balance_tooltip_key : StringName = &"diplomatic_balance_tooltip"

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
		const projected_daily_balance_key : StringName = &"projected_daily_balance"

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
