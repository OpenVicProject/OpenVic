extends GUINode

# Country info
var _country_flag_button : GUIMaskedFlagButton
var _country_flag_overlay_icon : GUIIcon
var _country_name_label : GUILabel
var _country_rank_label : GUILabel
var _country_prestige_label : GUILabel
var _country_prestige_rank_label : GUILabel
var _country_industrial_power_label : GUILabel
var _country_industrial_power_rank_label : GUILabel
var _country_military_power_label : GUILabel
var _country_military_power_rank_label : GUILabel
var _country_colonial_power_label : GUILabel

# Time controls
var _speed_up_button : GUIIconButton
var _speed_down_button : GUIIconButton
var _speed_indicator_button : GUIIconButton
var _date_label : GUILabel

# NationManagement.Screen-GUIIconButton
var _nation_management_buttons : Dictionary

# Production
var _production_top_goods_icons : Array[GUIIcon]
var _production_alert_building_icon : GUIIcon
var _production_alert_closed_icon : GUIIcon
var _production_alert_unemployment_icon : GUIIcon

# Budget
# TODO - line chart
var _budget_funds_label : GUILabel

# Technology
var _technology_progress_bar : GUIProgressBar
var _technology_current_research_label : GUILabel
var _technology_literacy_label : GUILabel
var _technology_research_points_label : GUILabel

# Politics
var _politics_party_icon : GUIIcon
var _politics_party_label : GUILabel
var _politics_suppression_points_label : GUILabel
var _politics_infamy_label : GUILabel
var _politics_reforms_button : GUIButton
var _politics_decisions_button : GUIIconButton
var _politics_election_icon : GUIIcon
var _politics_rebels_button : GUIIconButton

# Population
var _population_total_size_label : GUILabel
var _population_national_foci_label : GUILabel
var _population_militancy_label : GUILabel
var _population_consciousness_label : GUILabel

# Trade
var _trade_imported_icons : Array[GUIIcon]
var _trade_exported_icons : Array[GUIIcon]

# Diplomacy
var _diplomacy_peace_label : GUILabel
var _diplomacy_war_enemies_overlapping_elements_box : GUIOverlappingElementsBox
var _diplomacy_diplomatic_points_label : GUILabel
var _diplomacy_alert_colony_button : GUIIconButton
var _diplomacy_alert_crisis_icon : GUIIcon
var _diplomacy_alert_sphere_icon : GUIIcon
var _diplomacy_alert_great_power_icon : GUIIcon

# Military
var _military_army_size_label : GUILabel
var _military_navy_size_label : GUILabel
var _military_mobilisation_size_label : GUILabel
var _military_leadership_points_label : GUILabel

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)
	GameSingleton.clock_state_changed.connect(_update_speed_controls)

	add_gui_element("topbar", "topbar")

	hide_nodes([
		^"./topbar/topbar_outlinerbutton_bg",
		^"./topbar/topbar_outlinerbutton"
	])

	# Disables all consuming invisible panel
	var topbar : Panel = get_panel_from_nodepath(^"./topbar")
	if topbar:
		topbar.mouse_filter = Control.MOUSE_FILTER_IGNORE
	set_click_mask_from_nodepaths([^"./topbar/topbar_bg", ^"./topbar/topbar_paper"])

	# Country info
	_country_flag_button = get_gui_masked_flag_button_from_nodepath(^"./topbar/player_flag")
	if _country_flag_button:
		_country_flag_button.pressed.connect(
			func() -> void:
				# TODO - open the diplomacy menu on the Wars tab
				Events.NationManagementScreens.open_nation_management_screen(NationManagement.Screen.DIPLOMACY)
		)
	_country_flag_overlay_icon = get_gui_icon_from_nodepath(^"./topbar/topbar_flag_overlay")
	_country_name_label = get_gui_label_from_nodepath(^"./topbar/CountryName")
	_country_rank_label = get_gui_label_from_nodepath(^"./topbar/nation_totalrank")
	_country_prestige_label = get_gui_label_from_nodepath(^"./topbar/country_prestige")
	_country_prestige_rank_label = get_gui_label_from_nodepath(^"./topbar/selected_prestige_rank")
	_country_industrial_power_label = get_gui_label_from_nodepath(^"./topbar/country_economic")
	_country_industrial_power_rank_label = get_gui_label_from_nodepath(^"./topbar/selected_industry_rank")
	_country_military_power_label = get_gui_label_from_nodepath(^"./topbar/country_military")
	_country_military_power_rank_label = get_gui_label_from_nodepath(^"./topbar/selected_military_rank")
	_country_colonial_power_label = get_gui_label_from_nodepath(^"./topbar/country_colonial_power")

	# Time controls
	_speed_up_button = get_gui_icon_button_from_nodepath(^"./topbar/button_speedup")
	if _speed_up_button:
		_speed_up_button.pressed.connect(_on_increase_speed_button_pressed)
	_speed_down_button = get_gui_icon_button_from_nodepath(^"./topbar/button_speeddown")
	if _speed_down_button:
		_speed_down_button.pressed.connect(_on_decrease_speed_button_pressed)
	var pause_bg_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./topbar/pause_bg")
	if pause_bg_button:
		pause_bg_button.pressed.connect(_on_play_pause_button_pressed)
	_speed_indicator_button = get_gui_icon_button_from_nodepath(^"./topbar/speed_indicator")
	if _speed_indicator_button:
		_speed_indicator_button.pressed.connect(_on_play_pause_button_pressed)
	_date_label = get_gui_label_from_nodepath(^"./topbar/DateText")

	# Nation management screens
	const screen_nodepaths : Dictionary = {
		NationManagement.Screen.PRODUCTION : ^"./topbar/topbarbutton_production",
		NationManagement.Screen.BUDGET     : ^"./topbar/topbarbutton_budget",
		NationManagement.Screen.TECHNOLOGY : ^"./topbar/topbarbutton_tech",
		NationManagement.Screen.POLITICS   : ^"./topbar/topbarbutton_politics",
		NationManagement.Screen.POPULATION : ^"./topbar/topbarbutton_pops",
		NationManagement.Screen.TRADE      : ^"./topbar/topbarbutton_trade",
		NationManagement.Screen.DIPLOMACY  : ^"./topbar/topbarbutton_diplomacy",
		NationManagement.Screen.MILITARY   : ^"./topbar/topbarbutton_military"
	}
	for screen : NationManagement.Screen in screen_nodepaths:
		var button : GUIIconButton = get_gui_icon_button_from_nodepath(screen_nodepaths[screen])
		if button:
			button.pressed.connect(
				Events.NationManagementScreens.toggle_nation_management_screen.bind(screen)
			)
			_nation_management_buttons[screen] = button
	Events.NationManagementScreens.update_active_nation_management_screen.connect(
		_on_update_active_nation_management_screen
	)

	# Production
	const PRODUCED_GOOD_COUNT : int = 5
	for idx : int in PRODUCED_GOOD_COUNT:
		_production_top_goods_icons.push_back(get_gui_icon_from_nodepath("./topbar/topbar_produced%d" % idx))
	_production_alert_building_icon = get_gui_icon_from_nodepath(^"./topbar/alert_building_factories")
	_production_alert_closed_icon = get_gui_icon_from_nodepath(^"./topbar/alert_closed_factories")
	_production_alert_unemployment_icon = get_gui_icon_from_nodepath(^"./topbar/alert_unemployed_workers")

	# Budget
	_budget_funds_label = get_gui_label_from_nodepath(^"./topbar/budget_funds")

	# Technology
	_technology_progress_bar = get_gui_progress_bar_from_nodepath(^"./topbar/topbar_tech_progress")
	_technology_current_research_label = get_gui_label_from_nodepath(^"./topbar/tech_current_research")
	_technology_literacy_label = get_gui_label_from_nodepath(^"./topbar/tech_literacy_value")
	_technology_research_points_label = get_gui_label_from_nodepath(^"./topbar/topbar_researchpoints_value")

	# Politics
	_politics_party_icon = get_gui_icon_from_nodepath(^"./topbar/politics_party_icon")
	_politics_party_label = get_gui_label_from_nodepath(^"./topbar/politics_ruling_party")
	var politics_suppression_button : GUIIconButton = get_gui_icon_button_from_nodepath(^"./topbar/topbar_supression_icon")
	if politics_suppression_button:
		politics_suppression_button.pressed.connect(
			func() -> void:
				# TODO - open the politics menu on the Movements tab
				Events.NationManagementScreens.toggle_nation_management_screen(NationManagement.Screen.POLITICS)
		)
	_politics_suppression_points_label = get_gui_label_from_nodepath(^"./topbar/politics_supressionpoints_value")
	_politics_infamy_label = get_gui_label_from_nodepath(^"./topbar/politics_infamy_value")
	_politics_reforms_button = get_gui_icon_button_from_nodepath(^"./topbar/alert_can_do_reforms")
	if _politics_reforms_button:
		_politics_reforms_button.pressed.connect(
			func() -> void:
				# TODO - open the politics menu on the Reforms tab
				Events.NationManagementScreens.toggle_nation_management_screen(NationManagement.Screen.POLITICS)
		)
	_politics_decisions_button = get_gui_icon_button_from_nodepath(^"./topbar/alert_can_do_decisions")
	if _politics_decisions_button:
		_politics_decisions_button.pressed.connect(
			func() -> void:
				# TODO - open the politics menu on the Decisions tab
				Events.NationManagementScreens.toggle_nation_management_screen(NationManagement.Screen.POLITICS)
		)
	_politics_election_icon = get_gui_icon_from_nodepath(^"./topbar/alert_is_in_election")
	_politics_rebels_button = get_gui_icon_button_from_nodepath(^"./topbar/alert_have_rebels")
	if _politics_rebels_button:
		_politics_rebels_button.pressed.connect(
			func() -> void:
				# TODO - open the politics menu on the Movements tab
				Events.NationManagementScreens.toggle_nation_management_screen(NationManagement.Screen.POLITICS)
		)

	# Population
	_population_total_size_label = get_gui_label_from_nodepath(^"./topbar/population_total_value")
	_population_national_foci_label = get_gui_label_from_nodepath(^"./topbar/topbar_focus_value")
	_population_militancy_label = get_gui_label_from_nodepath(^"./topbar/population_avg_mil_value")
	_population_consciousness_label = get_gui_label_from_nodepath(^"./topbar/population_avg_con_value")

	# Trade
	const TRADE_GOOD_COUNT : int = 3
	for idx in TRADE_GOOD_COUNT:
		_trade_imported_icons.push_back(get_gui_icon_from_nodepath("./topbar/topbar_import%d" % idx))
		_trade_exported_icons.push_back(get_gui_icon_from_nodepath("./topbar/topbar_export%d" % idx))

	# Diplomacy
	_diplomacy_peace_label = get_gui_label_from_nodepath(^"./topbar/diplomacy_status")
	_diplomacy_war_enemies_overlapping_elements_box = get_gui_overlapping_elements_box_from_nodepath(^"./topbar/diplomacy_at_war")
	_diplomacy_diplomatic_points_label = get_gui_label_from_nodepath(^"./topbar/diplomacy_diplopoints_value")
	_diplomacy_alert_colony_button = get_gui_icon_button_from_nodepath(^"./topbar/alert_colony")
	if _diplomacy_alert_colony_button:
		_diplomacy_alert_colony_button.pressed.connect(
			func() -> void:
				# TODO - move to and select province in upgradable colony if any exist
				Events.NationManagementScreens.open_nation_management_screen(NationManagement.Screen.DIPLOMACY)
		)
	_diplomacy_alert_crisis_icon = get_gui_icon_from_nodepath(^"./topbar/alert_crisis")
	_diplomacy_alert_sphere_icon = get_gui_icon_from_nodepath(^"./topbar/alert_can_increase_opinion")
	_diplomacy_alert_great_power_icon = get_gui_icon_from_nodepath(^"./topbar/alert_loosing_gp")

	# Military
	_military_army_size_label = get_gui_label_from_nodepath(^"./topbar/military_army_value")
	_military_navy_size_label = get_gui_label_from_nodepath(^"./topbar/military_navy_value")
	_military_mobilisation_size_label = get_gui_label_from_nodepath(^"./topbar/military_manpower_value")
	_military_leadership_points_label = get_gui_label_from_nodepath(^"./topbar/military_leadership_value")

	_update_info()
	_update_speed_controls()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()

func _update_info() -> void:
	# Placeholder data
	const player_country : String = "ENG"

	## Country info
	if _country_flag_button:
		_country_flag_button.set_flag_country_name(player_country)

	if _country_flag_overlay_icon:
		# 1 - Great Power
		# 2 - Secondary Power
		# 3 - Civilised
		# 4 - Uncivilised
		_country_flag_overlay_icon.set_icon_index(1)

	if _country_name_label:
		_country_name_label.set_text(player_country)

	if _country_rank_label:
		_country_rank_label.set_text(str(1))

	if _country_prestige_label:
		_country_prestige_label.set_text(str(11))

	if _country_prestige_rank_label:
		_country_prestige_rank_label.set_text(str(1))

	if _country_industrial_power_label:
		_country_industrial_power_label.set_text(str(22))

	if _country_industrial_power_rank_label:
		_country_industrial_power_rank_label.set_text(str(2))

	if _country_military_power_label:
		_country_military_power_label.set_text(str(33))

	if _country_military_power_rank_label:
		_country_military_power_rank_label.set_text(str(3))

	if _country_colonial_power_label:
		var available_colonial_power : int = 123
		var total_colonial_power : int = 456
		_country_colonial_power_label.set_text(
			"§%s%s§!/%s" % ["W" if available_colonial_power > 0 else "R", available_colonial_power, total_colonial_power]
		)

	## Time control
	if _date_label:
		_date_label.text = MenuSingleton.get_longform_date()

	## Production
	for idx : int in _production_top_goods_icons.size():
		if _production_top_goods_icons[idx]:
			_production_top_goods_icons[idx].set_icon_index(idx + 2)

	if _production_alert_building_icon:
		_production_alert_building_icon.set_icon_index(2)

	if _production_alert_closed_icon:
		_production_alert_closed_icon.set_icon_index(2)

	if _production_alert_unemployment_icon:
		_production_alert_unemployment_icon.set_icon_index(2)

	## Budget
	if _budget_funds_label:
		var cash : float = 0.0
		var earnings : float = 0.0
		_budget_funds_label.set_text("§Y%s§!¤(§%s%s§!¤)" % [
			GUINode.float_to_string_suffixed(cash),
			"G+" if earnings > 0.0 else "R" if earnings < 0.0 else "Y+",
			GUINode.float_to_string_suffixed(earnings)
		])

	## Technology
	if _technology_progress_bar:
		pass # TODO - set tech progress

	if _technology_current_research_label:
		# TODO - set current research or "unciv_nation" (in red) if uncivilised
		_technology_current_research_label.set_text("TB_TECH_NO_CURRENT")

	if _technology_literacy_label:
		_technology_literacy_label.set_text("§Y%s§W%%" % GUINode.float_to_string_dp(80.0, 1))

	if _technology_research_points_label:
		_technology_research_points_label.set_text("§Y%s" % GUINode.float_to_string_dp(10.0, 2))

	## Politics
	if _politics_party_icon:
		_politics_party_icon.set_modulate(Color(1.0, 1.0, 0.0))

	if _politics_party_label:
		_politics_party_label.set_text("ENG_liberal")

	if _politics_suppression_points_label:
		_politics_suppression_points_label.set_text("§Y%s" % GUINode.float_to_string_dp(2.5, 1))

	if _politics_infamy_label:
		_politics_infamy_label.set_text("§Y%s" % GUINode.float_to_string_dp(0.0, 2))

	if _politics_reforms_button:
		_politics_reforms_button.set_icon_index(2)

	if _politics_decisions_button:
		_politics_decisions_button.set_icon_index(2)

	if _politics_election_icon:
		_politics_election_icon.set_icon_index(2)

	if _politics_rebels_button:
		_politics_rebels_button.set_icon_index(2)

	## Population
	if _population_total_size_label:
		# TODO - suffixes on both numbers should be white!
		var total_population : int = 16000000
		var growth : int = 1500
		_population_total_size_label.set_text("§Y%s§!(§%s%s§!)" % [
			GUINode.int_to_string_suffixed(total_population),
			"G" if growth >= 0 else "R",
			GUINode.int_to_string_suffixed(growth),
		])

	if _population_national_foci_label:
		var foci_used : int = 1
		var max_foci : int = 1
		_population_national_foci_label.set_text("§%s%d/%d" % ["R" if foci_used < max_foci else "G", foci_used, max_foci])

	if _population_militancy_label:
		_population_militancy_label.set_text("§Y%s" % GUINode.float_to_string_dp(1.5, 2))

	if _population_consciousness_label:
		_population_consciousness_label.set_text("§Y%s" % GUINode.float_to_string_dp(0.05, 2))

	## Trade
	for idx : int in _trade_imported_icons.size():
		if _trade_imported_icons[idx]:
			_trade_imported_icons[idx].set_icon_index(idx + 2 + _production_top_goods_icons.size())

	for idx : int in _trade_exported_icons.size():
		if _trade_exported_icons[idx]:
			_trade_exported_icons[idx].set_icon_index(idx + 2 + _production_top_goods_icons.size() + _trade_imported_icons.size())

	## Diplomacy
	if _diplomacy_peace_label:
		_diplomacy_peace_label.set_text("TOPBAR_AT_PEACE")

	# TODO - add war enemy flags to _diplomacy_war_enemies_overlapping_elements_box

	if _diplomacy_diplomatic_points_label:
		_diplomacy_diplomatic_points_label.set_text("§Y%s" % GUINode.float_to_string_dp(7.4, 0))

	if _diplomacy_alert_colony_button:
		_diplomacy_alert_colony_button.set_icon_index(3)

	if _diplomacy_alert_crisis_icon:
		_diplomacy_alert_crisis_icon.set_icon_index(3)

	if _diplomacy_alert_sphere_icon:
		_diplomacy_alert_sphere_icon.set_icon_index(2)

	if _diplomacy_alert_great_power_icon:
		_diplomacy_alert_great_power_icon.set_icon_index(2)

	## Military
	if _military_army_size_label:
		_military_army_size_label.set_text("§Y%d/%d" % [57, 120])

	if _military_navy_size_label:
		_military_navy_size_label.set_text("§Y%d/%d" % [123, 267])

	if _military_mobilisation_size_label:
		_military_mobilisation_size_label.set_text("§Y%d" % 38)

	if _military_leadership_points_label:
		_military_leadership_points_label.set_text("§Y%d" % 15)

func _update_speed_controls() -> void:
	#  TODO - decide whether to disable these or not
	# (they don't appear to get disabled in the base game)
	#if _speed_up_button:
	#	_speed_up_button.disabled = not MenuSingleton.can_increase_speed()

	#if _speed_down_button:
	#	_speed_down_button.disabled = not MenuSingleton.can_decrease_speed()

	if _speed_indicator_button:
		var index : int = 1
		if not MenuSingleton.is_paused():
			index += MenuSingleton.get_speed() + 1
		_speed_indicator_button.set_icon_index(index)

# REQUIREMENTS:
# * UIFUN-71
func _on_play_pause_button_pressed() -> void:
	print("Toggling pause!")
	MenuSingleton.toggle_paused()

# REQUIREMENTS:
# * UIFUN-72
func _on_increase_speed_button_pressed() -> void:
	print("Speed up!")
	MenuSingleton.increase_speed()

# REQUIREMENTS:
# * UIFUN-73
func _on_decrease_speed_button_pressed() -> void:
	print("Speed down!")
	MenuSingleton.decrease_speed()

func _on_update_active_nation_management_screen(active_screen : NationManagement.Screen) -> void:
	for screen : NationManagement.Screen in _nation_management_buttons:
		_nation_management_buttons[screen].set_icon_index(1 + int(screen == active_screen))
