extends SettingOptionButton

@export
var default_value : Vector2i = Vector2i(-1, -1)

func add_resolution(value : Vector2i, selection_name : String = "") -> void:
	if selection_name.is_empty():
		selection_name = "%sx%s" % [value.x, value.y]
	add_item(selection_name)
	set_item_metadata(item_count - 1, value)

func find_resolution_value(value : Vector2i) -> int:
	for item_index in range(item_count):
		if get_item_metadata(item_index) == value:
			return item_index
	return -1

func _setup_button():
	if default_value.x < 0:
		default_value.x = ProjectSettings.get_setting("display/window/size/viewport_width")

	if default_value.y < 0:
		default_value.y = ProjectSettings.get_setting("display/window/size/viewport_height")

	clear()
	default_selected = -1
	selected = -1
	for resolution in Resolution.get_resolution_name_list():
		var resolution_value := Resolution.get_resolution(resolution)
		add_resolution(resolution_value, resolution)

		if resolution_value == default_value:
			default_selected = item_count - 1

		if resolution_value == Resolution.get_current_resolution():
			selected = item_count - 1

	if default_selected == -1:
		add_resolution(default_value)
		default_selected = item_count - 1

	if selected == -1:
		selected = default_selected

func _get_value_for_file(select_value : int):
	return get_item_metadata(select_value)

func _set_value_from_file(load_value):
	var resolution_value := load_value as Vector2i
	selected = find_resolution_value(resolution_value)
	if selected == -1:
		if add_nonstandard_value:
			add_resolution(resolution_value)
			selected = item_count - 1
		else: push_error("Setting value '%s' invalid for setting [%s] %s" % [load_value, section_name, setting_name])

func _on_item_selected(index):
	Resolution.set_resolution(get_item_metadata(index))
