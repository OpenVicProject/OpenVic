extends GUINode

# Header
var _province_name_label : Label
var _region_name_label : Label
var _slave_status_icon : TextureRect
var _colony_status_button : Button
var _colony_status_button_texture : GFXIconTexture
var _administrative_percentage_label : Label
var _owner_percentage_label : Label
var _terrain_type_texture : GFXIconTexture
var _life_rating_bar : TextureProgressBar
var _controller_flag_texture : GFXMaskedFlagTexture
# state and province modifiers

# Statistics
var _rgo_icon_texture : GFXIconTexture
var _rgo_produced_label : Label
var _rgo_income_label : Label
# rgo employment progress bar (execpt it isn't a progress bar?)
var _rgo_employment_population_label : Label
var _rgo_employment_percentage_label : Label
var _crime_name_label : Label
var _crime_icon_texture : GFXIconTexture
# crime fighting
var _total_population_label : Label
var _migration_label : Label
var _population_growth_label : Label
var _pop_types_piechart : GFXPieChartTexture
var _pop_ideologies_piechart : GFXPieChartTexture
var _pop_cultures_piechart : GFXPieChartTexture
# supply_limit_label
# cores

const _missing_suffix : String = "_MISSING"

const _province_info_province_key : StringName = &"province"
const _province_info_region_key : StringName = &"region"
const _province_info_controller_key : StringName = &"controller"
const _province_info_life_rating_key : StringName = &"life_rating"
const _province_info_terrain_type_key : StringName = &"terrain_type"
const _province_info_crime_name_key : StringName = &"crime_name"
const _province_info_crime_icon_key : StringName = &"crime_icon"
const _province_info_total_population_key : StringName = &"total_population"
const _province_info_pop_types_key : StringName = &"pop_types"
const _province_info_pop_ideologies_key : StringName = &"pop_ideologies"
const _province_info_pop_cultures_key : StringName = &"pop_cultures"
const _province_info_rgo_name_key : StringName = &"rgo_name"
const _province_info_rgo_icon_key : StringName = &"rgo_icon"
const _province_info_colony_status_key : StringName = &"colony_status"
const _province_info_slave_status_key : StringName = &"slave_status"
const _province_info_buildings_key : StringName = &"buildings"

const _building_info_building_key : StringName = &"building"
const _building_info_level_key : StringName = &"level"
const _building_info_expansion_state_key : StringName = &"expansion_state"
const _building_info_start_date_key : StringName = &"start_date"
const _building_info_end_date_key : StringName = &"end_date"
const _building_info_expansion_progress_key : StringName = &"expansion_progress"

const _piechart_info_size_key : StringName = &"size"
const _piechart_info_colour_key : StringName = &"colour"

var _selected_index : int:
	get: return _selected_index
	set(v):
		_selected_index = v
		_update_info()
var _province_info : Dictionary

func _ready():
	GameSingleton.province_selected.connect(_on_province_selected)
	GameSingleton.state_updated.connect(_update_info)

	add_gui_element("province_interface.gui", "province_view")

	var close_button : Button = get_button_node(^"./province_view/close_button")
	if close_button:
		close_button.pressed.connect(_on_close_button_pressed)

	# Header
	_province_name_label = get_label_node(^"./province_view/province_view_header/province_name")
	_region_name_label = get_label_node(^"./province_view/province_view_header/state_name")
	_slave_status_icon = get_texture_rect_node(^"./province_view/province_view_header/slave_state_icon")
	var slave_status_icon_texture : GFXIconTexture = get_gfx_icon_texture_from_node(^"./province_view/province_view_header/slave_state_icon")
	if slave_status_icon_texture:
		slave_status_icon_texture.set_icon_index(GameSingleton.get_slave_pop_icon_index())
	_colony_status_button = get_button_node(^"./province_view/province_view_header/colony_button")
	_colony_status_button_texture = get_gfx_icon_texture_from_node(^"./province_view/province_view_header/colony_button")
	var admin_icon_texture : GFXIconTexture = get_gfx_icon_texture_from_node(^"./province_view/province_view_header/admin_icon")
	if admin_icon_texture:
		admin_icon_texture.set_icon_index(GameSingleton.get_administrative_pop_icon_index())
	_administrative_percentage_label = get_label_node(^"./province_view/province_view_header/admin_efficiency")
	_owner_percentage_label = get_label_node(^"./province_view/province_view_header/owner_presence")
	_terrain_type_texture = get_gfx_icon_texture_from_node(^"./province_view/province_view_header/prov_terrain")
	_life_rating_bar = get_progress_bar_node(^"./province_view/province_view_header/liferating")
	_controller_flag_texture = get_gfx_masked_flag_texture_from_node(^"./province_view/province_view_header/controller_flag")

	# Statistics
	_rgo_icon_texture = get_gfx_icon_texture_from_node(^"./province_view/province_statistics/goods_type")
	_rgo_produced_label = get_label_node(^"./province_view/province_statistics/produced")
	_rgo_income_label = get_label_node(^"./province_view/province_statistics/income")
	_rgo_employment_population_label = get_label_node(^"./province_view/province_statistics/rgo_population")
	_rgo_employment_percentage_label = get_label_node(^"./province_view/province_statistics/rgo_percent")
	_crime_name_label = get_label_node(^"./province_view/province_statistics/crime_name")
	_crime_icon_texture = get_gfx_icon_texture_from_node(^"./province_view/province_statistics/crime_icon")
	_total_population_label = get_label_node(^"./province_view/province_statistics/total_population")
	_migration_label = get_label_node(^"./province_view/province_statistics/migration")
	_population_growth_label = get_label_node(^"./province_view/province_statistics/growth")
	_pop_types_piechart = get_gfx_pie_chart_texture_from_node(^"./province_view/province_statistics/workforce_chart")
	_pop_ideologies_piechart = get_gfx_pie_chart_texture_from_node(^"./province_view/province_statistics/ideology_chart")
	_pop_cultures_piechart = get_gfx_pie_chart_texture_from_node(^"./province_view/province_statistics/culture_chart")

	#^"./province_view/building"
	#^"./province_view/province_core"
	#^"./province_view/prov_state_modifier"

	#add_gui_element("province_interface.gui", "building", "building0")
	#var building0 : Panel = get_panel_node(^"./building0")
	#building0.set_anchors_and_offsets_preset(Control.PRESET_BOTTOM_LEFT)
	#building0.set_position(pop_cultures_piechart_icon.get_position())

	# TODO - fix checkbox positions
	for path in [
		^"./province_view/province_buildings/rallypoint_checkbox",
		^"./province_view/province_buildings/rallypoint_merge_checkbox",
		^"./province_view/province_buildings/rallypoint_checkbox_naval",
		^"./province_view/province_buildings/rallypoint_merge_checkbox_naval"
	]:
		var rally_checkbox : CheckBox = get_check_box_node(path)
		rally_checkbox.set_position(rally_checkbox.get_position() - Vector2(3, 3))

	hide_nodes([
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

func _notification(what : int):
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()
"""
enum { CANNOT_EXPAND, CAN_EXPAND, PREPARING, EXPANDING }

func _expand_building(building_identifier : String) -> void:
	if GameSingleton.expand_building(_selected_index, building_identifier) != OK:
		push_error("Failed to expand ", building_identifier, " in province #", _selected_index);

# Each building row contains:
# level        - Level Label
# name         - Name Label
# button       - Expansion Button
# progress_bar - Expansion ProgressBar
var _building_rows : Array[Dictionary]


# REQUIREMENTS:
# * UI-183, UI-185, UI-186, UI-765, UI-187, UI-188, UI-189
# * UI-191, UI-193, UI-194, UI-766, UI-195, UI-196, UI-197
# * UI-199, UI-201, UI-202, UI-767, UI-203, UI-204, UI-205
func _add_building_row() -> void:
	var row : Dictionary = {}

	row.level = Label.new()
	row.level.text = "X"
	_buildings_container.add_child(row.level)

	row.name = Label.new()
	row.name.text = _building_info_building_key + _missing_suffix
	_buildings_container.add_child(row.name)

	row.button = Button.new()
	row.button.text = "EXPAND_PROVINCE_BUILDING"
	row.button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	row.button.mouse_filter = Control.MOUSE_FILTER_PASS
	row.button.focus_mode = FOCUS_NONE
	row.button.pressed.connect(func(): _expand_building(row.name.text))
	_buildings_container.add_child(row.button)

	row.progress_bar = ProgressBar.new()
	row.progress_bar.max_value = 1
	row.progress_bar.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	row.progress_bar.mouse_filter = Control.MOUSE_FILTER_PASS
	_buildings_container.add_child(row.progress_bar)

	_building_rows.append(row)
	_set_building_row(_building_rows.size() - 1, {})

func _set_building_row(index : int, building : Dictionary) -> void:
	if index < 0 or index > _building_rows.size():
		push_error("Invalid building row index: ", index, " (max ", _building_rows.size(), ")")
		return
	if index == _building_rows.size(): _add_building_row()
	var row := _building_rows[index]
	if building.is_empty():
		row.level.visible = false
		row.name.visible = false
		row.progress_bar.visible = false
		row.button.visible = false
		return
	row.level.text = str(building.get(_building_info_level_key, 0))
	row.level.visible = true
	row.name.text = building.get(_building_info_building_key,
		_building_info_building_key + _missing_suffix)
	row.name.visible = true

	var expansion_state : int = building.get(_building_info_expansion_state_key,
		CANNOT_EXPAND)
	var show_progress_bar := expansion_state == PREPARING or expansion_state == EXPANDING
	row.progress_bar.value = building.get(_building_info_expansion_progress_key, 0)
	row.progress_bar.visible = show_progress_bar
	row.button.disabled = expansion_state != CAN_EXPAND
	row.button.visible = not show_progress_bar
"""

enum { STATE, PROTECTORATE, COLONY }
func _update_info() -> void:
	_province_info = GameSingleton.get_province_info_from_index(_selected_index)
	if _province_info:
		# Header
		if _province_name_label:
			_province_name_label.text = "PROV" + _province_info.get(_province_info_province_key,
				_province_info_province_key + _missing_suffix)

		if _region_name_label:
			_region_name_label.text = _province_info.get(_province_info_region_key,
				_province_info_region_key + _missing_suffix)

		if _slave_status_icon:
			_slave_status_icon.visible = _province_info.get(_province_info_slave_status_key, false)

		var colony_status : int = _province_info.get(_province_info_colony_status_key, 0)
		if _colony_status_button:
			if colony_status == STATE:
				_colony_status_button.hide()
			else:
				if _colony_status_button_texture:
					_colony_status_button_texture.set_icon_index(colony_status)
				_colony_status_button.show()

		if _administrative_percentage_label:
			pass

		if _owner_percentage_label:
			pass

		if _terrain_type_texture:
			var terrain_type : String = _province_info.get(_province_info_terrain_type_key, "")
			if terrain_type:
				const _terrain_type_prefix : String = "GFX_terrainimg_"
				if _terrain_type_texture.set_gfx_texture_sprite_name(_terrain_type_prefix + terrain_type) != OK:
					push_error("Failed to set terrain type texture: ", terrain_type)

		if _life_rating_bar:
			_life_rating_bar.value = _province_info.get(_province_info_life_rating_key, 0)

		if _controller_flag_texture:
			var controller : String = _province_info.get(_province_info_controller_key, "REB")
			_controller_flag_texture.set_flag_country_name_and_type(controller, &"")

		# Statistics
		if _rgo_icon_texture:
			_rgo_icon_texture.set_icon_index(_province_info.get(_province_info_rgo_icon_key, -1) + 2)

		if _rgo_produced_label:
			_rgo_produced_label.text = _province_info.get(_province_info_rgo_name_key, _province_info_rgo_name_key + _missing_suffix)

		if _rgo_income_label:
			_rgo_income_label.text = GameSingleton.float_to_formatted_string(12.34567)
			# TODO - add £ sign

		if _rgo_employment_population_label:
			_rgo_employment_population_label.text = GameSingleton.int_to_formatted_string(_province_info.get(_province_info_total_population_key, 0) / 10)

		if _rgo_employment_percentage_label:
			pass

		if _crime_name_label:
			_crime_name_label.text = _province_info.get(_province_info_crime_name_key, "")

		if _crime_icon_texture:
			_crime_icon_texture.set_icon_index(_province_info.get(_province_info_crime_icon_key, 0) + 1)

		if _total_population_label:
			_total_population_label.text = GameSingleton.int_to_formatted_string(_province_info.get(_province_info_total_population_key, 0))

		if _migration_label:
			pass

		if _population_growth_label:
			pass

		if _pop_types_piechart:
			_pop_types_piechart.set_slices(_province_info.get(_province_info_pop_types_key, []))

		if _pop_ideologies_piechart:
			_pop_ideologies_piechart.set_slices(_province_info.get(_province_info_pop_ideologies_key, []))

		if _pop_cultures_piechart:
			_pop_cultures_piechart.set_slices(_province_info.get(_province_info_pop_cultures_key, []))

		#var buildings : Array = _province_info.get(_province_info_buildings_key, [])
		#for i in max(buildings.size(), _building_rows.size()):
		#	_set_building_row(i, buildings[i] if i < buildings.size() else {})

		show()
	else:
		hide()
		mouse_exited.emit()

func _on_province_selected(index : int) -> void:
	_selected_index = index

func _on_close_button_pressed() -> void:
	GameSingleton.set_selected_province(0)
