extends GUINode

# Header
var _province_name_label : Label
var _region_name_label : Label
var _slave_status_icon : TextureRect
var _colony_status_button : Button
var _colony_status_button_texture : GFXSpriteTexture
var _administrative_percentage_label : Label
var _owner_percentage_label : Label
var _province_modifiers_overlapping_elements_box : GUIOverlappingElementsBox
var _terrain_type_texture : GFXSpriteTexture
var _life_rating_bar : TextureProgressBar
var _controller_flag_texture : GFXMaskedFlagTexture

# Statistics
var _rgo_icon_texture : GFXSpriteTexture
var _rgo_produced_label : Label
var _rgo_income_label : Label
var _rgo_employment_percentage_texture : GFXSpriteTexture
var _rgo_employment_population_label : Label
var _rgo_employment_percentage_label : Label
var _crime_name_label : Label
var _crime_icon_texture : GFXSpriteTexture
var _crime_fighting_label : Label
var _total_population_label : Label
var _migration_label : Label
var _population_growth_label : Label
var _pop_types_piechart : GFXPieChartTexture
var _pop_ideologies_piechart : GFXPieChartTexture
var _pop_cultures_piechart : GFXPieChartTexture
var _supply_limit_label : Label
var _cores_overlapping_elements_box : GUIOverlappingElementsBox

# Buildings
var _buildings_panel : Panel
var _building_slots : Array[BuildingSlot]

# REQUIREMENTS:
# * UI-183, UI-185, UI-186, UI-765, UI-187, UI-188, UI-189
# * UI-191, UI-193, UI-194, UI-766, UI-195, UI-196, UI-197
# * UI-199, UI-201, UI-202, UI-767, UI-203, UI-204, UI-205
class BuildingSlot:
	var _slot_index : int
	var _slot_node : Control

	var _building_icon : GFXSpriteTexture
	var _expand_button : Button
	var _expanding_icon : TextureRect
	var _expanding_progress_bar : TextureProgressBar
	var _expanding_label : Label

	func _init(new_slot_index : int, new_slot_node : Control) -> void:
		if new_slot_index < 0:
			push_error("Invalid building slot index: ", new_slot_index)
			return
		_slot_index = new_slot_index
		if not new_slot_node:
			push_error("Invalid building slot node: null!")
			return
		_slot_node = new_slot_node

		for icon_index : int in MenuSingleton.get_province_building_count():
			var icon := _slot_node.get_node("build_icon%d" % icon_index)
			if icon:
				if icon_index == _slot_index:
					_building_icon = GUINode.get_gfx_sprite_texture_from_node(icon)
				else:
					icon.hide()

		var building_name := GUINode.get_label_from_node(_slot_node.get_node(^"./description"))
		if building_name:
			building_name.text = MenuSingleton.get_province_building_identifier(_slot_index)
		_expand_button = GUINode.get_button_from_node(_slot_node.get_node(^"./expand"))
		if _expand_button:
			_expand_button.pressed.connect(func() -> void: MenuSingleton.expand_selected_province_building(_slot_index))
		_expanding_icon = GUINode.get_texture_rect_from_node(_slot_node.get_node(^"./underconstruction_icon"))
		_expanding_progress_bar = GUINode.get_progress_bar_from_node(_slot_node.get_node(^"./building_progress"))
		if _expanding_progress_bar:
			_expanding_progress_bar.max_value = 1.0
			_expanding_progress_bar.step = _expanding_progress_bar.max_value / 100
		_expanding_label = GUINode.get_label_from_node(_slot_node.get_node(^"./expand_text"))

	enum ExpansionState { CannotExpand, CanExpand, Preparing, Expanding }

	func update_info(info : Dictionary) -> void:
		const building_info_level_key              : StringName = &"level"
		const building_info_expansion_state_key    : StringName = &"expansion_state"
		const building_info_start_date_key         : StringName = &"start_date"
		const building_info_end_date_key           : StringName = &"end_date"
		const building_info_expansion_progress_key : StringName = &"expansion_progress"

		if _building_icon:
			_building_icon.set_icon_index(info.get(building_info_level_key, 0) + 1)

		var expansion_state : int = info.get(building_info_expansion_state_key, ExpansionState.CannotExpand)
		var expansion_in_progress : bool = expansion_state == ExpansionState.Preparing or expansion_state == ExpansionState.Expanding

		if _expand_button:
			_expand_button.visible = not expansion_in_progress
			_expand_button.disabled = expansion_state != ExpansionState.CanExpand

		if _expanding_icon:
			_expanding_icon.visible = expansion_in_progress

		if _expanding_progress_bar:
			_expanding_progress_bar.visible = expansion_in_progress
			_expanding_progress_bar.value = info.get(building_info_expansion_progress_key, 0)

		if _expanding_label:
			_expanding_label.visible = expansion_in_progress

var _selected_index : int:
	get: return _selected_index
	set(v):
		_selected_index = v
		_update_info()
var _province_info : Dictionary

func _ready() -> void:
	GameSingleton.province_selected.connect(_on_province_selected)
	GameSingleton.gamestate_updated.connect(_update_info)

	if add_gui_element("province_interface", "province_view") != OK:
		push_error("Failed to generate province overview panel!")
		return

	# Disables all consuming invisible panel
	var prov_view := get_panel_from_nodepath(^"./province_view")
	if prov_view:
		prov_view.mouse_filter = Control.MOUSE_FILTER_IGNORE
	set_click_mask_from_nodepaths([^"./province_view/background"])

	var close_button : Button = get_button_from_nodepath(^"./province_view/close_button")
	if close_button:
		close_button.pressed.connect(_on_close_button_pressed)

	# Header
	_province_name_label = get_label_from_nodepath(^"./province_view/province_view_header/province_name")
	_region_name_label = get_label_from_nodepath(^"./province_view/province_view_header/state_name")
	_slave_status_icon = get_texture_rect_from_nodepath(^"./province_view/province_view_header/slave_state_icon")
	var slave_status_icon_texture : GFXSpriteTexture = get_gfx_sprite_texture_from_nodepath(^"./province_view/province_view_header/slave_state_icon")
	if slave_status_icon_texture:
		slave_status_icon_texture.set_icon_index(MenuSingleton.get_slave_pop_icon_index())
	_colony_status_button = get_button_from_nodepath(^"./province_view/province_view_header/colony_button")
	_colony_status_button_texture = get_gfx_sprite_texture_from_nodepath(^"./province_view/province_view_header/colony_button")
	var admin_icon_texture : GFXSpriteTexture = get_gfx_sprite_texture_from_nodepath(^"./province_view/province_view_header/admin_icon")
	if admin_icon_texture:
		admin_icon_texture.set_icon_index(MenuSingleton.get_administrative_pop_icon_index())
	_administrative_percentage_label = get_label_from_nodepath(^"./province_view/province_view_header/admin_efficiency")
	_owner_percentage_label = get_label_from_nodepath(^"./province_view/province_view_header/owner_presence")
	_province_modifiers_overlapping_elements_box = get_gui_overlapping_elements_box_from_nodepath(^"./province_view/province_view_header/province_modifiers")
	if _province_modifiers_overlapping_elements_box and _province_modifiers_overlapping_elements_box.set_gui_child_element_name("province_interface", "prov_state_modifier") != OK:
		_province_modifiers_overlapping_elements_box = null # hide province modifiers box since we can't do anything with it
	_terrain_type_texture = get_gfx_sprite_texture_from_nodepath(^"./province_view/province_view_header/prov_terrain")
	_life_rating_bar = get_progress_bar_from_nodepath(^"./province_view/province_view_header/liferating")
	_controller_flag_texture = get_gfx_masked_flag_texture_from_nodepath(^"./province_view/province_view_header/controller_flag")

	# Statistics
	_rgo_icon_texture = get_gfx_sprite_texture_from_nodepath(^"./province_view/province_statistics/goods_type")
	_rgo_produced_label = get_label_from_nodepath(^"./province_view/province_statistics/produced")
	_rgo_income_label = get_label_from_nodepath(^"./province_view/province_statistics/income")
	_rgo_employment_percentage_texture = get_gfx_sprite_texture_from_nodepath(^"./province_view/province_statistics/employment_ratio")
	_rgo_employment_population_label = get_label_from_nodepath(^"./province_view/province_statistics/rgo_population")
	_rgo_employment_percentage_label = get_label_from_nodepath(^"./province_view/province_statistics/rgo_percent")
	_crime_name_label = get_label_from_nodepath(^"./province_view/province_statistics/crime_name")
	_crime_icon_texture = get_gfx_sprite_texture_from_nodepath(^"./province_view/province_statistics/crime_icon")
	_crime_fighting_label = get_label_from_nodepath(^"./province_view/province_statistics/crimefight_percent")
	_total_population_label = get_label_from_nodepath(^"./province_view/province_statistics/total_population")
	_migration_label = get_label_from_nodepath(^"./province_view/province_statistics/migration")
	_population_growth_label = get_label_from_nodepath(^"./province_view/province_statistics/growth")
	_pop_types_piechart = get_gfx_pie_chart_texture_from_nodepath(^"./province_view/province_statistics/workforce_chart")
	_pop_ideologies_piechart = get_gfx_pie_chart_texture_from_nodepath(^"./province_view/province_statistics/ideology_chart")
	_pop_cultures_piechart = get_gfx_pie_chart_texture_from_nodepath(^"./province_view/province_statistics/culture_chart")
	var population_menu_button : Button = get_button_from_nodepath(^"./province_view/province_statistics/open_popscreen")
	if population_menu_button:
		population_menu_button.pressed.connect(
			func() -> void:
				MenuSingleton.population_menu_select_province(_selected_index)
				_on_close_button_pressed()
				Events.NationManagementScreens.open_nation_management_screen(NationManagement.Screen.POPULATION)
		)
	_supply_limit_label = get_label_from_nodepath(^"./province_view/province_statistics/supply_limit_label")
	_cores_overlapping_elements_box = get_gui_overlapping_elements_box_from_nodepath(^"./province_view/province_statistics/core_icons")
	if _cores_overlapping_elements_box and _cores_overlapping_elements_box.set_gui_child_element_name("province_interface", "province_core") != OK:
		_cores_overlapping_elements_box = null # hide cores box since we can't do anything with it

	_buildings_panel = get_panel_from_nodepath(^"./province_view/province_buildings")
	if _buildings_panel:
		var target_slot_count : int = MenuSingleton.get_province_building_count()
		var slot_y : float = 0.0
		for current_slot_count : int in target_slot_count:
			var slot := GUINode.generate_gui_element("province_interface", "building", "building_slot_%d" % current_slot_count)
			if slot:
				_buildings_panel.add_child(slot)
				slot.set_position(Vector2(0.0, slot_y))
				slot_y += slot.get_size().y
				_building_slots.push_back(BuildingSlot.new(current_slot_count, slot))
			else:
				push_error("Failed to generate building slot ", current_slot_count, " / ", target_slot_count)
				break

	hide_nodes([
		^"./province_view/province_view_header/state_modifiers",
		^"./province_view/province_view_header/occupation_progress",
		^"./province_view/province_view_header/occupation_icon",
		^"./province_view/province_view_header/occupation_flag",
		^"./province_view/province_colony",
		^"./province_view/province_other",
		^"./province_view/province_buildings/army_size",
		^"./province_view/province_buildings/army_text",
		^"./province_view/province_buildings/navy_text",
		^"./province_view/national_focus_window",
	])

	_update_info()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()

enum ColonyStatus { STATE, PROTECTORATE, COLONY }

# This assumes _cores_overlapping_elements_box is non-null
func _set_core_flag(core_index : int, country : String) -> void:
	var core_flag_texture : GFXMaskedFlagTexture = GUINode.get_gfx_masked_flag_texture_from_node(
		_cores_overlapping_elements_box.get_child(core_index).get_node(^"./country_flag")
	)
	if core_flag_texture:
		core_flag_texture.set_flag_country_name(country)

func _update_info() -> void:
	const _province_info_province_key         : StringName = &"province"
	const _province_info_state_key            : StringName = &"state"
	const _province_info_slave_status_key     : StringName = &"slave_status"
	const _province_info_colony_status_key    : StringName = &"colony_status"
	const _province_info_terrain_type_key     : StringName = &"terrain_type"
	const _province_info_life_rating_key      : StringName = &"life_rating"
	const _province_info_controller_key       : StringName = &"controller"
	const _province_info_rgo_name_key         : StringName = &"rgo_name"
	const _province_info_rgo_icon_key         : StringName = &"rgo_icon"
	const _province_info_crime_name_key       : StringName = &"crime_name"
	const _province_info_crime_icon_key       : StringName = &"crime_icon"
	const _province_info_total_population_key : StringName = &"total_population"
	const _province_info_pop_types_key        : StringName = &"pop_types"
	const _province_info_pop_ideologies_key   : StringName = &"pop_ideologies"
	const _province_info_pop_cultures_key     : StringName = &"pop_cultures"
	const _province_info_cores_key            : StringName = &"cores"
	const _province_info_buildings_key        : StringName = &"buildings"

	const _missing_suffix : String = "_MISSING"

	_province_info = MenuSingleton.get_province_info_from_index(_selected_index)
	if _province_info:
		# Header
		if _province_name_label:
			_province_name_label.text = GUINode.format_province_name(_province_info.get(_province_info_province_key, _missing_suffix))

		if _region_name_label:
			_region_name_label.text = _province_info.get(_province_info_state_key,
				_province_info_state_key + _missing_suffix)

		if _slave_status_icon:
			_slave_status_icon.visible = _province_info.get(_province_info_slave_status_key, false)

		var colony_status : ColonyStatus = _province_info.get(_province_info_colony_status_key, 0)
		if _colony_status_button:
			if colony_status == ColonyStatus.STATE:
				_colony_status_button.hide()
			else:
				if _colony_status_button_texture:
					_colony_status_button_texture.set_icon_index(colony_status)
				_colony_status_button.show()

		if _administrative_percentage_label:
			pass

		if _owner_percentage_label:
			pass

		if _province_modifiers_overlapping_elements_box:
			# TODO - replace example icons with those from the province's list of modifier instances
			_province_modifiers_overlapping_elements_box.set_child_count(8)
			for i : int in _province_modifiers_overlapping_elements_box.get_child_count():
				var icon : GFXSpriteTexture = GUINode.get_gfx_sprite_texture_from_node(
					_province_modifiers_overlapping_elements_box.get_child(i).get_node(^"./modifier")
				)
				if icon:
					icon.set_icon_index(2 * i + (i & 1) + 1)

		if _terrain_type_texture:
			var terrain_type : String = _province_info.get(_province_info_terrain_type_key, "")
			if terrain_type:
				const _terrain_type_prefix : String = "GFX_terrainimg_"
				if _terrain_type_texture.set_gfx_texture_sprite_name(_terrain_type_prefix + terrain_type) != OK:
					push_error("Failed to set terrain type texture: ", terrain_type)

		if _life_rating_bar:
			_life_rating_bar.value = _province_info.get(_province_info_life_rating_key, 0) / 100.0

		if _controller_flag_texture:
			_controller_flag_texture.set_flag_country_name(_province_info.get(_province_info_controller_key, ""))

		# Statistics
		if _rgo_icon_texture:
			_rgo_icon_texture.set_icon_index(_province_info.get(_province_info_rgo_icon_key, -1) + 2)

		if _rgo_produced_label:
			# TODO - replace name with amount produced
			_rgo_produced_label.text = _province_info.get(_province_info_rgo_name_key, _province_info_rgo_name_key + _missing_suffix)

		if _rgo_income_label:
			# TODO - add £ sign and replace placeholder with actual value
			_rgo_income_label.text = "%s £" % GUINode.float_to_string_dp(12.34567, 3)

		if _rgo_employment_percentage_texture:
			pass

		if _rgo_employment_population_label:
			# TODO - replace placeholder with actual value
			_rgo_employment_population_label.text = GUINode.int_to_string_suffixed(_province_info.get(_province_info_total_population_key, 0) / 10)

		if _rgo_employment_percentage_label:
			pass

		if _crime_name_label:
			_crime_name_label.text = _province_info.get(_province_info_crime_name_key, "")

		if _crime_icon_texture:
			_crime_icon_texture.set_icon_index(_province_info.get(_province_info_crime_icon_key, 0) + 1)

		if _crime_fighting_label:
			pass

		if _total_population_label:
			_total_population_label.text = GUINode.int_to_string_suffixed(_province_info.get(_province_info_total_population_key, 0))

		if _migration_label:
			pass

		if _population_growth_label:
			pass

		if _pop_types_piechart:
			_pop_types_piechart.set_slices_array(_province_info.get(_province_info_pop_types_key, []))

		if _pop_ideologies_piechart:
			_pop_ideologies_piechart.set_slices_array(_province_info.get(_province_info_pop_ideologies_key, []))

		if _pop_cultures_piechart:
			_pop_cultures_piechart.set_slices_array(_province_info.get(_province_info_pop_cultures_key, []))

		if _supply_limit_label:
			pass

		if _cores_overlapping_elements_box:
			var cores : PackedStringArray = _province_info.get(_province_info_cores_key, [])
			_cores_overlapping_elements_box.set_child_count(cores.size())
			for core_index : int in min(cores.size(), _cores_overlapping_elements_box.get_child_count()):
				_set_core_flag(core_index, cores[core_index])
			for core_index : int in range(cores.size(), _cores_overlapping_elements_box.get_child_count()):
				_set_core_flag(core_index, "")

		# Buildings
		if _buildings_panel:
			var buildings : Array[Dictionary] = _province_info.get(_province_info_buildings_key, [] as Array[Dictionary])
			for slot_index : int in min(buildings.size(), _building_slots.size()):
				_building_slots[slot_index].update_info(buildings[slot_index])
			for slot_index : int in range(buildings.size(), _building_slots.size()):
				_building_slots[slot_index].update_info({})

		show()
	else:
		hide()
		mouse_exited.emit()

func _on_province_selected(index : int) -> void:
	_selected_index = index

func _on_close_button_pressed() -> void:
	GameSingleton.unset_selected_province()
