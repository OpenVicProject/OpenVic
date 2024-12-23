extends GUINode

var _active: bool = false

const _screen: NationManagement.Screen = NationManagement.Screen.POPULATION

const _scene_name: String = "country_pops"

var _pop_screen_panel: Panel

var _province_listbox: GUIListBox
var _province_list_scroll_index: int = 0
var _province_list_types: Array[MenuSingleton.ProvinceListEntry]
var _province_list_indices: PackedInt32Array
var _province_list_panels: Array[Panel]
var _province_list_buttons: Array[GUIIconButton]
var _province_list_name_labels: Array[GUILabel]
var _province_list_size_labels: Array[GUILabel]
var _province_list_growth_icons: Array[GUIIcon]
var _province_list_colony_buttons: Array[GUIIconButton]
var _province_list_national_focus_buttons: Array[GUIIconButton]
var _province_list_expand_buttons: Array[GUIIconButton]

var _pop_filter_buttons: Array[GUIIconButton]
var _pop_filter_icons: Array[GFXSpriteTexture]
var _pop_filter_selected_icons: Array[GFXButtonStateTexture]
var _pop_filter_hover_icons: Array[GFXButtonStateTexture]

var _distribution_charts: Array[GUIPieChart]
var _distribution_lists: Array[GUIListBox]

var _pop_list_scrollbar: GUIScrollbar
var _pop_list_scroll_index: int = 0

var _pop_list_rows: Array[Panel]
var _pop_list_size_labels: Array[GUILabel]
var _pop_list_type_buttons: Array[GUIIconButton]
var _pop_list_producing_icons: Array[GUIIcon]
var _pop_list_culture_labels: Array[GUILabel]
var _pop_list_religion_icons: Array[GUIIcon]
var _pop_list_location_labels: Array[GUILabel]
var _pop_list_militancy_labels: Array[GUILabel]
var _pop_list_consciousness_labels: Array[GUILabel]
var _pop_list_ideology_charts: Array[GUIPieChart]
var _pop_list_issues_charts: Array[GUIPieChart]
var _pop_list_unemployment_progressbars: Array[GUIProgressBar]
var _pop_list_cash_labels: Array[GUILabel]
var _pop_list_life_needs_progressbars: Array[GUIProgressBar]
var _pop_list_everyday_needs_progressbars: Array[GUIProgressBar]
var _pop_list_luxury_needs_progressbars: Array[GUIProgressBar]
var _pop_list_rebel_icons: Array[GUIIcon]
var _pop_list_social_movement_icons: Array[GUIIcon]
var _pop_list_political_movement_icons: Array[GUIIcon]
var _pop_list_national_movement_flags: Array[GUIMaskedFlag]
var _pop_list_size_change_icons: Array[GUIIcon]
var _pop_list_literacy_labels: Array[GUILabel]

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)
	MenuSingleton.population_menu_province_list_changed.connect(_setup_province_list)
	MenuSingleton.population_menu_province_list_selected_changed.connect(_update_province_list)
	MenuSingleton.population_menu_pops_changed.connect(_update_pops)

	MenuSingleton.population_menu_update_locale_sort_cache()

	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	add_gui_element(_scene_name, "country_pop")

	set_click_mask_from_nodepaths([^"./country_pop/main_bg"])

	var close_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_pop/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	_pop_screen_panel = get_panel_from_nodepath(^"./country_pop")

	_setup_province_list()
	_setup_sort_buttons()
	_setup_pop_filter_buttons()
	_setup_distribution_windows()
	_setup_pop_list()

	_update_info()

func _generate_province_list_row(index: int, type: MenuSingleton.ProvinceListEntry) -> Error:
	while _province_list_types.size() <= index:
		_province_list_types.push_back(MenuSingleton.LIST_ENTRY_NONE)
		_province_list_indices.push_back(-1)
		_province_list_panels.push_back(null)
		_province_list_buttons.push_back(null)
		_province_list_name_labels.push_back(null)
		_province_list_size_labels.push_back(null)
		_province_list_growth_icons.push_back(null)
		_province_list_colony_buttons.push_back(null)
		_province_list_national_focus_buttons.push_back(null)
		_province_list_expand_buttons.push_back(null)

	if _province_list_types[index] == type:
		return OK

	if _province_list_panels[index]:
		_province_listbox.remove_child(_province_list_panels[index])
		_province_list_panels[index].queue_free()

	_province_list_types[index] = MenuSingleton.LIST_ENTRY_NONE
	_province_list_indices[index] = -1
	_province_list_panels[index] = null
	_province_list_buttons[index] = null
	_province_list_name_labels[index] = null
	_province_list_size_labels[index] = null
	_province_list_growth_icons[index] = null
	_province_list_colony_buttons[index] = null
	_province_list_national_focus_buttons[index] = null
	_province_list_expand_buttons[index] = null

	if type == MenuSingleton.LIST_ENTRY_NONE:
		return OK

	const gui_element_names: Dictionary = {
		MenuSingleton.LIST_ENTRY_COUNTRY: "poplistitem_country",
		MenuSingleton.LIST_ENTRY_STATE: "poplistitem_state",
		MenuSingleton.LIST_ENTRY_PROVINCE: "poplistitem_province"
	}

	var entry_panel: Panel = GUINode.generate_gui_element(_scene_name, gui_element_names[type])

	if not entry_panel:
		return FAILED

	_province_list_types[index] = type

	_province_list_panels[index] = entry_panel

	_province_list_buttons[index] = GUINode.get_gui_icon_button_from_node(entry_panel.get_node(^"./poplistbutton"))
	if _province_list_buttons[index]:
		_province_list_buttons[index].pressed.connect(
			func() -> void: MenuSingleton.population_menu_select_province_list_entry(_province_list_indices[index])
		)

	_province_list_name_labels[index] = GUINode.get_gui_label_from_node(entry_panel.get_node(^"./poplist_name"))

	_province_list_size_labels[index] = GUINode.get_gui_label_from_node(entry_panel.get_node(^"./poplist_numpops"))

	_province_list_growth_icons[index] = GUINode.get_gui_icon_from_node(entry_panel.get_node(^"./growth_indicator"))

	if type == MenuSingleton.LIST_ENTRY_STATE:
		_province_list_colony_buttons[index] = GUINode.get_gui_icon_button_from_node(entry_panel.get_node(^"./colonial_state_icon"))

		# TODO - connect national focus button to national focus selection submenu
		_province_list_national_focus_buttons[index] = GUINode.get_gui_icon_button_from_node(entry_panel.get_node(^"./state_focus"))

		_province_list_expand_buttons[index] = GUINode.get_gui_icon_button_from_node(entry_panel.get_node(^"./expand"))
		if _province_list_expand_buttons[index]:
			_province_list_expand_buttons[index].pressed.connect(
				func() -> void: MenuSingleton.population_menu_toggle_expanded(_province_list_indices[index])
			)

	_province_listbox.add_child(entry_panel)
	_province_listbox.move_child(entry_panel, index)

	return OK

func _setup_province_list() -> void:
	if not _province_listbox:
		_province_listbox = get_gui_listbox_from_nodepath(^"./country_pop/pop_province_list")

		if not _province_listbox:
			return
		_province_listbox.scroll_index_changed.connect(_update_province_list)

	if _province_list_panels.size() < 1 or not _province_list_panels[0]:
		if _generate_province_list_row(0, MenuSingleton.LIST_ENTRY_COUNTRY) != OK or _province_list_panels.size() < 1 or not _province_list_panels[0]:
			push_error("Failed to generate country row in population menu province list to determine row height!")
			return

	_province_listbox.set_fixed(MenuSingleton.get_population_menu_province_list_row_count(), _province_list_panels[0].size.y, false)

func _setup_sort_buttons() -> void:
	# button_path : NodePath, clear_text : bool, sort_key : GameSingleton.PopSortKey
	const sort_button_info: Array[Array] = [
		[^"./country_pop/sortby_size_button", false, MenuSingleton.SORT_SIZE],
		[^"./country_pop/sortby_type_button", false, MenuSingleton.SORT_TYPE],
		[^"./country_pop/sortby_nationality_button", false, MenuSingleton.SORT_CULTURE],
		[^"./country_pop/sortby_religion_button", false, MenuSingleton.SORT_RELIGION],
		[^"./country_pop/sortby_location_button", false, MenuSingleton.SORT_LOCATION],
		[^"./country_pop/sortby_mil_button", true, MenuSingleton.SORT_MILITANCY],
		[^"./country_pop/sortby_con_button", true, MenuSingleton.SORT_CONSCIOUSNESS],
		[^"./country_pop/sortby_ideology_button", true, MenuSingleton.SORT_IDEOLOGY],
		[^"./country_pop/sortby_issues_button", true, MenuSingleton.SORT_ISSUES],
		[^"./country_pop/sortby_unemployment_button", true, MenuSingleton.SORT_UNEMPLOYMENT],
		[^"./country_pop/sortby_cash_button", true, MenuSingleton.SORT_CASH],
		[^"./country_pop/sortby_subsistence_button", true, MenuSingleton.SORT_LIFE_NEEDS],
		[^"./country_pop/sortby_eve_button", true, MenuSingleton.SORT_EVERYDAY_NEEDS],
		[^"./country_pop/sortby_luxury_button", true, MenuSingleton.SORT_LUXURY_NEEDS],
		[^"./country_pop/sortby_revoltrisk_button", true, MenuSingleton.SORT_REBEL_FACTION],
		[^"./country_pop/sortby_change_button", true, MenuSingleton.SORT_SIZE_CHANGE],
		[^"./country_pop/sortby_literacy_button", true, MenuSingleton.SORT_LITERACY]
	]

	for button_info: Array in sort_button_info:
		var sort_button: GUIIconButton = get_gui_icon_button_from_nodepath(button_info[0])
		if sort_button:
			if button_info[1]:
				sort_button.set_text("")
			sort_button.pressed.connect(MenuSingleton.population_menu_select_sort_key.bind(button_info[2]))

func _setup_pop_filter_buttons() -> void:
	if not _pop_screen_panel:
		push_error("Cannot set up pop filter buttons without pop screen to add them to")
		return

	var pop_filter_sprite_indices: PackedInt32Array = MenuSingleton.get_population_menu_pop_filter_setup_info()

	var pop_filter_start: Vector2 = GUINode.get_gui_position(_scene_name, "popfilter_start")
	var pop_filter_step: Vector2 = GUINode.get_gui_position(_scene_name, "popfilter_offset")

	for index: int in pop_filter_sprite_indices.size():
		var pop_filter_button: GUIIconButton = GUINode.get_gui_icon_button_from_node(GUINode.generate_gui_element(_scene_name, "pop_filter_button"))
		var pop_filter_icon: GFXSpriteTexture = null
		var pop_filter_selected_icon: GFXButtonStateTexture = null
		var pop_filter_hover_icon: GFXButtonStateTexture = null

		if pop_filter_button:
			_pop_screen_panel.add_child(pop_filter_button)
			pop_filter_button.set_position(pop_filter_start + pop_filter_step * index)
			pop_filter_button.pressed.connect(MenuSingleton.population_menu_toggle_pop_filter.bind(index))
			pop_filter_icon = pop_filter_button.get_gfx_sprite_texture()

			if pop_filter_icon:
				pop_filter_icon.set_icon_index(pop_filter_sprite_indices[index])
				pop_filter_selected_icon = pop_filter_icon.get_button_state_texture(GFXButtonStateTexture.SELECTED)
				pop_filter_hover_icon = pop_filter_icon.get_button_state_texture(GFXButtonStateTexture.HOVER)

		_pop_filter_buttons.push_back(pop_filter_button)
		_pop_filter_icons.push_back(pop_filter_icon)
		_pop_filter_selected_icons.push_back(pop_filter_selected_icon)
		_pop_filter_hover_icons.push_back(pop_filter_hover_icon)

	var select_all_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_pop/popfilter_ALL")
	if select_all_button:
		select_all_button.pressed.connect(MenuSingleton.population_menu_select_all_pop_filters)

	var deselect_all_button: GUIIconButton = get_gui_icon_button_from_nodepath(^"./country_pop/popfilter_DESELECT_ALL")
	if deselect_all_button:
		deselect_all_button.pressed.connect(MenuSingleton.population_menu_deselect_all_pop_filters)

func _setup_distribution_windows() -> void:
	if not _pop_screen_panel:
		push_error("Cannot set up distribution windows without pop screen to add them to")
		return

	const columns: int = 3

	var distribution_names: PackedStringArray = MenuSingleton.get_population_menu_distribution_setup_info()

	var distribution_start: Vector2 = GUINode.get_gui_position(_scene_name, "popdistribution_start")
	var distribution_step: Vector2 = GUINode.get_gui_position(_scene_name, "popdistribution_offset")

	for index: int in distribution_names.size():
		var distribution_panel: Panel = GUINode.generate_gui_element(_scene_name, "distribution_window")
		var distribution_chart: GUIPieChart = null
		var distribution_list: GUIListBox = null

		if distribution_panel:
			_pop_screen_panel.add_child(distribution_panel)
			distribution_panel.set_position(distribution_start + distribution_step * Vector2(index % columns, index / columns))

			var name_label: GUILabel = GUINode.get_gui_label_from_node(distribution_panel.get_node(^"./item_name"))
			if name_label:
				name_label.set_text(distribution_names[index])

			distribution_chart = GUINode.get_gui_pie_chart_from_node(distribution_panel.get_node(^"./chart"))
			distribution_list = GUINode.get_gui_listbox_from_node(distribution_panel.get_node(^"./member_names"))

		_distribution_charts.push_back(distribution_chart)
		_distribution_lists.push_back(distribution_list)

func _setup_pop_list() -> void:
	_pop_list_scrollbar = get_gui_scrollbar_from_nodepath(^"./country_pop/external_scroll_slider")

	var pop_list_panel: Panel = get_panel_from_nodepath(^"./country_pop/pop_list")
	if not pop_list_panel:
		return

	if _pop_list_scrollbar:
		_pop_list_scrollbar.value_changed.connect(
			func(value: int) -> void:
				_pop_list_scroll_index = value
				_update_pop_list()
		)

		pop_list_panel.gui_input.connect(
			func(event: InputEvent) -> void:
				if event is InputEventMouseButton:
					if event.is_pressed():
						if event.get_button_index() == MOUSE_BUTTON_WHEEL_UP:
							_pop_list_scrollbar.decrement_value()
						elif event.get_button_index() == MOUSE_BUTTON_WHEEL_DOWN:
							_pop_list_scrollbar.increment_value()
		)

	var height: float = 0.0
	while height < pop_list_panel.size.y:
		var pop_row_panel: Panel = GUINode.generate_gui_element(_scene_name, "popinfomember_popview")
		if not pop_row_panel:
			break

		pop_list_panel.add_child(pop_row_panel)
		pop_row_panel.set_position(Vector2(0, height))
		height += pop_row_panel.size.y
		_pop_list_rows.push_back(pop_row_panel)

		_pop_list_size_labels.push_back(GUINode.get_gui_label_from_node(pop_row_panel.get_node(^"./pop_size")))

		var pop_type_button: GUIIconButton = GUINode.get_gui_icon_button_from_node(pop_row_panel.get_node(^"./pop_type"))
		# TODO - open pop details menu on pop type button press
		_pop_list_type_buttons.push_back(pop_type_button)

		_pop_list_producing_icons.push_back(GUINode.get_gui_icon_from_node(pop_row_panel.get_node(^"./pop_producing_icon")))

		_pop_list_culture_labels.push_back(GUINode.get_gui_label_from_node(pop_row_panel.get_node(^"./pop_nation")))

		_pop_list_religion_icons.push_back(GUINode.get_gui_icon_from_node(pop_row_panel.get_node(^"./pop_religion")))

		_pop_list_location_labels.push_back(GUINode.get_gui_label_from_node(pop_row_panel.get_node(^"./pop_location")))

		_pop_list_militancy_labels.push_back(GUINode.get_gui_label_from_node(pop_row_panel.get_node(^"./pop_mil")))

		_pop_list_consciousness_labels.push_back(GUINode.get_gui_label_from_node(pop_row_panel.get_node(^"./pop_con")))

		_pop_list_ideology_charts.push_back(GUINode.get_gui_pie_chart_from_node(pop_row_panel.get_node(^"./pop_ideology")))

		_pop_list_issues_charts.push_back(GUINode.get_gui_pie_chart_from_node(pop_row_panel.get_node(^"./pop_issues")))

		_pop_list_unemployment_progressbars.push_back(GUINode.get_gui_progress_bar_from_node(pop_row_panel.get_node(^"./pop_unemployment_bar")))

		_pop_list_cash_labels.push_back(GUINode.get_gui_label_from_node(pop_row_panel.get_node(^"./pop_cash")))

		var pop_list_life_needs_progressbar: GUIProgressBar = GUINode.get_gui_progress_bar_from_node(pop_row_panel.get_node(^"./lifeneed_progress"))
		_pop_list_life_needs_progressbars.push_back(pop_list_life_needs_progressbar)
		if pop_list_life_needs_progressbar:
			pop_list_life_needs_progressbar.position += Vector2(1, 0)

		var pop_list_everyday_needs_progressbar: GUIProgressBar = GUINode.get_gui_progress_bar_from_node(pop_row_panel.get_node(^"./eveneed_progress"))
		_pop_list_everyday_needs_progressbars.push_back(pop_list_everyday_needs_progressbar)
		if pop_list_everyday_needs_progressbar:
			pop_list_everyday_needs_progressbar.position += Vector2(1, 0)

		_pop_list_luxury_needs_progressbars.push_back(GUINode.get_gui_progress_bar_from_node(pop_row_panel.get_node(^"./luxneed_progress")))

		_pop_list_rebel_icons.push_back(GUINode.get_gui_icon_from_node(pop_row_panel.get_node(^"./pop_revolt")))

		_pop_list_social_movement_icons.push_back(GUINode.get_gui_icon_from_node(pop_row_panel.get_node(^"./pop_movement_social")))

		_pop_list_political_movement_icons.push_back(GUINode.get_gui_icon_from_node(pop_row_panel.get_node(^"./pop_movement_political")))

		_pop_list_national_movement_flags.push_back(GUINode.get_gui_masked_flag_from_node(pop_row_panel.get_node(^"./pop_movement_flag")))

		_pop_list_size_change_icons.push_back(GUINode.get_gui_icon_from_node(pop_row_panel.get_node(^"./growth_indicator")))

		_pop_list_literacy_labels.push_back(GUINode.get_gui_label_from_node(pop_row_panel.get_node(^"./pop_literacy")))

func _notification(what: int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			MenuSingleton.population_menu_update_locale_sort_cache()
			_update_info()

func _on_update_active_nation_management_screen(active_screen: NationManagement.Screen) -> void:
	_active = active_screen == _screen
	_update_info()

func _update_info() -> void:
	if _active:
		# Province list
		_update_province_list()

		# Pop filter buttons, Distributions, Pop list
		_update_pops()

		show()
	else:
		hide()

func get_growth_icon_index(size_change: int) -> int:
	return 1 + int(size_change <= 0) + int(size_change < 0)

func _update_province_list(scroll_index: int = -1) -> void:
	if not _province_listbox:
		return

	if scroll_index >= 0:
		_province_listbox.set_scroll_index(scroll_index, false)

	_province_list_scroll_index = _province_listbox.get_scroll_index()

	var province_list_info_list: Array[Dictionary] = MenuSingleton.get_population_menu_province_list_rows(_province_list_scroll_index, _province_listbox.get_fixed_visible_items())

	for index: int in province_list_info_list.size():
		const type_key: StringName = &"type"
		const index_key: StringName = &"index"
		const name_key: StringName = &"name"
		const size_key: StringName = &"size"
		const change_key: StringName = &"change"
		const selected_key: StringName = &"selected"
		const expanded_key: StringName = &"expanded"
		const colony_key: StringName = &"colony"

		var province_list_info: Dictionary = province_list_info_list[index]

		var type: MenuSingleton.ProvinceListEntry = province_list_info[type_key]

		if _generate_province_list_row(index, type) != OK:
			continue

		if type == MenuSingleton.LIST_ENTRY_NONE or type != _province_list_types[index]:
			continue

		_province_list_indices[index] = province_list_info[index_key]

		if _province_list_buttons[index]:
			_province_list_buttons[index].set_icon_index(1 + int(province_list_info[selected_key]))

		if _province_list_name_labels[index]:
			_province_list_name_labels[index].set_text(
				GUINode.format_province_name(province_list_info[name_key]) if type == MenuSingleton.LIST_ENTRY_PROVINCE
				else province_list_info[name_key]
			)

		if _province_list_size_labels[index]:
			_province_list_size_labels[index].set_text(GUINode.int_to_string_suffixed(province_list_info[size_key]))

		if _province_list_growth_icons[index]:
			_province_list_growth_icons[index].set_icon_index(get_growth_icon_index(province_list_info[change_key]))

		if type == MenuSingleton.LIST_ENTRY_STATE:
			if _province_list_colony_buttons[index]:
				_province_list_colony_buttons[index].set_visible(province_list_info[colony_key])

			if _province_list_expand_buttons[index]:
				_province_list_expand_buttons[index].set_icon_index(1 + int(province_list_info[expanded_key]))

			# TODO - set _province_list_national_focus_buttons[index]

	# Clear any excess rows
	for index: int in range(province_list_info_list.size(), _province_list_types.size()):
		_generate_province_list_row(index, MenuSingleton.LIST_ENTRY_NONE)

func _update_pops() -> void:
	_update_pop_filters()
	_update_distributions()
	_update_pop_list()

func _update_pop_filters() -> void:
	var pop_filter_info_list: Array[Dictionary] = MenuSingleton.get_population_menu_pop_filter_info()

	for index: int in pop_filter_info_list.size():
		const pop_filter_count_key: StringName = &"count"
		const pop_filter_change_key: StringName = &"change"
		const pop_filter_selected_key: StringName = &"selected"

		var pop_filter_info: Dictionary = pop_filter_info_list[index]

		var pop_filter_button: GUIIconButton = _pop_filter_buttons[index]
		if not pop_filter_button:
			continue
		pop_filter_button.disabled = pop_filter_info[pop_filter_count_key] <= 0

		const normal_theme: StringName = &"normal"
		const hover_theme: StringName = &"hover"

		if pop_filter_info[pop_filter_selected_key] or pop_filter_button.disabled:
			pop_filter_button.get_theme_stylebox(normal_theme).set_texture(_pop_filter_icons[index])
			pop_filter_button.get_theme_stylebox(hover_theme).set_texture(_pop_filter_hover_icons[index])
		else:
			pop_filter_button.get_theme_stylebox(normal_theme).set_texture(_pop_filter_selected_icons[index])
			pop_filter_button.get_theme_stylebox(hover_theme).set_texture(_pop_filter_selected_icons[index])
		# TODO - size and promotion/demotion change tooltip

func _update_distributions():
	const slice_identifier_key: StringName = &"identifier"
	const slice_colour_key: StringName = &"colour"
	const slice_weight_key: StringName = &"weight"

	var distribution_info_list: Array[Array] = MenuSingleton.get_population_menu_distribution_info()

	for distribution_index: int in distribution_info_list.size():
		var distribution_info: Array[Dictionary] = distribution_info_list[distribution_index]

		if _distribution_charts[distribution_index]:
			_distribution_charts[distribution_index].set_slices_array(distribution_info)

		if _distribution_lists[distribution_index]:
			distribution_info.sort_custom(func(a: Dictionary, b: Dictionary) -> bool: return a[slice_weight_key] > b[slice_weight_key])

			var list: GUIListBox = _distribution_lists[distribution_index]

			list.clear_children(distribution_info.size())

			while list.get_child_count() < distribution_info.size():
				var child: Panel = GUINode.generate_gui_element(_scene_name, "pop_legend_item")
				if not child:
					break
				child.set_mouse_filter(Control.MOUSE_FILTER_IGNORE)
				list.add_child(child)

			for list_index: int in min(list.get_child_count(), distribution_info.size()):

				var child: Panel = list.get_child(list_index)

				var distribution_row: Dictionary = distribution_info[list_index]

				var colour_icon: GUIIcon = GUINode.get_gui_icon_from_node(child.get_node(^"./legend_color"))
				if colour_icon:
					colour_icon.set_modulate(distribution_row[slice_colour_key])
					colour_icon.set_tooltip_string_and_substitution_dict(
						"§Y$ID$§!: $PC$%" + "\nTEST: colour_icon",
						{
							"ID": distribution_row[slice_identifier_key],
							"PC": GUINode.float_to_string_dp(distribution_row[slice_weight_key] * 100.0, 2)
						}
					)

				var identifier_label: GUILabel = GUINode.get_gui_label_from_node(child.get_node(^"./legend_title"))
				if identifier_label:
					identifier_label.set_text(distribution_row[slice_identifier_key])

				var weight_label: GUILabel = GUINode.get_gui_label_from_node(child.get_node(^"./legend_value"))
				if weight_label:
					weight_label.set_text("%s%%" % GUINode.float_to_string_dp(distribution_row[slice_weight_key] * 100.0, 1))

func _update_pop_list() -> void:
	if _pop_list_scrollbar:
		var max_scroll_index: int = MenuSingleton.get_population_menu_pop_row_count() - _pop_list_rows.size()
		if max_scroll_index > 0:
			_pop_list_scrollbar.set_limits(0, max_scroll_index)
			_pop_list_scrollbar.show()
		else:
			_pop_list_scrollbar.set_limits(0, 0)
			_pop_list_scrollbar.hide()

	var pop_rows = MenuSingleton.get_population_menu_pop_rows(_pop_list_scroll_index, _pop_list_rows.size())

	for index: int in _pop_list_rows.size():
		if not _pop_list_rows[index]:
			continue
		if index < pop_rows.size():
			const pop_size_key: StringName = &"size"
			const pop_type_icon_key: StringName = &"pop_type_icon"
			const pop_culture_key: StringName = &"culture"
			const pop_religion_icon_key: StringName = &"religion_icon"
			const pop_location_key: StringName = &"location"
			const pop_militancy_key: StringName = &"militancy"
			const pop_consciousness_key: StringName = &"consciousness"
			const pop_ideology_key: StringName = &"ideology"
			const pop_issues_key: StringName = &"issues"
			const pop_unemployment_key: StringName = &"unemployment"
			const pop_cash_key: StringName = &"cash"
			const pop_life_needs_key: StringName = &"life_needs"
			const pop_everyday_needs_key: StringName = &"everyday_needs"
			const pop_luxury_needs_key: StringName = &"luxury_needs"
			const pop_rebel_icon_key: StringName = &"rebel_icon"
			const pop_size_change_key: StringName = &"size_change"
			const pop_literacy_key: StringName = &"literacy"

			var pop_row: Dictionary = pop_rows[index]

			if _pop_list_size_labels[index]:
				_pop_list_size_labels[index].set_text(GUINode.int_to_string_suffixed(pop_row[pop_size_key]))
			if _pop_list_type_buttons[index]:
				_pop_list_type_buttons[index].set_icon_index(pop_row[pop_type_icon_key])
				# TODO - replace with actual poptype
				_pop_list_type_buttons[index].set_tooltip_string("Pop Type #%d" % pop_row[pop_type_icon_key])
			if _pop_list_culture_labels[index]:
				_pop_list_culture_labels[index].set_text(pop_row[pop_culture_key])
				_pop_list_culture_labels[index].set_tooltip_string("NO_ASSIM_NOW")
			if _pop_list_religion_icons[index]:
				_pop_list_religion_icons[index].set_icon_index(pop_row[pop_religion_icon_key])
				# TODO - replace with actual religion
				_pop_list_religion_icons[index].set_tooltip_string("Religion #%d" % pop_row[pop_religion_icon_key])
			if _pop_list_location_labels[index]:
				var province_name: String = GUINode.format_province_name(pop_row.get(pop_location_key, ""))
				_pop_list_location_labels[index].set_text(province_name)
				_pop_list_location_labels[index].set_tooltip_string(province_name)
			if _pop_list_militancy_labels[index]:
				_pop_list_militancy_labels[index].set_text(GUINode.float_to_string_dp(pop_row[pop_militancy_key], 2))
				# TODO - test tooltip, add monthly change + source breakdown
				_pop_list_militancy_labels[index].set_tooltip_string("POP_MIL_TOTAL")
			if _pop_list_consciousness_labels[index]:
				_pop_list_consciousness_labels[index].set_text(GUINode.float_to_string_dp(pop_row[pop_consciousness_key], 2))
				# TODO - test tooltip, add monthly change + source breakdown
				_pop_list_consciousness_labels[index].set_tooltip_string("POP_CON_TOTAL")
			if _pop_list_ideology_charts[index]:
				_pop_list_ideology_charts[index].set_slices_array(pop_row[pop_ideology_key])
			if _pop_list_issues_charts[index]:
				_pop_list_issues_charts[index].set_slices_array(pop_row[pop_issues_key])
			if _pop_list_unemployment_progressbars[index]:
				var unemployment: float = pop_row[pop_unemployment_key]
				_pop_list_unemployment_progressbars[index].set_value_no_signal(unemployment)
				_pop_list_unemployment_progressbars[index].set_tooltip_string("%s: §Y%s%%" % [
					tr("UNEMPLOYMENT"), GUINode.float_to_string_dp(unemployment * 100.0, 3)
				])
			if _pop_list_cash_labels[index]:
				_pop_list_cash_labels[index].set_text("%s¤" % GUINode.float_to_string_dp(pop_row[pop_cash_key], 2))
				_pop_list_cash_labels[index].set_tooltip_string_and_substitution_dict("POP_DAILY_MONEY", {
					"VAL": GUINode.float_to_string_dp(1.23, 2)
				})
			if _pop_list_life_needs_progressbars[index]:
				var life_needs: float = pop_row[pop_life_needs_key]
				_pop_list_life_needs_progressbars[index].set_value_no_signal(life_needs)
				_pop_list_life_needs_progressbars[index].set_tooltip_string_and_substitution_dict("GETTING_NEEDS", {
					"NEED": "LIFE_NEEDS", "VAL": GUINode.float_to_string_dp(life_needs * 100.0, 1)
				})
			if _pop_list_everyday_needs_progressbars[index]:
				var everyday_needs: float = pop_row[pop_everyday_needs_key]
				_pop_list_everyday_needs_progressbars[index].set_value_no_signal(everyday_needs)
				_pop_list_everyday_needs_progressbars[index].set_tooltip_string_and_substitution_dict("GETTING_NEEDS", {
					"NEED": "EVERYDAY_NEEDS", "VAL": GUINode.float_to_string_dp(everyday_needs * 100.0, 1)
				})
			if _pop_list_luxury_needs_progressbars[index]:
				var luxury_needs: float = pop_row[pop_luxury_needs_key]
				_pop_list_luxury_needs_progressbars[index].set_value_no_signal(luxury_needs)
				_pop_list_luxury_needs_progressbars[index].set_tooltip_string_and_substitution_dict("GETTING_NEEDS", {
					"NEED": "LUXURY_NEEDS", "VAL": GUINode.float_to_string_dp(luxury_needs * 100.0, 1)
				})
			if _pop_list_rebel_icons[index]:
				var rebel_icon: int = pop_row.get(pop_rebel_icon_key, 0)
				if rebel_icon > 0:
					_pop_list_rebel_icons[index].set_icon_index(rebel_icon)
					_pop_list_rebel_icons[index].show()
				else:
					_pop_list_rebel_icons[index].hide()

			# TODO - handle social/political reform and country rebels
			if _pop_list_social_movement_icons[index]:
				_pop_list_social_movement_icons[index].hide()
			if _pop_list_political_movement_icons[index]:
				_pop_list_political_movement_icons[index].hide()
			if _pop_list_national_movement_flags[index]:
				_pop_list_national_movement_flags[index].hide()

			if _pop_list_size_change_icons[index]:
				var pop_change: int = pop_row[pop_size_change_key]
				_pop_list_size_change_icons[index].set_icon_index(get_growth_icon_index(pop_change))
				_pop_list_size_change_icons[index].set_tooltip_string("%s §%s%s" % [
					tr("POPULATION_CHANGED_BY"), "G+" if pop_change > 0 else "Y+" if pop_change == 0 else "R", str(pop_change)
				])
			if _pop_list_literacy_labels[index]:
				_pop_list_literacy_labels[index].set_text("%s%%" % GUINode.float_to_string_dp(pop_row[pop_literacy_key], 2))
				_pop_list_literacy_labels[index].set_tooltip_string("%s: §G%s%%" % [
					tr("LIT_CHANGE"), GUINode.float_to_string_dp(pop_row[pop_literacy_key] / 64.0, 2)
				])

			_pop_list_rows[index].show()
		else:
			_pop_list_rows[index].hide()
