extends SettingOptionButton

# REQUIREMENTS
# * UIFUN-21
# * UIFUN-28

@export
var default_value : Vector2i = Resolution.error_resolution

func _find_resolution_index_by_value(value : Vector2i) -> int:
	for item_index in item_count:
		if get_item_metadata(item_index) == value:
			return item_index
	return -1

func _sync_resolutions(to_select : Vector2i = Resolution.get_current_resolution()) -> void:
	clear()
	default_selected = -1
	selected = -1
	for resolution_value in Resolution.get_resolution_value_list():
		add_item(Resolution.get_resolution_display_name(resolution_value))
		set_item_metadata(item_count - 1, resolution_value)

		if resolution_value == default_value:
			default_selected = item_count - 1

		if resolution_value == to_select:
			selected = item_count - 1

	if default_selected == -1:
		default_selected = item_count - 1

	if selected == -1:
		selected = default_selected

func _setup_button():
	if default_value.x <= 0:
		default_value.x = ProjectSettings.get_setting("display/window/size/viewport_width")
	if default_value.y <= 0:
		default_value.y = ProjectSettings.get_setting("display/window/size/viewport_height")
	Resolution.add_resolution(default_value, &"default")
	_sync_resolutions()

func _get_value_for_file(select_value : int):
	if _valid_index(select_value):
		return get_item_metadata(select_value)
	else:
		return null

func _set_value_from_file(load_value):
	var target_resolution := Resolution.error_resolution
	match typeof(load_value):
		TYPE_VECTOR2I: target_resolution = load_value
		TYPE_STRING, TYPE_STRING_NAME: target_resolution = Resolution.get_resolution_value_from_string(load_value)
	if target_resolution != Resolution.error_resolution:
		selected = _find_resolution_index_by_value(target_resolution)
		if selected != -1: return
		if Resolution.add_resolution(target_resolution):
			_sync_resolutions(target_resolution)
			return
	push_error("Setting value '%s' invalid for setting [%s] %s" % [load_value, section_name, setting_name])
	selected = default_selected

func _on_item_selected(index : int):
	if _valid_index(index):
		Resolution.set_resolution(get_item_metadata(index))
	else:
		push_error("Invalid ResolutionSelector index: %d" % index)
		reset_setting()
