extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.MILITARY

const _gui_file : String = "country_military"

# Military stats
var _war_exhaustion_label : GUILabel
var _supply_consumption_label : GUILabel
var _organisation_regain_label : GUILabel
var _land_organisation_label : GUILabel
var _naval_organisation_label : GUILabel
var _unit_start_experience_label : GUILabel
var _recruit_time_label : GUILabel
var _combat_width_label : GUILabel
var _dig_in_cap_label : GUILabel
var _military_tactics_label : GUILabel

# Mobilisation
var _mobilise_button : GUIIconButton
var _demobilise_button : GUIIconButton
var _mobilisation_progress_bar : GUIProgressBar
var _mobilisation_progress_label : GUILabel
var _mobilisation_size_label : GUILabel
var _mobilisation_economy_impact_label : GUILabel

# Leaders
var _general_count_label : GUILabel
var _admiral_count_label : GUILabel
var _create_general_button : GUIIconButton
var _create_admiral_button : GUIIconButton
var _auto_create_leader_button : GUIIconButton
var _auto_assign_leader_button : GUIIconButton
var _leader_listbox : GUIListBox
var _leader_sort_key : MenuSingleton.LeaderSortKey = MenuSingleton.LeaderSortKey.LEADER_SORT_NONE
var _leader_sort_descending : bool = true

# Armies and Navies
var _army_count_label : GUILabel
var _in_progress_brigade_count_label : GUILabel
var _disarmed_army_icon : GUIIcon
var _build_army_button : GUIIconButton
var _army_listbox : GUIListBox
var _army_sort_key : MenuSingleton.UnitGroupSortKey = MenuSingleton.UnitGroupSortKey.UNIT_GROUP_SORT_NONE
var _army_sort_descending : bool = true

var _navy_count_label : GUILabel
var _in_progress_ship_count_label : GUILabel
var _disarmed_navy_icon : GUIIcon
var _build_navy_button : GUIIconButton
var _navy_listbox : GUIListBox
var _navy_sort_key : MenuSingleton.UnitGroupSortKey = MenuSingleton.UnitGroupSortKey.UNIT_GROUP_SORT_NONE
var _navy_sort_descending : bool = true

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element(_gui_file, "country_military")

	var military_menu : Panel = get_panel_from_nodepath(^"./country_military")
	if not military_menu:
		return

	set_click_mask_from_nodepaths([^"./country_military/main_bg"])

	var close_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(military_menu.get_node(^"./close_button"))
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	# Military stats
	var stats_panel : Panel = GUINode.get_panel_from_node(military_menu.get_node(^"./stats"))
	if stats_panel:
		_war_exhaustion_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./war_exhaustion"))
		_supply_consumption_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./supply_consumption"))
		_organisation_regain_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./org_regain"))
		_land_organisation_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./army_org"))
		_naval_organisation_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./navy_org"))
		_unit_start_experience_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./unit_experience"))
		_recruit_time_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./recruit_time"))
		_combat_width_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./combat_width"))
		_dig_in_cap_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./digin_cap"))
		_military_tactics_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./tactics_level"))

	# Mobilisation
	_mobilise_button = GUINode.get_gui_icon_button_from_node(military_menu.get_node(^"./mobilize"))
	if _mobilise_button:
		_mobilise_button.pressed.connect(func() -> void: print("MOBILISE PRESSED"))
	_demobilise_button = GUINode.get_gui_icon_button_from_node(military_menu.get_node(^"./demobilize"))
	if _demobilise_button:
		_demobilise_button.pressed.connect(func() -> void: print("DEMOBILISE PRESSED"))
		_demobilise_button.set_tooltip_string("$MILITARY_DEMOBILIZE$" + MenuSingleton.get_tooltip_separator() + "$MILITARY_DEMOBILIZE_DESC$")
	_mobilisation_progress_bar = GUINode.get_gui_progress_bar_from_node(military_menu.get_node(^"./mobilize_progress"))
	_mobilisation_progress_label = GUINode.get_gui_label_from_node(military_menu.get_node(^"./mobilize_progress_text"))
	_mobilisation_size_label = GUINode.get_gui_label_from_node(military_menu.get_node(^"./mob_size"))
	_mobilisation_economy_impact_label = GUINode.get_gui_label_from_node(military_menu.get_node(^"./mob_impact"))

	# Leaders
	var leaders_panel : Panel = GUINode.get_panel_from_node(military_menu.get_node(^"./leaders"))
	if leaders_panel:
		_general_count_label = GUINode.get_gui_label_from_node(leaders_panel.get_node(^"./generals"))
		_admiral_count_label = GUINode.get_gui_label_from_node(leaders_panel.get_node(^"./admirals"))
		var sort_leader_prestige_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./sort_leader_prestige"))
		if sort_leader_prestige_button:
			sort_leader_prestige_button.pressed.connect(
				func() -> void:
					_leader_sort_key = MenuSingleton.LeaderSortKey.LEADER_SORT_PRESTIGE
					_leader_sort_descending = not _leader_sort_descending
					print("SORT LEADERS BY PRESTIGE ", "DESCENDING" if _leader_sort_descending else "ASCENDING")
					_update_info()
			)
			sort_leader_prestige_button.set_tooltip_string("SORT_BY_PRESTIGE")
		var sort_leader_type_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./sort_leader_type"))
		if sort_leader_type_button:
			sort_leader_type_button.pressed.connect(
				func() -> void:
					_leader_sort_key = MenuSingleton.LeaderSortKey.LEADER_SORT_TYPE
					_leader_sort_descending = not _leader_sort_descending
					print("SORT LEADERS BY TYPE ", "DESCENDING" if _leader_sort_descending else "ASCENDING")
					_update_info()
			)
			sort_leader_type_button.set_tooltip_string("MILITARY_SORT_BY_TYPE_TOOLTIP")
		var sort_leader_name_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./sort_leader_name"))
		if sort_leader_name_button:
			sort_leader_name_button.pressed.connect(
				func() -> void:
					_leader_sort_key = MenuSingleton.LeaderSortKey.LEADER_SORT_NAME
					_leader_sort_descending = not _leader_sort_descending
					print("SORT LEADERS BY NAME ", "DESCENDING" if _leader_sort_descending else "ASCENDING")
					_update_info()
			)
			sort_leader_name_button.set_tooltip_string("MILITARY_SORT_BY_NAME_TOOLTIP")
		var sort_leader_army_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./sort_leader_army"))
		if sort_leader_army_button:
			sort_leader_army_button.set_text("MILITARY_SORT_ARMY")
			sort_leader_army_button.pressed.connect(
				func() -> void:
					_leader_sort_key = MenuSingleton.LeaderSortKey.LEADER_SORT_ASSIGNMENT
					_leader_sort_descending = not _leader_sort_descending
					print("SORT LEADERS BY ASSIGNMENT ", "DESCENDING" if _leader_sort_descending else "ASCENDING")
					_update_info()
			)
			sort_leader_army_button.set_tooltip_string("MILITARY_SORT_BY_ASSIGNMENT_TOOLTIP")
		_create_general_button = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./new_general"))
		if _create_general_button:
			_create_general_button.pressed.connect(func() -> void: print("CREATE GENERAL"))
		_create_admiral_button = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./new_admiral"))
		if _create_admiral_button:
			_create_admiral_button.pressed.connect(func() -> void: print("CREATE ADMIRAL"))
		_auto_create_leader_button = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./auto_create"))
		if _auto_create_leader_button:
			_auto_create_leader_button.toggled.connect(func(state : bool) -> void: print("AUTO CREATE LEADERS = ", state))
			_auto_create_leader_button.set_tooltip_string("MILITARY_AUTOCREATE_TOOLTIP")
		_auto_assign_leader_button = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./auto_assign"))
		if _auto_assign_leader_button:
			_auto_assign_leader_button.toggled.connect(func(state : bool) -> void: print("AUTO ASSIGN LEADERS = ", state))
			_auto_assign_leader_button.set_tooltip_string("MILITARY_AUTOASSIGN_TOOLTIP")
		_leader_listbox = GUINode.get_gui_listbox_from_node(military_menu.get_node(^"./leaders/leader_listbox"))

	# Armies and Navies
	var army_pos : Vector2 = GUINode.get_gui_position(_gui_file, "army_pos")
	var army_window : Panel = GUINode.generate_gui_element(_gui_file, "unit_window")
	if army_window:
		army_window.set_position(army_pos)
		military_menu.add_child(army_window)

		_army_count_label = GUINode.get_gui_label_from_node(army_window.get_node(^"./current_count"))
		_in_progress_brigade_count_label = GUINode.get_gui_label_from_node(army_window.get_node(^"./under_construction"))
		_disarmed_army_icon = GUINode.get_gui_icon_from_node(army_window.get_node(^"./cut_down_to_size"))

		var sort_armies_name_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(army_window.get_node(^"./sort_name"))
		if sort_armies_name_button:
			sort_armies_name_button.pressed.connect(
				func() -> void:
					_army_sort_key = MenuSingleton.UnitGroupSortKey.UNIT_GROUP_SORT_NAME
					_army_sort_descending = not _army_sort_descending
					print("SORT ARMIES BY NAME ", "DESCENDING" if _army_sort_descending else "ASCENDING")
					_update_info()
			)
			sort_armies_name_button.set_tooltip_string("MILITARY_SORT_BY_NAME_TOOLTIP")
		var sort_armies_strength_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(army_window.get_node(^"./sort_strength"))
		if sort_armies_strength_button:
			sort_armies_strength_button.pressed.connect(
				func() -> void:
					_army_sort_key = MenuSingleton.UnitGroupSortKey.UNIT_GROUP_SORT_STRENGTH
					_army_sort_descending = not _army_sort_descending
					print("SORT ARMIES BY STRENGTH ", "DESCENDING" if _army_sort_descending else "ASCENDING")
					_update_info()
			)
			sort_armies_strength_button.set_tooltip_string("MILITARY_SORT_BY_STRENGTH_TOOLTIP")

		_build_army_button = GUINode.get_gui_icon_button_from_node(army_window.get_node(^"./build_new"))
		if _build_army_button:
			_build_army_button.pressed.connect(func() -> void: "BUILD ARMY")
			_build_army_button.set_text("MILITARY_BUILD_ARMY")
			_build_army_button.set_tooltip_string("MILITARY_BUILD_ARMY_TOOLTIP")

		_army_listbox = GUINode.get_gui_listbox_from_node(army_window.get_node(^"./unit_listbox"))

	var navy_pos : Vector2 = GUINode.get_gui_position(_gui_file, "navy_pos")
	var navy_window : Panel = GUINode.generate_gui_element(_gui_file, "unit_window")
	if navy_window:
		navy_window.set_position(navy_pos)
		military_menu.add_child(navy_window)

		_navy_count_label = GUINode.get_gui_label_from_node(navy_window.get_node(^"./current_count"))
		_in_progress_ship_count_label = GUINode.get_gui_label_from_node(navy_window.get_node(^"./under_construction"))
		_disarmed_navy_icon = GUINode.get_gui_icon_from_node(navy_window.get_node(^"./cut_down_to_size"))

		var sort_navies_name_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(navy_window.get_node(^"./sort_name"))
		if sort_navies_name_button:
			sort_navies_name_button.pressed.connect(
				func() -> void:
					_navy_sort_key = MenuSingleton.UnitGroupSortKey.UNIT_GROUP_SORT_NAME
					_navy_sort_descending = not _navy_sort_descending
					print("SORT NAVIES BY NAME ", "DESCENDING" if _navy_sort_descending else "ASCENDING")
					_update_info()
			)
			sort_navies_name_button.set_tooltip_string("MILITARY_SORT_BY_NAME_TOOLTIP")
		var sort_navies_strength_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(navy_window.get_node(^"./sort_strength"))
		if sort_navies_strength_button:
			sort_navies_strength_button.pressed.connect(
				func() -> void:
					_navy_sort_key = MenuSingleton.UnitGroupSortKey.UNIT_GROUP_SORT_STRENGTH
					_navy_sort_descending = not _navy_sort_descending
					print("SORT NAVIES BY STRENGTH ", "DESCENDING" if _navy_sort_descending else "ASCENDING")
					_update_info()
			)
			sort_navies_strength_button.set_tooltip_string("MILITARY_SORT_BY_STRENGTH_TOOLTIP")

		_build_navy_button = GUINode.get_gui_icon_button_from_node(navy_window.get_node(^"./build_new"))
		if _build_navy_button:
			_build_navy_button.pressed.connect(func() -> void: "BUILD NAVY")
			_build_navy_button.set_text("MILITARY_BUILD_NAVY")
			_build_navy_button.set_tooltip_string("MILITARY_BUILD_NAVY_TOOLTIP")

		_navy_listbox = GUINode.get_gui_listbox_from_node(navy_window.get_node(^"./unit_listbox"))

	_update_info()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()

func _on_update_active_nation_management_screen(active_screen : NationManagement.Screen) -> void:
	_active = active_screen == _screen
	_update_info()

func _update_unit_group_list(listbox : GUIListBox, unit_groups : Array[Dictionary], in_progress_units : Array[Dictionary], is_army : bool) -> void:
	var total_entry_count : int = unit_groups.size() + in_progress_units.size()
	listbox.clear_children(total_entry_count)
	while listbox.get_child_count() < total_entry_count:
		var entry : Panel = GUINode.generate_gui_element(_gui_file, "unit_entry")
		if not entry:
			break
		if not is_army:
			var men_label : GUILabel = GUINode.get_gui_label_from_node(entry.get_node(^"./men"))
			if men_label:
				men_label.hide()
			var dig_in_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry.get_node(^"./digin"))
			if dig_in_icon:
				dig_in_icon.hide()
		listbox.add_child(entry)

	const military_info_unit_group_leader_picture_key : StringName = &"unit_group_leader_picture"
	const military_info_unit_group_leader_tooltip_key : StringName = &"unit_group_leader_tooltip"
	const military_info_unit_group_name_key           : StringName = &"unit_group_name"
	const military_info_unit_group_location_key       : StringName = &"unit_group_location"
	const military_info_unit_group_unit_count_key     : StringName = &"unit_group_unit_count"
	const military_info_unit_group_men_count_key      : StringName = &"unit_group_men_count" # armies only
	const military_info_unit_group_max_men_count_key  : StringName = &"unit_group_max_men_count" # armies only
	const military_info_unit_group_organisation_key   : StringName = &"unit_group_organisation"
	const military_info_unit_group_strength_key       : StringName = &"unit_group_strength"
	const military_info_unit_group_moving_tooltip_key : StringName = &"unit_group_moving_tooltip"
	const military_info_unit_group_dig_in_tooltip_key : StringName = &"unit_group_dig_in_tooltip" # armies only
	const military_info_unit_group_combat_key         : StringName = &"unit_group_combat"

	for index : int in mini(unit_groups.size(), listbox.get_child_count()):
		var entry_menu : Panel = GUINode.get_panel_from_node(listbox.get_child(index))
		var unit_group_dict : Dictionary = unit_groups[index]

		var entry_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_unit_entry_bg"))
		if entry_button:
			# TODO - sort out repeat connections!!!
			entry_button.pressed.connect(func() -> void: print("OPENING UNIT GROUP"))
		var unit_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./unit_progress"))
		if unit_progress_bar:
			unit_progress_bar.hide()
		var leader_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./leader"))
		if leader_icon:
			var leader_texture : Texture2D = unit_group_dict.get(military_info_unit_group_leader_picture_key, null)
			if leader_texture:
				leader_icon.show()
				leader_icon.set_texture(leader_texture)
				leader_icon.set_tooltip_string(unit_group_dict.get(military_info_unit_group_leader_tooltip_key, ""))
			else:
				leader_icon.hide()
		var unit_strip_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./unit_strip"))
		if unit_strip_icon:
			unit_strip_icon.hide()
		var name_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./name"))
		if name_label:
			name_label.set_text(unit_group_dict.get(military_info_unit_group_name_key, ""))
		var location_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./location"))
		if location_label:
			location_label.set_text(GUINode.format_province_name(unit_group_dict.get(military_info_unit_group_location_key, "")))
		var unit_eta_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./unit_eta"))
		if unit_eta_label:
			unit_eta_label.hide()
		var unit_count_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./regiments"))
		if unit_count_label:
			unit_count_label.show()
			var unit_count_string : String = str(unit_group_dict.get(military_info_unit_group_unit_count_key, 0))
			unit_count_label.set_text(unit_count_string)
			unit_count_label.set_tooltip_string(tr(&"MILITARY_REGIMENTS_TOOLTIP" if is_army else &"MILITARY_SHIPS_TOOLTIP").replace("$VALUE$", unit_count_string))
		var strength : float = unit_group_dict.get(military_info_unit_group_strength_key, 0)
		var strength_tooltip : String = tr(&"MILITARY_STRENGTH_TOOLTIP2" if is_army else &"MILITARY_SHIPSTRENGTH_TOOLTIP2").replace("$PERCENT$", str(int(strength * 100)))
		if is_army:
			var men_count : int = unit_group_dict.get(military_info_unit_group_men_count_key, 0) if is_army else 0
			var max_men_count : int = unit_group_dict.get(military_info_unit_group_max_men_count_key, 0) if is_army else 0
			var men_count_string : String = GUINode.int_to_string_commas(men_count) if is_army else ""
			strength_tooltip = strength_tooltip.replace("$VALUE$", men_count_string).replace("$MAX$", GUINode.int_to_string_commas(max_men_count))
			var men_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./men"))
			if men_label:
				men_label.show()
				men_label.set_text(men_count_string)
				men_label.set_tooltip_string(strength_tooltip)
		var military_cancel_unit_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_cancel_unit"))
		if military_cancel_unit_button:
			military_cancel_unit_button.hide()
		var organisation_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./morale_progress"))
		if organisation_progress_bar:
			organisation_progress_bar.show()
			var organisation : float = unit_group_dict.get(military_info_unit_group_organisation_key, 0.0)
			organisation_progress_bar.set_value_no_signal(organisation)
			organisation_progress_bar.set_tooltip_string(tr(&"MILITARY_MORALE_TOOLTIP").replace("$VALUE$", str(int(organisation * 100))))
		var strength_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./strength_progress"))
		if strength_progress_bar:
			strength_progress_bar.show()
			strength_progress_bar.set_value_no_signal(strength)
			strength_progress_bar.set_tooltip_string(strength_tooltip)
		var moving_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./moving"))
		if moving_icon:
			var moving_tooltip : String = unit_group_dict.get(military_info_unit_group_moving_tooltip_key, "")
			moving_icon.set_visible(not moving_tooltip.is_empty())
			moving_icon.set_tooltip_string(moving_tooltip)
		var dig_in_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./digin"))
		if is_army:
			if dig_in_icon:
				var dig_in_tooltip : String = unit_group_dict.get(military_info_unit_group_dig_in_tooltip_key, "")
				dig_in_icon.set_visible(not dig_in_tooltip.is_empty())
				dig_in_icon.set_tooltip_string(dig_in_tooltip)
		var combat_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./combat"))
		if combat_icon:
			combat_icon.set_visible(unit_group_dict.get(military_info_unit_group_combat_key, false))

	const military_info_unit_progress_key : StringName = &"unit_progress"
	const military_info_unit_icon_key : StringName = &"unit_icon"
	const military_info_unit_name_key : StringName = &"unit_name"
	const military_info_unit_location_key : StringName = &"unit_location"
	const military_info_unit_eta_key : StringName = &"unit_eta"
	const military_info_unit_tooltip_key : StringName = &"unit_tooltip"

	for index : int in clampi(listbox.get_child_count() - unit_groups.size(), 0, in_progress_units.size()):
		var entry_menu : Panel = GUINode.get_panel_from_node(listbox.get_child(index + unit_groups.size()))
		var unit_dict : Dictionary = in_progress_units[index]
		var unit_tooltip : String = unit_dict.get(military_info_unit_tooltip_key, "")

		var unit_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./unit_progress"))
		if unit_progress_bar:
			unit_progress_bar.show()
			unit_progress_bar.set_value_no_signal(unit_dict.get(military_info_unit_progress_key, 0))
			# This is enough to show the tooltip everywhere we need, the only place
			# in the base game that obviously also has it is the ETA date label
			unit_progress_bar.set_tooltip_string(unit_tooltip)
		var leader_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./leader"))
		if leader_icon:
			leader_icon.hide()
		var unit_strip_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./unit_strip"))
		if unit_strip_icon:
			unit_strip_icon.show()
			unit_strip_icon.set_icon_index(unit_dict.get(military_info_unit_icon_key, 0))
		var name_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./name"))
		if name_label:
			name_label.set_text(unit_dict.get(military_info_unit_name_key, ""))
		var location_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./location"))
		if location_label:
			location_label.set_text(GUINode.format_province_name(unit_dict.get(military_info_unit_location_key, "")))
		var unit_eta_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./unit_eta"))
		if unit_eta_label:
			unit_eta_label.show()
			unit_eta_label.set_text(unit_dict.get(military_info_unit_eta_key, ""))
		var unit_count_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./regiments"))
		if unit_count_label:
			unit_count_label.hide()
		if is_army:
			var men_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./men"))
			if men_label:
				men_label.hide()
		var military_cancel_unit_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_cancel_unit"))
		if military_cancel_unit_button:
			military_cancel_unit_button.show()
			# TODO - sort out repeat connections!!!
			military_cancel_unit_button.pressed.connect(func() -> void: print("CANCELLED UNIT CONSTRUCTION"))
		var organisation_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./morale_progress"))
		if organisation_progress_bar:
			organisation_progress_bar.hide()
		var strength_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./strength_progress"))
		if strength_progress_bar:
			strength_progress_bar.hide()
		var moving_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./moving"))
		if moving_icon:
			moving_icon.hide()
		if is_army:
			var dig_in_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./digin"))
			if dig_in_icon:
				dig_in_icon.hide()
		var combat_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./combat"))
		if combat_icon:
			combat_icon.hide()

func _update_info() -> void:
	if _active:
		# Military stats
		const military_info_war_exhaustion_key                : StringName = &"war_exhaustion"
		const military_info_war_exhaustion_tooltip_key        : StringName = &"war_exhaustion_tooltip"
		const military_info_supply_consumption_key            : StringName = &"supply_consumption"
		const military_info_supply_consumption_tooltip_key    : StringName = &"supply_consumption_tooltip"
		const military_info_organisation_regain_key           : StringName = &"organisation_regain"
		const military_info_organisation_regain_tooltip_key   : StringName = &"organisation_regain_tooltip"
		const military_info_land_organisation_key             : StringName = &"land_organisation"
		const military_info_land_organisation_tooltip_key     : StringName = &"land_organisation_tooltip"
		const military_info_naval_organisation_key            : StringName = &"naval_organisation"
		const military_info_naval_organisation_tooltip_key    : StringName = &"naval_organisation_tooltip"
		const military_info_land_unit_start_experience_key    : StringName = &"land_unit_start_experience"
		const military_info_naval_unit_start_experience_key   : StringName = &"naval_unit_start_experience"
		const military_info_unit_start_experience_tooltip_key : StringName = &"unit_start_experience_tooltip"
		const military_info_recruit_time_key                  : StringName = &"recruit_time"
		const military_info_recruit_time_tooltip_key          : StringName = &"recruit_time_tooltip"
		const military_info_combat_width_key                  : StringName = &"combat_width"
		const military_info_combat_width_tooltip_key          : StringName = &"combat_width_tooltip"
		const military_info_dig_in_cap_key                     : StringName = &"dig_in_cap"
		const military_info_military_tactics_key              : StringName = &"military_tactics"

		# Mobilisation
		const military_info_is_mobilised_key                  : StringName = &"is_mobilised"
		const military_info_mobilisation_progress_key         : StringName = &"mobilisation_progress"
		const military_info_mobilisation_size_key             : StringName = &"mobilisation_size"
		const military_info_mobilisation_size_tooltip_key     : StringName = &"mobilisation_size_tooltip"
		const military_info_mobilisation_impact_tooltip_key   : StringName = &"mobilisation_impact_tooltip"
		const military_info_mobilisation_economy_impact_key   : StringName = &"mobilisation_economy_impact"
		const military_info_mobilisation_economy_impact_tooltip_key : StringName = &"mobilisation_economy_impact_tooltip"

		# Leaders
		const military_info_general_count_key                 : StringName = &"general_count"
		const military_info_admiral_count_key                 : StringName = &"admiral_count"
		const military_info_create_leader_count_key           : StringName = &"create_leader_count"
		const military_info_create_leader_cost_key            : StringName = &"create_leader_cost"
		const military_info_auto_create_leaders_key           : StringName = &"auto_create_leaders"
		const military_info_auto_assign_leaders_key           : StringName = &"auto_assign_leaders"
		const military_info_leaders_list_key                  : StringName = &"leaders_list"

		# Armies and Navies
		const military_info_is_disarmed_key                   : StringName = &"is_disarmed"
		const military_info_armies_key                        : StringName = &"armies"
		const military_info_in_progress_brigades_key          : StringName = &"in_progress_brigades"
		const military_info_navies_key                        : StringName = &"navies"
		const military_info_in_progress_ships_key             : StringName = &"in_progress_ships"

		var military_info : Dictionary = MenuSingleton.get_military_menu_info(
			_leader_sort_key, _leader_sort_descending,
			_army_sort_key, _army_sort_descending,
			_navy_sort_key, _navy_sort_descending
		)

		# Military stats
		if _war_exhaustion_label:
			_war_exhaustion_label.set_text(military_info.get(military_info_war_exhaustion_key, "0.00/0.00"))
			_war_exhaustion_label.set_tooltip_string(military_info.get(military_info_war_exhaustion_tooltip_key, ""))
		if _supply_consumption_label:
			_supply_consumption_label.set_text("%d%%" % int(100 * military_info.get(military_info_supply_consumption_key, 0)))
			_supply_consumption_label.set_tooltip_string(military_info.get(military_info_supply_consumption_tooltip_key, ""))
		if _organisation_regain_label:
			_organisation_regain_label.set_text("%d%%" % int(100 * military_info.get(military_info_organisation_regain_key, 0)))
			_organisation_regain_label.set_tooltip_string(military_info.get(military_info_organisation_regain_tooltip_key, ""))
		if _land_organisation_label:
			_land_organisation_label.set_text("%d%%" % int(100 * military_info.get(military_info_land_organisation_key, 0)))
			_land_organisation_label.set_tooltip_string(military_info.get(military_info_land_organisation_tooltip_key, ""))
		if _naval_organisation_label:
			_naval_organisation_label.set_text("%d%%" % int(100 * military_info.get(military_info_naval_organisation_key, 0)))
			_naval_organisation_label.set_tooltip_string(military_info.get(military_info_naval_organisation_tooltip_key, ""))
		if _unit_start_experience_label:
			_unit_start_experience_label.set_text(
				"%s/%s" % [
					GUINode.float_to_string_dp(military_info.get(military_info_land_unit_start_experience_key, 0), 2),
					GUINode.float_to_string_dp(military_info.get(military_info_naval_unit_start_experience_key, 0), 2)
				]
			)
			_unit_start_experience_label.set_tooltip_string(military_info.get(military_info_unit_start_experience_tooltip_key, ""))
		if _recruit_time_label:
			_recruit_time_label.set_text("%d%%" % int(100 * military_info.get(military_info_recruit_time_key, 0)))
			_recruit_time_label.set_tooltip_string(military_info.get(military_info_recruit_time_tooltip_key, ""))
		if _combat_width_label:
			_combat_width_label.set_text(str(military_info.get(military_info_combat_width_key, 0)))
			_combat_width_label.set_tooltip_string(military_info.get(military_info_combat_width_tooltip_key, ""))
		if _dig_in_cap_label:
			_dig_in_cap_label.set_text(str(military_info.get(military_info_dig_in_cap_key, 0)))
		if _military_tactics_label:
			_military_tactics_label.set_text("%s%%" % GUINode.float_to_string_dp(100 * military_info.get(military_info_military_tactics_key, 0), 2))

		# Mobilisation
		var is_mobilised : bool = military_info.get(military_info_is_mobilised_key, false)
		var mobilisation_size : int = military_info.get(military_info_mobilisation_size_key, 0)
		var can_mobilise : bool = mobilisation_size > 0
		var mobilisation_size_tooltip : String = military_info.get(military_info_mobilisation_size_tooltip_key, "")
		var mobilisation_impact_tooltip : String = military_info.get(military_info_mobilisation_impact_tooltip_key, "")
		if _mobilise_button:
			_mobilise_button.set_visible(not is_mobilised)
			_mobilise_button.set_disabled(not can_mobilise)
			_mobilise_button.set_tooltip_string(
				tr(&"MILITARY_MOBILIZE") + "\n" + mobilisation_size_tooltip +
				("\n\n" + tr(&"NOT_ENOUGH_FOR_BRIGADE") + "\n\n" if not can_mobilise else "\n\n\n") +
				mobilisation_impact_tooltip + MenuSingleton.get_tooltip_separator() + tr(&"MILITARY_MOBILIZE_DESC")
			)
		if _demobilise_button:
			_demobilise_button.set_visible(is_mobilised)
		var mobilisation_progress : float = military_info.get(military_info_mobilisation_progress_key, 0)
		var mobilisation_progress_string : String = GUINode.float_to_string_dp(100 * mobilisation_progress, 2)
		if _mobilisation_progress_bar:
			_mobilisation_progress_bar.set_value_no_signal(mobilisation_progress)
			_mobilisation_progress_bar.set_tooltip_string(
				tr(&"MOBILIZATION_PROGRESS_PENDING").replace("$PROG$", mobilisation_progress_string) if is_mobilised else "MOBILIZATION_PROGRESS_NOT_MOBILIZED"
			)
		if _mobilisation_progress_label:
			_mobilisation_progress_label.set_text("%s%%" % mobilisation_progress_string)
		if _mobilisation_size_label:
			_mobilisation_size_label.set_text(str(mobilisation_size))
			_mobilisation_size_label.set_tooltip_string(
				mobilisation_size_tooltip + ("" if mobilisation_impact_tooltip.is_empty() else "\n\n\n" + mobilisation_impact_tooltip)
			)
		if _mobilisation_economy_impact_label:
			_mobilisation_economy_impact_label.set_text("%s%%" % GUINode.float_to_string_dp(100 * military_info.get(military_info_mobilisation_economy_impact_key, 0), 2))
			_mobilisation_economy_impact_label.set_tooltip_string(military_info.get(military_info_mobilisation_economy_impact_tooltip_key, ""))

		# Leaders
		if _general_count_label:
			_general_count_label.set_text(str(military_info.get(military_info_general_count_key, 0)))
		if _admiral_count_label:
			_admiral_count_label.set_text(str(military_info.get(military_info_admiral_count_key, 0)))
		var create_leader_count : int = military_info.get(military_info_create_leader_count_key, 0)
		var can_create_leaders : bool = create_leader_count > 0
		var create_leader_count_string : String = str(create_leader_count)
		var create_leader_cost_tooltip : String = tr(&"MILITARY_LACK_OF_LEADER").replace("$VALUE$", str(military_info.get(military_info_create_leader_cost_key, 0)))
		# TODO - leave the main text set and only update $VALUE$ using substitution dict functionality once buttons get proper text support
		if _create_general_button:
			_create_general_button.set_text(tr(&"MILITARY_CREATE_GENERAL").replace("$VALUE$", create_leader_count_string))
			_create_general_button.set_disabled(not can_create_leaders)
			_create_general_button.set_tooltip_string(
				tr(&"MILITARY_NEW_GENERAL_TOOLTIP").replace("$VALUE$", create_leader_count_string) if can_create_leaders else create_leader_cost_tooltip
			)
		if _create_admiral_button:
			_create_admiral_button.set_text(tr(&"MILITARY_CREATE_ADMIRAL").replace("$VALUE$", create_leader_count_string))
			_create_admiral_button.set_disabled(not can_create_leaders)
			_create_admiral_button.set_tooltip_string(
				tr(&"MILITARY_NEW_ADMIRAL_TOOLTIP").replace("$VALUE$", create_leader_count_string) if can_create_leaders else create_leader_cost_tooltip
			)
		## TODO - look into why set_pressed_no_signal didn't work
		if _auto_create_leader_button:
			_auto_create_leader_button.set_pressed(military_info.get(military_info_auto_create_leaders_key, false))
		if _auto_assign_leader_button:
			_auto_assign_leader_button.set_pressed(military_info.get(military_info_auto_assign_leaders_key, false))
		if _leader_listbox:
			var leader_entries : Array[Dictionary] = military_info.get(military_info_leaders_list_key, [] as Array[Dictionary])
			_leader_listbox.clear_children(leader_entries.size())
			while _leader_listbox.get_child_count() < leader_entries.size():
				var unit_entry : Panel = GUINode.generate_gui_element(_gui_file, "milview_leader_entry")
				if not unit_entry:
					break
				_leader_listbox.add_child(unit_entry)

			const military_info_leader_id_key : StringName = &"leader_id"
			const military_info_leader_name_key : StringName = &"leader_name"
			const military_info_leader_picture_key : StringName = &"leader_picture"
			const military_info_leader_prestige_key : StringName = &"leader_prestige"
			const military_info_leader_prestige_tooltip_key : StringName = &"leader_prestige_tooltip"
			const military_info_leader_background_key : StringName = &"leader_background"
			const military_info_leader_personality_key : StringName = &"leader_personality"
			const military_info_leader_can_be_used_key : StringName = &"leader_can_be_used"
			const military_info_leader_assignment_key : StringName = &"leader_assignment"
			const military_info_leader_location_key : StringName = &"leader_location"
			const military_info_leader_tooltip_key : StringName = &"leader_tooltip"

			for index : int in mini(leader_entries.size(), _leader_listbox.get_child_count()):
				var entry_menu : Panel = GUINode.get_panel_from_node(_leader_listbox.get_child(index))
				var leader_dict : Dictionary = leader_entries[index]

				var leader_id : int = leader_dict.get(military_info_leader_id_key, 0)
				if leader_id == 0:
					push_error("Leader ID is 0 or missing in leader dictionary for entry index ", index, ", skipping!")
					continue
				else:
					entry_menu.set_meta(military_info_leader_id_key, leader_id)

				var prestige_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./leader_prestige_bar"))
				if prestige_progress_bar:
					prestige_progress_bar.set_value_no_signal(leader_dict.get(military_info_leader_prestige_key, 0))
					prestige_progress_bar.set_tooltip_string(leader_dict.get(military_info_leader_prestige_tooltip_key, ""))

				var leader_name : String = leader_dict.get(military_info_leader_name_key, "")
				var leader_tooltip : String = leader_dict.get(military_info_leader_tooltip_key, "")

				var entry_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./military_leader_entry_bg"))
				if entry_icon:
					entry_icon.set_tooltip_string(leader_tooltip)
				var leader_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./leader"))
				if leader_icon:
					var leader_texture : Texture2D = leader_dict.get(military_info_leader_picture_key, null)
					if leader_texture:
						leader_icon.set_texture(leader_texture)
						leader_icon.show()
					else:
						leader_icon.hide()
					leader_icon.set_tooltip_string(leader_tooltip)
				var name_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./name"))
				if name_label:
					name_label.set_text(leader_name)
					name_label.set_tooltip_string(leader_tooltip)
				var background_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./background"))
				if background_label:
					background_label.set_text(leader_dict.get(military_info_leader_background_key, ""))
					background_label.set_tooltip_string(leader_tooltip)
				var personality_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./personality"))
				if personality_label:
					personality_label.set_text(leader_dict.get(military_info_leader_personality_key, ""))
					personality_label.set_tooltip_string(leader_tooltip)
				var use_leader_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./use_leader"))
				if use_leader_button:
					# TODO - investigate why "set_pressed_no_signal" wasn't enough
					use_leader_button.set_pressed(leader_dict.get(military_info_leader_can_be_used_key, false))
					# TODO - ensure only one connection?
					use_leader_button.toggled.connect(func(state : bool) -> void: print("Toggled use_leader to ", state))
					use_leader_button.set_tooltip_string("USE_LEADER" if use_leader_button.is_pressed() else "")

				var army_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./army"))
				var location_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./location"))

				var leader_assignment_untype : Variant = leader_dict.get(military_info_leader_assignment_key, null)
				if leader_assignment_untype != null:
					var leader_assignment : String = leader_assignment_untype as String
					var leader_location : String = GUINode.format_province_name(leader_dict.get(military_info_leader_location_key, ""), true)
					var assignment_tooltip : String = tr(&"MILITARY_LEADER_NAME_TOOLTIP").replace("$NAME$", leader_name).replace("$ARMY$", leader_assignment).replace("$LOCATION$", leader_location)

					if army_label:
						army_label.set_text(leader_assignment)
						army_label.set_tooltip_string(assignment_tooltip)
					if location_label:
						location_label.set_text(leader_location)
						location_label.set_tooltip_string(assignment_tooltip)
				else:
					if army_label:
						army_label.set_text("MILITARY_UNASSIGNED")
						army_label.clear_tooltip()
					if location_label:
						location_label.set_text("")
						location_label.clear_tooltip()

		# Armies and Navies
		var is_disarmed : bool = military_info.get(military_info_is_disarmed_key, false)

		var armies : Array[Dictionary] = military_info.get(military_info_armies_key, [] as Array[Dictionary])
		var in_progress_brigades : Array[Dictionary] = military_info.get(military_info_in_progress_brigades_key, [] as Array[Dictionary])
		if _army_count_label:
			var army_size_string : String = str(armies.size())
			_army_count_label.set_text(army_size_string)
			_army_count_label.set_tooltip_string(tr(&"MILITARY_ARMY_COUNT_TOOLTIP").replace("$VALUE$", army_size_string))
		if _in_progress_brigade_count_label:
			var constructing_brigades_count_string : String = str(in_progress_brigades.size())
			_in_progress_brigade_count_label.set_text("(+%s)" % constructing_brigades_count_string)
			_in_progress_brigade_count_label.set_tooltip_string(tr(&"MILITARY_ARMY_CONSTRUCTION_TOOLTIP").replace("$VALUE$", constructing_brigades_count_string))
		if _disarmed_army_icon:
			_disarmed_army_icon.set_visible(is_disarmed)
		if _build_army_button:
			_build_army_button.set_disabled(is_disarmed)
		if _army_listbox:
			_update_unit_group_list(_army_listbox, armies, in_progress_brigades, true)

		var navies : Array[Dictionary] = military_info.get(military_info_navies_key, [] as Array[Dictionary])
		var in_progress_ships : Array[Dictionary] = military_info.get(military_info_in_progress_ships_key, [] as Array[Dictionary])
		if _navy_count_label:
			var navy_size_string : String = str(navies.size())
			_navy_count_label.set_text(navy_size_string)
			_navy_count_label.set_tooltip_string(tr(&"MILITARY_NAVY_COUNT_TOOLTIP").replace("$VALUE$", navy_size_string))
		if _in_progress_ship_count_label:
			var constructing_ships_count_string : String = str(in_progress_ships.size())
			_in_progress_ship_count_label.set_text("(+%s)" % constructing_ships_count_string)
			_in_progress_ship_count_label.set_tooltip_string(tr(&"MILITARY_NAVY_CONSTRUCTION_TOOLTIP").replace("$VALUE$", constructing_ships_count_string))
		if _disarmed_navy_icon:
			_disarmed_navy_icon.set_visible(is_disarmed)
		if _build_navy_button:
			_build_navy_button.set_disabled(is_disarmed)
		if _navy_listbox:
			_update_unit_group_list(_navy_listbox, navies, in_progress_ships, false)

		show()
	else:
		hide()
