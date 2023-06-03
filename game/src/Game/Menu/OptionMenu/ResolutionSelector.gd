extends SettingRevertButton

# REQUIREMENTS
# * UIFUN-21
# * UIFUN-28
# * UIFUN-301
# * UIFUN-302

@export var default_value : Vector2i = Resolution.error_resolution

func _find_resolution_index_by_value(value : Vector2i) -> int:
	for item_index in item_count:
		if get_item_metadata(item_index) == value:
			return item_index
	return -1

func _sync_resolutions(
	value : Vector2i = Resolution.error_resolution,
	_resolution_name = null,
	_resolution_display_name = null
) -> void:
	clear()
	default_selected = -1
	selected = -1
	var resolution_list := Resolution.get_resolution_value_list()
	if value != Resolution.error_resolution:
		resolution_list.append(value)
	for resolution_value in resolution_list:
		var display_name := "%sx%s" % [resolution_value.x, resolution_value.y]
		var resolution_name := Resolution.get_resolution_name(resolution_value)
		if resolution_name == &"Default":
			display_name = "Default (%s)" % resolution_name
		if not resolution_name.is_empty():
			display_name = "%s (%s)" % [display_name, resolution_name + (", Default" if resolution_value == default_value else "")]
		add_item(display_name)
		set_item_metadata(item_count - 1, resolution_value)

		if resolution_value == default_value:
			default_selected = item_count - 1

		if resolution_value == Resolution.get_current_resolution():
			selected = item_count - 1

	if default_selected == -1:
		default_selected = item_count - 1

	if selected == -1:
		selected = default_selected

func _setup_button() -> void:
	Resolution.resolution_added.connect(_sync_resolutions)
	if default_value.x <= 0:
		default_value.x = ProjectSettings.get_setting("display/window/size/viewport_width")
	if default_value.y <= 0:
		default_value.y = ProjectSettings.get_setting("display/window/size/viewport_height")
	if not Resolution.has_resolution(default_value):
		Resolution.add_resolution(default_value, &"Default")
	else:
		_sync_resolutions()

func _get_value_for_file(select_value : int) -> Variant:
	if _valid_index(select_value):
		return get_item_metadata(select_value)
	else:
		return null

func _set_value_from_file(load_value) -> void:
	var target_resolution := Resolution.error_resolution
	match typeof(load_value):
		TYPE_VECTOR2I: target_resolution = load_value
		TYPE_STRING, TYPE_STRING_NAME: target_resolution = Resolution.get_resolution_value_from_string(load_value)
	if target_resolution != Resolution.error_resolution:
		selected = _find_resolution_index_by_value(target_resolution)
		if selected != -1: return
		if Resolution.add_resolution(target_resolution):
			Resolution.set_resolution(target_resolution)
			return
	push_error("Setting value '%s' invalid for setting [%s] %s" % [load_value, section_name, setting_name])
	selected = default_selected

func _on_option_selected(index : int, by_user : bool) -> void:
	if _valid_index(index):
		if by_user:
			print("Start Revert Countdown!")
			revert_dialog.show_dialog.call_deferred(self)
			previous_index = _find_resolution_index_by_value(Resolution.get_current_resolution())

		Resolution.set_resolution(get_item_metadata(index))
	else:
		push_error("Invalid ResolutionSelector index: %d" % index)
		reset_setting(not by_user)
