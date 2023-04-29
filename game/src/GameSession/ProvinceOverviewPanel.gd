extends PanelContainer

signal province_deselected

@export var _province_name_label : Label
@export var _region_name_label : Label
@export var _life_rating_bar : ProgressBar
@export var _buildings_container : Container

const _missing_suffix : String = "_MISSING"

var _selected_index : int:
	get: return _selected_index
	set(v):
		_selected_index = v
		update_info()
var _province_info : Dictionary

func _ready():
	GameSingleton.state_updated.connect(update_info)
	update_info()

enum { CANNOT_EXPAND, CAN_EXPAND, PREPARING, EXPANDING }

func _expand_building(building_identifier : String) -> void:
	if GameSingleton.expand_building(_selected_index, building_identifier) != OK:
		push_error("Failed to expand ", building_identifier, " in province #", _selected_index);

# REQUIREMENTS:
# * UI-183, UI-185, UI-186, UI-765, UI-187, UI-188, UI-189
# * UI-191, UI-193, UI-194, UI-766, UI-195, UI-196, UI-197
# * UI-199, UI-201, UI-202, UI-767, UI-203, UI-204, UI-205
func _add_building(building : Dictionary) -> void:
	const _building_key : StringName = &"building"
	const _level_key : StringName = &"level"
	const _expansion_state_key : StringName = &"expansion_state"
	const _start_key : StringName = &"start"
	const _end_key : StringName = &"end"
	const _expansion_progress_key : StringName = &"expansion_progress"

	const _expand_province_building : String = "EXPAND_PROVINCE_BUILDING"

	var level_label := Label.new()
	level_label.text = str(building.get(_level_key, 0))
	_buildings_container.add_child(level_label)

	var building_label := Label.new()
	building_label.text = building.get(_building_key, _building_key + _missing_suffix)
	_buildings_container.add_child(building_label)

	var expansion_state : int = building.get(_expansion_state_key, CANNOT_EXPAND)
	if expansion_state == PREPARING or expansion_state == EXPANDING:
		var progress_bar := ProgressBar.new()
		progress_bar.max_value = 1
		progress_bar.value = building.get(_expansion_progress_key, 0)
		progress_bar.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		_buildings_container.add_child(progress_bar)
	else:
		var expand_button := Button.new()
		expand_button.text = _expand_province_building
		expand_button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		expand_button.disabled = expansion_state != CAN_EXPAND
		expand_button.pressed.connect(func(): _expand_building(building_label.text))
		_buildings_container.add_child(expand_button)

func update_info() -> void:
	const _province_key : StringName = &"province"
	const _region_key : StringName = &"region"
	const _life_rating_key : StringName = &"life_rating"
	const _buildings_key : StringName = &"buildings"

	const _life_rating_tooltip : String = "LIFE_RATING_TOOLTIP"

	_province_info = GameSingleton.get_province_info_from_index(_selected_index)
	if _province_info:
		_province_name_label.text = _province_info.get(_province_key, _province_key + _missing_suffix)
		_region_name_label.text = _province_info.get(_region_key, _region_key + _missing_suffix)

		_life_rating_bar.value = _province_info.get(_life_rating_key, 0)
		_life_rating_bar.tooltip_text = tr(_life_rating_tooltip) % _life_rating_bar.value

		for child in _buildings_container.get_children():
			_buildings_container.remove_child(child)
			child.queue_free()
		var buildings : Array = _province_info.get(_buildings_key, [])
		for building in buildings:
			_add_building(building)

		show()
	else:
		hide()

func _on_province_selected(index : int) -> void:
	_selected_index = index

func _on_close_button_pressed() -> void:
	province_deselected.emit()
