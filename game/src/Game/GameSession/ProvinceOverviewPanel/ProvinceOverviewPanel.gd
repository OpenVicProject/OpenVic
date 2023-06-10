extends PanelContainer

@export var _province_name_label : Label
@export var _region_name_label : Label
@export var _life_rating_bar : ProgressBar
@export var _rgo_icon_texture_rect : TextureRect
@export var _rgo_name_label : Label
@export var _buildings_container : Container

const _missing_suffix : String = "_MISSING"

var _selected_index : int:
	get: return _selected_index
	set(v):
		_selected_index = v
		_update_info()
var _province_info : Dictionary

func _ready():
	GameSingleton.province_selected.connect(_on_province_selected)
	GameSingleton.state_updated.connect(_update_info)
	_update_info()

func _notification(what : int):
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()

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
	var row : Dictionary

	row.level = Label.new()
	row.level.text = "X"
	_buildings_container.add_child(row.level)

	row.name = Label.new()
	row.name.text = GameSingleton.get_building_info_building_key() + _missing_suffix
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
	row.level.text = str(building.get(GameSingleton.get_building_info_level_key(), 0))
	row.level.visible = true
	row.name.text = building.get(GameSingleton.get_building_info_building_key(),
		GameSingleton.get_building_info_building_key() + _missing_suffix)
	row.name.visible = true

	var expansion_state : int = building.get(GameSingleton.get_building_info_expansion_state_key(),
		CANNOT_EXPAND)
	var show_progress_bar := expansion_state == PREPARING or expansion_state == EXPANDING
	row.progress_bar.value = building.get(GameSingleton.get_building_info_expansion_progress_key(), 0)
	row.progress_bar.visible = show_progress_bar
	row.button.disabled = expansion_state != CAN_EXPAND
	row.button.visible = not show_progress_bar

func _update_info() -> void:
	_province_info = GameSingleton.get_province_info_from_index(_selected_index)
	if _province_info:
		_province_name_label.text = _province_info.get(GameSingleton.get_province_info_province_key(),
			GameSingleton.get_province_info_province_key() + _missing_suffix)
		_region_name_label.text = _province_info.get(GameSingleton.get_province_info_region_key(),
			GameSingleton.get_province_info_region_key() + _missing_suffix)

		_life_rating_bar.value = _province_info.get(GameSingleton.get_province_info_life_rating_key(), 0)
		_life_rating_bar.tooltip_text = tr("LIFE_RATING_TOOLTIP").format({
			"life_rating": Events.Localisation.tr_number(_life_rating_bar.value)
		})

		_rgo_name_label.text = _province_info.get(GameSingleton.get_province_info_rgo_key(),
			GameSingleton.get_province_info_rgo_key() + _missing_suffix)
		_rgo_icon_texture_rect.texture = GameSingleton.get_good_icon_texture(_rgo_name_label.text)

		var buildings : Array = _province_info.get(GameSingleton.get_province_info_buildings_key(), [])
		for i in max(buildings.size(), _building_rows.size()):
			_set_building_row(i, buildings[i] if i < buildings.size() else {})

		show()
	else:
		hide()
		mouse_exited.emit()

func _on_province_selected(index : int) -> void:
	_selected_index = index

func _on_close_button_pressed() -> void:
	GameSingleton.set_selected_province(0)
