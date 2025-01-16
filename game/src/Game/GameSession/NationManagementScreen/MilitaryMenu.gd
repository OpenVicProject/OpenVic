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
var _digin_cap_label : GUILabel
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

# Armies and Navies
var _army_count_label : GUILabel
var _in_progress_brigade_count_label : GUILabel
var _disarmed_army_icon : GUIIcon
var _build_army_button : GUIIconButton
var _army_listbox : GUIListBox

var _navy_count_label : GUILabel
var _in_progress_ship_count_label : GUILabel
var _disarmed_navy_icon : GUIIcon
var _build_navy_button : GUIIconButton
var _navy_listbox : GUIListBox

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
		_digin_cap_label = GUINode.get_gui_label_from_node(stats_panel.get_node(^"./digin_cap"))
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
					_update_info()
					print("SORT LEADERS BY PRESTIGE")
			)
			sort_leader_prestige_button.set_tooltip_string("SORT_BY_PRESTIGE")
		var sort_leader_type_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./sort_leader_type"))
		if sort_leader_type_button:
			sort_leader_type_button.pressed.connect(
				func() -> void:
					_leader_sort_key = MenuSingleton.LeaderSortKey.LEADER_SORT_TYPE
					_update_info()
					print("SORT LEADERS BY TYPE")
			)
			sort_leader_type_button.set_tooltip_string("MILITARY_SORT_BY_TYPE_TOOLTIP")
		var sort_leader_name_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./sort_leader_name"))
		if sort_leader_name_button:
			sort_leader_name_button.pressed.connect(
				func() -> void:
					_leader_sort_key = MenuSingleton.LeaderSortKey.LEADER_SORT_NAME
					_update_info()
					print("SORT LEADERS BY NAME")
			)
			sort_leader_name_button.set_tooltip_string("MILITARY_SORT_BY_NAME_TOOLTIP")
		var sort_leader_army_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./sort_leader_army"))
		if sort_leader_army_button:
			sort_leader_army_button.set_text("MILITARY_SORT_ARMY")
			sort_leader_army_button.pressed.connect(
				func() -> void:
					_leader_sort_key = MenuSingleton.LeaderSortKey.LEADER_SORT_ASSIGNMENT
					_update_info()
					print("SORT LEADERS BY ARMY")
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
		_auto_assign_leader_button = GUINode.get_gui_icon_button_from_node(leaders_panel.get_node(^"./auto_assign"))
		if _auto_assign_leader_button:
			_auto_assign_leader_button.toggled.connect(func(state : bool) -> void: print("AUTO ASSIGN LEADERS = ", state))
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
			sort_armies_name_button.pressed.connect(func() -> void: print("SORT ARMIES BY NAME"))
			sort_armies_name_button.set_tooltip_string("MILITARY_SORT_BY_NAME_TOOLTIP")
		var sort_armies_strength_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(army_window.get_node(^"./sort_strength"))
		if sort_armies_strength_button:
			sort_armies_strength_button.pressed.connect(func() -> void: print("SORT ARMIES BY STRENGTH"))
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
			sort_navies_name_button.pressed.connect(func() -> void: print("SORT NAVIES BY NAME"))
			sort_navies_name_button.set_tooltip_string("MILITARY_SORT_BY_NAME_TOOLTIP")
		var sort_navies_strength_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(navy_window.get_node(^"./sort_strength"))
		if sort_navies_strength_button:
			sort_navies_strength_button.pressed.connect(func() -> void: print("SORT NAVIES BY STRENGTH"))
			sort_navies_strength_button.set_tooltip_string("MILITARY_SORT_BY_STRENGTH_TOOLTIP")

		_build_navy_button = GUINode.get_gui_icon_button_from_node(navy_window.get_node(^"./build_new"))
		if _build_navy_button:
			_build_navy_button.pressed.connect(func() -> void: "BUILD NAVY")
			_build_navy_button.set_text("MILITARY_BUILD_NAVY")
			_build_navy_button.set_tooltip_string("MILITARY_BUILD_NAVY_TOOLTIP")

		_navy_listbox = GUINode.get_gui_listbox_from_node(navy_window.get_node(^"./unit_listbox"))

	# mobilize and demobilize GUIIconButtons (are both actually used?)
	# mobilize_progress GUIProgressBar, mobilize_progress_text GUILabel
	# mob_size and mob_impact GUILabels

	# leader Panel
	# - generals and admirals GUILabels
	# - sort_leader_prestige, sort_leader_type, sort_leader_name, sort_leader_army GUIIconButtons (sort_prestige_icon GUIIcon too)
	# - new_general and new_admiral GUIIconButtons
	# - auto_create and auto_assign GUIIconButton
	# - leader_listbox GUIListBox

	# milview_leader_entry Panel
	# - leader_prestige_bar GUIProgressBar
	# - leader GUIIcon
	# - name, background and personality GUILabels
	# - use_leader GUIIconButton
	# - army, location GUILabels

	# stats Panel
	# - war_exhaustion, supply_consumption, org_regain, army_org, navy_org, unit_experience, recruit_time, combat_width, digin_cap, tactics_level GUILabels

	# army & navy unit_window Panels
	# - current_count GUILabel
	# - under_construction GUILabel
	# - cut_down_to_size GUIIcon
	# - sort_name and sort_strength GUIIconButtons
	# - build_new GUIButton
	# - unit_listbox GUIListBox

	# unit_entry Panel
	# - unit_progress GUIProgressBar (used when building units)
	# - leader and unit_strip GUIIcons (unit_strip used when building units)
	# - name, location, unit_eta, regiments, men GUILabels
	# - military_cancel_unit GUIIconButton
	# - morale_progress, strength_progress GUIProgressBars
	# - moving, digin, combat GUIIcons

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
		# Military stats
		const military_info_war_exhaustion_key              : StringName = &"war_exhaustion"
		const military_info_war_exhaustion_max_key          : StringName = &"war_exhaustion_max"
		const military_info_supply_consumption_key          : StringName = &"supply_consumption"
		const military_info_organisation_regain_key         : StringName = &"organisation_regain"
		const military_info_land_organisation_key           : StringName = &"land_organisation"
		const military_info_naval_organisation_key          : StringName = &"naval_organisation"
		const military_info_land_unit_start_experience_key  : StringName = &"land_unit_start_experience"
		const military_info_naval_unit_start_experience_key : StringName = &"naval_unit_start_experience"
		const military_info_recruit_time_key                : StringName = &"recruit_time"
		const military_info_combat_width_key                : StringName = &"combat_width"
		const military_info_digin_cap_key                   : StringName = &"digin_cap"
		const military_info_military_tactics_key            : StringName = &"military_tactics"

		# Mobilisation
		const military_info_is_mobilised_key                : StringName = &"is_mobilised"
		const military_info_mobilisation_progress_key       : StringName = &"mobilisation_progress"
		const military_info_mobilisation_size_key           : StringName = &"mobilisation_size"
		const military_info_mobilisation_size_tooltip_key   : StringName = &"mobilisation_size_tooltip"
		const military_info_mobilisation_impact_tooltip_key : StringName = &"mobilisation_impact_tooltip"
		const military_info_mobilisation_economy_impact_key : StringName = &"mobilisation_economy_impact"
		const military_info_mobilisation_economy_impact_tooltip_key : StringName = &"mobilisation_economy_impact_tooltip"

		# Leaders
		const military_info_general_count_key               : StringName = &"general_count"
		const military_info_admiral_count_key               : StringName = &"admiral_count"
		const military_info_create_leader_count_key         : StringName = &"create_leader_count"
		const military_info_auto_create_leaders_key         : StringName = &"auto_create_leaders"
		const military_info_auto_assign_leaders_key         : StringName = &"auto_assign_leaders"
		const military_info_leaders_list_key                : StringName = &"leaders_list"

		# Armies and Navies
		const military_info_is_disarmed_key                 : StringName = &"is_disarmed"
		const military_info_armies_key                      : StringName = &"armies"
		const military_info_in_progress_brigades_key        : StringName = &"in_progress_brigades"
		const military_info_navies_key                      : StringName = &"navies"
		const military_info_in_progress_ships_key           : StringName = &"in_progress_ships"

		var military_info : Dictionary = MenuSingleton.get_military_menu_info(_leader_sort_key)

		# Military stats
		if _war_exhaustion_label:
			_war_exhaustion_label.set_text(
				"%s/%s" % [
					GUINode.float_to_string_dp(military_info.get(military_info_war_exhaustion_key, 0), 2),
					GUINode.float_to_string_dp(military_info.get(military_info_war_exhaustion_max_key, 0), 2)
				]
			)
		if _supply_consumption_label:
			_supply_consumption_label.set_text("%d%%" % int(100 * military_info.get(military_info_supply_consumption_key, 0)))
		if _organisation_regain_label:
			_organisation_regain_label.set_text("%d%%" % int(100 * military_info.get(military_info_organisation_regain_key, 0)))
		if _land_organisation_label:
			_land_organisation_label.set_text("%d%%" % int(100 * military_info.get(military_info_land_organisation_key, 0)))
		if _naval_organisation_label:
			_naval_organisation_label.set_text("%d%%" % int(100 * military_info.get(military_info_naval_organisation_key, 0)))
		if _unit_start_experience_label:
			_unit_start_experience_label.set_text(
				"%s/%s" % [
					GUINode.float_to_string_dp(military_info.get(military_info_land_unit_start_experience_key, 0), 2),
					GUINode.float_to_string_dp(military_info.get(military_info_naval_unit_start_experience_key, 0), 2)
				]
			)
		if _recruit_time_label:
			_recruit_time_label.set_text("%d%%" % int(100 * military_info.get(military_info_recruit_time_key, 0)))
		if _combat_width_label:
			_combat_width_label.set_text(str(military_info.get(military_info_combat_width_key, 0)))
		if _digin_cap_label:
			_digin_cap_label.set_text(str(military_info.get(military_info_digin_cap_key, 0)))
		if _military_tactics_label:
			_military_tactics_label.set_text("%s%%" % GUINode.float_to_string_dp(100 * military_info.get(military_info_military_tactics_key, 0), 2))

		# Mobilisation
		# TODO - mobilisation button could be disabled?
		var is_mobilised : bool = military_info.get(military_info_is_mobilised_key, false)
		var mobilisation_size_tooltip : String = military_info.get(military_info_mobilisation_size_tooltip_key, "")
		var mobilisation_impact_tooltip : String = military_info.get(military_info_mobilisation_impact_tooltip_key, "")
		if _mobilise_button:
			_mobilise_button.set_visible(not is_mobilised)
			_mobilise_button.set_tooltip_string(


			# TODO - make sure elements only appear when they're meant to!!!
			# in particular check out the case where you can't mobilise due to insufficient brigades


				tr(&"MILITARY_MOBILIZE") + "\n" + mobilisation_size_tooltip + "\n\n" + (tr(&"NOT_ENOUGH_FOR_BRIGADE") if true else "") +
				"\n\n" + mobilisation_impact_tooltip + MenuSingleton.get_tooltip_separator() + tr(&"MILITARY_MOBILIZE_DESC")
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
			_mobilisation_size_label.set_text(str(military_info.get(military_info_mobilisation_size_key, 0)))
			_mobilisation_size_label.set_tooltip_string(
				mobilisation_size_tooltip + ("" if mobilisation_size_tooltip.is_empty() or mobilisation_impact_tooltip.is_empty() else "\n\n\n") + mobilisation_impact_tooltip
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
		# TODO - leave the main text set and only update $VALUE$ using substitution dict functionality once buttons get proper text support
		if _create_general_button:
			_create_general_button.set_text(tr(&"MILITARY_CREATE_GENERAL").replace("$VALUE$", str(create_leader_count)))
			_create_general_button.set_disabled(create_leader_count < 1)
		if _create_admiral_button:
			_create_admiral_button.set_text(tr(&"MILITARY_CREATE_ADMIRAL").replace("$VALUE$", str(create_leader_count)))
			_create_admiral_button.set_disabled(create_leader_count < 1)
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
					#use_leader_button.toggled.connect(func(state : bool) -> void: print("Toggled use_leader to ", state))
					use_leader_button.set_tooltip_string("USE_LEADER" if use_leader_button.is_pressed() else "")

				var army_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./army"))
				var location_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./location"))

				if military_info_leader_assignment_key in leader_dict:
					var leader_assignment : String = leader_dict.get(military_info_leader_assignment_key, "")
					var leader_location : String = GUINode.format_province_name(leader_dict.get(military_info_leader_location_key, ""), true)

					# TODO - probably simpler to use directly translated and replaced string rather than set_tooltip_string_and_substitution_dict
					const assignment_tooltip_string : String = "MILITARY_LEADER_NAME_TOOLTIP"
					var assignment_tooltip_dict : Dictionary = {
						"NAME": leader_name,
						"ARMY": leader_assignment,
						"LOCATION": leader_location
					}

					if army_label:
						army_label.set_text(leader_assignment)
						army_label.set_tooltip_string_and_substitution_dict(assignment_tooltip_string, assignment_tooltip_dict)
					if location_label:
						location_label.set_text(leader_location)
						location_label.set_tooltip_string_and_substitution_dict(assignment_tooltip_string, assignment_tooltip_dict)
				else:
					if army_label:
						army_label.set_text("MILITARY_UNASSIGNED")
						army_label.clear_tooltip()
					if location_label:
						location_label.set_text("")
						location_label.clear_tooltip()

		# Armies and Navies
		var is_disarmed : bool = military_info.get(military_info_is_disarmed_key, false)

		# unit_entry Panel
		# - unit_progress GUIProgressBar (used when building units)
		# - leader and unit_strip GUIIcons (unit_strip used when building units)
		# - name, location, unit_eta, regiments, men GUILabels
		# - military_cancel_unit GUIIconButton
		# - morale_progress, strength_progress GUIProgressBars
		# - moving, digin, combat GUIIcons

		var armies : Array[Dictionary] = military_info.get(military_info_armies_key, [] as Array[Dictionary])
		var in_progress_brigades : Array[Dictionary] = military_info.get(military_info_in_progress_brigades_key, [] as Array[Dictionary])
		in_progress_brigades.push_back(
			{
				&"brigade_progress": 0.25,
				&"brigade_icon": 1,
				&"brigade_name": "Test Brigade",
				&"brigade_location": "300",
				&"brigade_eta": "2025.1.20",
				&"brigade_tooltip": tr(&"GOODS_PROJECT_LACK_GOODS")
			}
		)
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
			var total_entry_count : int = armies.size() + in_progress_brigades.size()
			_army_listbox.clear_children(total_entry_count)
			while _army_listbox.get_child_count() < total_entry_count:
				var army_entry : Panel = GUINode.generate_gui_element(_gui_file, "unit_entry")
				if not army_entry:
					break
				_army_listbox.add_child(army_entry)

			const military_info_army_leader_picture_key : StringName = &"army_leader_picture"
			const military_info_army_leader_tooltip_key : StringName = &"army_leader_tooltip"
			const military_info_army_name_key : StringName = &"army_name"
			const military_info_army_location_key : StringName = &"army_location"
			const military_info_army_regiment_count_key : StringName = &"army_regiment_count"
			const military_info_army_men_count_key : StringName = &"army_men_count"
			const military_info_army_max_men_count_key : StringName = &"army_max_men_count"
			const military_info_army_morale_key : StringName = &"army_morale"
			const military_info_army_moving_tooltip_key : StringName = &"army_moving_tooltip"
			const military_info_army_digin_tooltip_key : StringName = &"army_digin_tooltip"
			const military_info_army_combat_key : StringName = &"army_combat"

			for index : int in mini(armies.size(), _army_listbox.get_child_count()):
				var entry_menu : Panel = GUINode.get_panel_from_node(_army_listbox.get_child(index))
				var army_dict : Dictionary = armies[index]

				var entry_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_unit_entry_bg"))
				if entry_button:
					# TODO - sort out repeat connections!!!
					entry_button.pressed.connect(func() -> void: print("OPENING ARMY"))
				var unit_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./unit_progress"))
				if unit_progress_bar:
					unit_progress_bar.hide()
				var leader_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./leader"))
				if leader_icon:
					var leader_texture : Texture2D = army_dict.get(military_info_army_leader_picture_key, null)
					if leader_texture:
						leader_icon.show()
						leader_icon.set_texture(leader_texture)
						leader_icon.set_tooltip_string(army_dict.get(military_info_army_leader_tooltip_key, ""))
					else:
						leader_icon.hide()
				var unit_strip_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./unit_strip"))
				if unit_strip_icon:
					unit_strip_icon.hide()
				var name_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./name"))
				if name_label:
					name_label.set_text(army_dict.get(military_info_army_name_key, ""))
				var location_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./location"))
				if location_label:
					location_label.set_text(GUINode.format_province_name(army_dict.get(military_info_army_location_key, "")))
				var unit_eta_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./unit_eta"))
				if unit_eta_label:
					unit_eta_label.hide()
				var regiments_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./regiments"))
				if regiments_label:
					regiments_label.show()
					var regiment_count_string : String = str(army_dict.get(military_info_army_regiment_count_key, 0))
					regiments_label.set_text(regiment_count_string)
					regiments_label.set_tooltip_string(tr(&"MILITARY_REGIMENTS_TOOLTIP").replace("$VALUE$", regiment_count_string))
				var men_count : int = army_dict.get(military_info_army_men_count_key, 0)
				var max_men_count : int = army_dict.get(military_info_army_max_men_count_key, 0)
				var strength : float = men_count / maxi(max_men_count, 1)
				var men_count_string : String = GUINode.int_to_string_commas(men_count)
				var strength_tooltip : String = tr(&"MILITARY_STRENGTH_TOOLTIP2").replace("$PERCENT$", str(int(strength * 100))).replace("$VALUE$", men_count_string).replace("$MAX$", GUINode.int_to_string_commas(max_men_count))
				# MILITARY_STRENGTH_TOOLTIP2;Unit strength is at �Y$PERCENT$%�W.\nStrength: $VALUE$/�Y$MAX$�W
				# MILITARY_SHIPSTRENGTH_TOOLTIP2;Unit strength is at �Y$PERCENT$%�W.
				var men_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./men"))
				if men_label:
					men_label.show()
					men_label.set_text(men_count_string)
					men_label.set_tooltip_string(strength_tooltip)
				var military_cancel_unit_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_cancel_unit"))
				if military_cancel_unit_button:
					military_cancel_unit_button.hide()
				var morale_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./morale_progress"))
				if morale_progress_bar:
					morale_progress_bar.show()
					var morale : float = army_dict.get(military_info_army_morale_key, 0.0)
					morale_progress_bar.set_value_no_signal(morale)
					morale_progress_bar.set_tooltip_string(tr(&"MILITARY_MORALE_TOOLTIP").replace("$VALUE$", str(int(morale * 100))))
				var strength_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./strength_progress"))
				if strength_progress_bar:
					strength_progress_bar.show()
					strength_progress_bar.set_value_no_signal(strength)
					strength_progress_bar.set_tooltip_string(strength_tooltip)
				var moving_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./moving"))
				if moving_icon:
					var moving_tooltip : String = army_dict.get(military_info_army_moving_tooltip_key, "")
					moving_icon.set_visible(not moving_tooltip.is_empty())
					moving_icon.set_tooltip_string(moving_tooltip)
				var digin_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./digin"))
				if digin_icon:
					var digin_tooltip : String = army_dict.get(military_info_army_digin_tooltip_key, "")
					digin_icon.set_visible(not digin_tooltip.is_empty())
					digin_icon.set_tooltip_string(digin_tooltip)
				var combat_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./combat"))
				if combat_icon:
					combat_icon.set_visible(army_dict.get(military_info_army_combat_key, false))

			# unit_entry Panel
			# - military_unit_entry_bg GUIIconButton
			# - unit_progress GUIProgressBar (used when building units)
			# - leader and unit_strip GUIIcons (unit_strip used when building units)
			# - name, location, unit_eta, regiments, men GUILabels
			# - military_cancel_unit GUIIconButton
			# - morale_progress, strength_progress GUIProgressBars
			# - moving, digin, combat GUIIcons

			const military_info_brigade_progress_key : StringName = &"brigade_progress"
			const military_info_brigade_icon_key : StringName = &"brigade_icon"
			const military_info_brigade_name_key : StringName = &"brigade_name"
			const military_info_brigade_location_key : StringName = &"brigade_location"
			const military_info_brigade_eta_key : StringName = &"brigade_eta"
			# "Gathering the following goods before construction can begin: ..." - applied to most sub-nodes
			const military_info_brigade_tooltip_key : StringName = &"brigade_tooltip"

			for index : int in clampi(_army_listbox.get_child_count() - armies.size(), 0, in_progress_brigades.size()):
				var entry_menu : Panel = GUINode.get_panel_from_node(_army_listbox.get_child(index + armies.size()))
				var brigade_dict : Dictionary = in_progress_brigades[index]
				var brigade_tooltip : String = brigade_dict.get(military_info_brigade_tooltip_key, "")

				var unit_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./unit_progress"))
				if unit_progress_bar:
					unit_progress_bar.show()
					unit_progress_bar.set_value_no_signal(brigade_dict.get(military_info_brigade_progress_key, 0))
					# This is enough to show the tooltip everywhere we need, the only place
					# in the base game that obviously also has it is the ETA date label
					unit_progress_bar.set_tooltip_string(brigade_tooltip)
				var leader_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./leader"))
				if leader_icon:
					leader_icon.hide()
				var unit_strip_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./unit_strip"))
				if unit_strip_icon:
					unit_strip_icon.show()
					unit_strip_icon.set_icon_index(brigade_dict.get(military_info_brigade_icon_key, 0))
				var name_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./name"))
				if name_label:
					name_label.set_text(brigade_dict.get(military_info_brigade_name_key, ""))
				var location_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./location"))
				if location_label:
					location_label.set_text(GUINode.format_province_name(brigade_dict.get(military_info_brigade_location_key, "")))
				var unit_eta_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./unit_eta"))
				if unit_eta_label:
					unit_eta_label.show()
					unit_eta_label.set_text(brigade_dict.get(military_info_brigade_eta_key, ""))
				var regiments_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./regiments"))
				if regiments_label:
					regiments_label.hide()
				var men_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./men"))
				if men_label:
					men_label.hide()
				var military_cancel_unit_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_cancel_unit"))
				if military_cancel_unit_button:
					military_cancel_unit_button.show()
					# TODO - sort out repeat connections!!!
					military_cancel_unit_button.pressed.connect(func() -> void: print("CANCELLED BRIGADE CONSTRUCTION"))
				var morale_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./morale_progress"))
				if morale_progress_bar:
					morale_progress_bar.hide()
				var strength_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./strength_progress"))
				if strength_progress_bar:
					strength_progress_bar.hide()
				var moving_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./moving"))
				if moving_icon:
					moving_icon.hide()
				var digin_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./digin"))
				if digin_icon:
					digin_icon.hide()
				var combat_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./combat"))
				if combat_icon:
					combat_icon.hide()








#	TODO
#		- use same code/dictionary keys to generate in-progress constructions for LAND and NAVAL
#		- maybe combine LAND and NAVAL unit group generation a bit (ofc with special cases for specifics like digin and different localisation)
#		- implement sorting!
#		- i could store nodes in lists to avoid `get_node`-ing them every update?
#		- how to handle connections - maybe have a fixed Callable including the entry's index, which can be converted back into a leader/army/navy/in-progress construction at the C++ level?!?!











		var navies : Array[Dictionary] = military_info.get(military_info_navies_key, [] as Array[Dictionary])
		var in_progress_ships : Array[Dictionary] = military_info.get(military_info_in_progress_ships_key, [] as Array[Dictionary])
		in_progress_ships.push_back(
			{
				&"brigade_progress": 0.75,
				&"brigade_icon": 9,
				&"brigade_name": "Test Ship",
				&"brigade_location": "301",
				&"brigade_eta": "2025.11.3",
				&"brigade_tooltip": tr(&"GOODS_PROJECT_LACK_GOODS")
			}
		)
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
			var total_entry_count : int = navies.size() + in_progress_ships.size()
			_navy_listbox.clear_children(total_entry_count)
			while _navy_listbox.get_child_count() < total_entry_count:
				var navy_entry : Panel = GUINode.generate_gui_element(_gui_file, "unit_entry")
				if not navy_entry:
					break
				var men_label : GUILabel = GUINode.get_gui_label_from_node(navy_entry.get_node(^"./men"))
				if men_label:
					men_label.hide()
				var digin_icon : GUIIcon = GUINode.get_gui_icon_from_node(navy_entry.get_node(^"./digin"))
				if digin_icon:
					digin_icon.hide()
				_navy_listbox.add_child(navy_entry)

			const military_info_navy_leader_picture_key : StringName = &"navy_leader_picture"
			const military_info_navy_leader_tooltip_key : StringName = &"navy_leader_tooltip"
			const military_info_navy_name_key : StringName = &"navy_name"
			const military_info_navy_location_key : StringName = &"navy_location"
			const military_info_navy_ship_count_key : StringName = &"navy_ship_count"
			const military_info_navy_morale_key : StringName = &"navy_morale"
			const military_info_navy_strength_key : StringName = &"navy_strength"
			const military_info_navy_moving_tooltip_key : StringName = &"navy_moving_tooltip"
			const military_info_navy_combat_key : StringName = &"navy_combat"

			for index : int in mini(navies.size(), _navy_listbox.get_child_count()):
				var entry_menu : Panel = GUINode.get_panel_from_node(_navy_listbox.get_child(index))
				var navy_dict : Dictionary = navies[index]

				var entry_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_unit_entry_bg"))
				if entry_button:
					# TODO - sort out repeat connections!!!
					entry_button.pressed.connect(func() -> void: print("OPENING NAVY"))
				var unit_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./unit_progress"))
				if unit_progress_bar:
					unit_progress_bar.hide()
				var leader_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./leader"))
				if leader_icon:
					var leader_texture : Texture2D = navy_dict.get(military_info_navy_leader_picture_key, null)
					if leader_texture:
						leader_icon.show()
						leader_icon.set_texture(leader_texture)
						leader_icon.set_tooltip_string(navy_dict.get(military_info_navy_leader_tooltip_key, ""))
					else:
						leader_icon.hide()
				var unit_strip_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./unit_strip"))
				if unit_strip_icon:
					unit_strip_icon.hide()
				var name_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./name"))
				if name_label:
					name_label.set_text(navy_dict.get(military_info_navy_name_key, ""))
				var location_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./location"))
				if location_label:
					location_label.set_text(GUINode.format_province_name(navy_dict.get(military_info_navy_location_key, "")))
				var unit_eta_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./unit_eta"))
				if unit_eta_label:
					unit_eta_label.hide()
				var ships_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./regiments"))
				if ships_label:
					ships_label.show()
					var ship_count_string : String = str(navy_dict.get(military_info_navy_ship_count_key, 0))
					ships_label.set_text(ship_count_string)
					ships_label.set_tooltip_string(tr(&"MILITARY_SHIPS_TOOLTIP").replace("$VALUE$", ship_count_string))
				var military_cancel_unit_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_cancel_unit"))
				if military_cancel_unit_button:
					military_cancel_unit_button.hide()
				var morale_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./morale_progress"))
				if morale_progress_bar:
					morale_progress_bar.show()
					var morale : float = navy_dict.get(military_info_navy_morale_key, 0.0)
					morale_progress_bar.set_value_no_signal(morale)
					morale_progress_bar.set_tooltip_string(tr(&"MILITARY_MORALE_TOOLTIP").replace("$VALUE$", str(int(morale * 100))))
				var strength_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./strength_progress"))
				if strength_progress_bar:
					strength_progress_bar.show()
					var strength : float = navy_dict.get(military_info_navy_strength_key, 0)
					strength_progress_bar.set_value_no_signal(strength)
					strength_progress_bar.set_tooltip_string(tr(&"MILITARY_SHIPSTRENGTH_TOOLTIP2").replace("$PERCENT$", str(int(strength * 100))))
				var moving_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./moving"))
				if moving_icon:
					var moving_tooltip : String = navy_dict.get(military_info_navy_moving_tooltip_key, "")
					moving_icon.set_visible(not moving_tooltip.is_empty())
					moving_icon.set_tooltip_string(moving_tooltip)
				var combat_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./combat"))
				if combat_icon:
					combat_icon.set_visible(navy_dict.get(military_info_navy_combat_key, false))

			# unit_entry Panel
			# - military_unit_entry_bg GUIIconButton
			# - unit_progress GUIProgressBar (used when building units)
			# - leader and unit_strip GUIIcons (unit_strip used when building units)
			# - name, location, unit_eta, regiments, men GUILabels
			# - military_cancel_unit GUIIconButton
			# - morale_progress, strength_progress GUIProgressBars
			# - moving, digin, combat GUIIcons

			const military_info_brigade_progress_key : StringName = &"brigade_progress"
			const military_info_brigade_icon_key : StringName = &"brigade_icon"
			const military_info_brigade_name_key : StringName = &"brigade_name"
			const military_info_brigade_location_key : StringName = &"brigade_location"
			const military_info_brigade_eta_key : StringName = &"brigade_eta"
			# "Gathering the following goods before construction can begin: ..." - applied to most sub-nodes
			const military_info_brigade_tooltip_key : StringName = &"brigade_tooltip"

			for index : int in clampi(_navy_listbox.get_child_count() - navies.size(), 0, in_progress_ships.size()):
				var entry_menu : Panel = GUINode.get_panel_from_node(_navy_listbox.get_child(index + navies.size()))
				var brigade_dict : Dictionary = in_progress_ships[index]
				var brigade_tooltip : String = brigade_dict.get(military_info_brigade_tooltip_key, "")

				var unit_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./unit_progress"))
				if unit_progress_bar:
					unit_progress_bar.show()
					unit_progress_bar.set_value_no_signal(brigade_dict.get(military_info_brigade_progress_key, 0))
					# This is enough to show the tooltip everywhere we need, the only place
					# in the base game that obviously also has it is the ETA date label
					unit_progress_bar.set_tooltip_string(brigade_tooltip)
				var leader_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./leader"))
				if leader_icon:
					leader_icon.hide()
				var unit_strip_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./unit_strip"))
				if unit_strip_icon:
					unit_strip_icon.show()
					unit_strip_icon.set_icon_index(brigade_dict.get(military_info_brigade_icon_key, 0))
				var name_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./name"))
				if name_label:
					name_label.set_text(brigade_dict.get(military_info_brigade_name_key, ""))
				var location_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./location"))
				if location_label:
					location_label.set_text(GUINode.format_province_name(brigade_dict.get(military_info_brigade_location_key, "")))
				var unit_eta_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./unit_eta"))
				if unit_eta_label:
					unit_eta_label.show()
					unit_eta_label.set_text(brigade_dict.get(military_info_brigade_eta_key, ""))
				var ships_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./regiments"))
				if ships_label:
					ships_label.hide()
				var men_label : GUILabel = GUINode.get_gui_label_from_node(entry_menu.get_node(^"./men"))
				if men_label:
					men_label.hide()
				var military_cancel_unit_button : GUIIconButton = GUINode.get_gui_icon_button_from_node(entry_menu.get_node(^"./military_cancel_unit"))
				if military_cancel_unit_button:
					military_cancel_unit_button.show()
					# TODO - sort out repeat connections!!!
					military_cancel_unit_button.pressed.connect(func() -> void: print("CANCELLED BRIGADE CONSTRUCTION"))
				var morale_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./morale_progress"))
				if morale_progress_bar:
					morale_progress_bar.hide()
				var strength_progress_bar : GUIProgressBar = GUINode.get_gui_progress_bar_from_node(entry_menu.get_node(^"./strength_progress"))
				if strength_progress_bar:
					strength_progress_bar.hide()
				var moving_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./moving"))
				if moving_icon:
					moving_icon.hide()
				var combat_icon : GUIIcon = GUINode.get_gui_icon_from_node(entry_menu.get_node(^"./combat"))
				if combat_icon:
					combat_icon.hide()

		show()
	else:
		hide()
