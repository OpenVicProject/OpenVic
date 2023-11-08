extends Control

var _province_name_label : Label
var _region_name_label : Label
var _life_rating_bar : TextureProgressBar
var _total_population_label : Label
var _rgo_icon_texture : AtlasTexture

const _missing_suffix : String = "_MISSING"

const _province_info_province_key : StringName = &"province"
const _province_info_region_key : StringName = &"region"
const _province_info_life_rating_key : StringName = &"life_rating"
const _province_info_terrain_type_key : StringName = &"terrain_type"
const _province_info_total_population_key : StringName = &"total_population"
const _province_info_pop_types_key : StringName = &"pop_types"
const _province_info_pop_ideologies_key : StringName = &"pop_ideologies"
const _province_info_pop_cultures_key : StringName = &"pop_cultures"
const _province_info_rgo_key : StringName = &"rgo"
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

func _check_class(object : Object, klass : String, name : String) -> bool:
	if object.get_class() == klass:
		return true
	else:
		push_error("Invalid ", name, " class: ", object.get_class(), " (expected ", klass, ")")
		return false

func _try_get_node(path : NodePath, klass : String) -> Node:
	var node : Node = get_node(path)
	if node != null:
		if _check_class(node, klass, path):
			return node
	else:
		push_error("Failed to get node: ", path, " (returned null)")
	return null

func _ready():
	GameSingleton.province_selected.connect(_on_province_selected)
	GameSingleton.state_updated.connect(_update_info)

	add_child(GameSingleton.generate_gui("province_interface.gui", "province_view"))

	var close_button : Button = _try_get_node(^"./province_view/close_button", "Button")
	if close_button != null:
		close_button.pressed.connect(_on_close_button_pressed)

	_region_name_label = _try_get_node(^"./province_view/province_view_header/state_name", "Label")

	_province_name_label = _try_get_node(^"./province_view/province_view_header/province_name", "Label")

	_life_rating_bar = _try_get_node(^"./province_view/province_view_header/liferating", "TextureProgressBar")

	var goods_icon : TextureRect = _try_get_node(^"./province_view/province_statistics/goods_type", "TextureRect")
	if goods_icon != null:
		var texture := goods_icon.texture
		if _check_class(texture, "GFXIconTexture", "good_texture"):
			_rgo_icon_texture = texture


	var rgo_population_label : Label = _try_get_node(^"./province_view/province_statistics/rgo_population", "Label")
	if rgo_population_label != null:
		rgo_population_label.text = "0"

	#^"./province_view/province_statistics/crime_icon"

	_total_population_label = _try_get_node(^"./province_view/province_statistics/total_population", "Label")

	#^"./province_view/province_statistics/growth"
	#^"./province_view/province_statistics/workforce_chart"
	#^"./province_view/province_statistics/ideology_chart"
	#^"./province_view/province_statistics/culture_chart"

	$province_view/province_view_header/occupation_progress.visible = false
	$province_view/province_view_header/occupation_icon.visible = false
	$province_view/province_view_header/occupation_flag.visible = false
	$province_view/province_colony.visible = false
	$province_view/province_other.visible = false
	$province_view/province_buildings/army_size.visible = false
	$province_view/province_buildings/army_text.visible = false
	$province_view/province_buildings/navy_text.visible = false
	$province_view/national_focus_window.visible = false

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
func _update_info() -> void:
	_province_info = GameSingleton.get_province_info_from_index(_selected_index)
	if _province_info:
		if _province_name_label:
			_province_name_label.text = "PROV" + _province_info.get(_province_info_province_key,
				_province_info_province_key + _missing_suffix)
		if _region_name_label:
			_region_name_label.text = _province_info.get(_province_info_region_key,
				_province_info_region_key + _missing_suffix)
		if _life_rating_bar:
			_life_rating_bar.value = _province_info.get(_province_info_life_rating_key, 0) * 0

		if _total_population_label:
			_total_population_label.text = Localisation.tr_number(_province_info.get(_province_info_total_population_key, 0))

		#_pop_type_chart.set_to_distribution(_province_info.get(_province_info_pop_types_key, {}))
		#_pop_ideology_chart.set_to_distribution(_province_info.get(_province_info_pop_ideologies_key, {}))
		#_pop_culture_chart.set_to_distribution(_province_info.get(_province_info_pop_cultures_key, {}))

		if _rgo_icon_texture:
			_rgo_icon_texture.set_icon_index((_selected_index % 40) + 1)

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
