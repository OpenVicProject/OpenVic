extends SettingOptionButton

# REQUIREMENTS
# * UIFUN-24
# * UIFUN-31

@export
var default_value : float = GuiScale.error_guiscale

func _find_guiscale_index_by_value(value : float) -> int:
	for item_index in item_count:
		if get_item_metadata(item_index) == value:
			return item_index
	return -1

func _sync_guiscales(to_select : float = GuiScale.get_current_guiscale()) -> void:
	clear()
	default_selected = -1
	selected = -1
	for guiscale_value : float in GuiScale.get_guiscale_value_list():
		add_item(GuiScale.get_guiscale_display_name(guiscale_value))
		set_item_metadata(item_count - 1, guiscale_value)

		if guiscale_value == default_value:
			default_selected = item_count - 1

		if guiscale_value == to_select:
			selected = item_count - 1

	if default_selected == -1:
		default_selected = item_count - 1

	if selected == -1:
		selected = default_selected

func _setup_button() -> void:
	if default_value <= 0:
		default_value = ProjectSettings.get_setting("display/window/stretch/scale")
	GuiScale.add_guiscale(default_value, &"default")
	_sync_guiscales()

func _get_value_for_file(select_value : int):
	if _valid_index(select_value):
		return get_item_metadata(select_value)
	else:
		return null

func _set_value_from_file(load_value : Variant) -> void:
	if typeof(load_value) == TYPE_FLOAT:
		var target_guiscale : float = load_value
		selected = _find_guiscale_index_by_value(target_guiscale)
		if selected != -1: return
		if GuiScale.add_guiscale(target_guiscale):
			_sync_guiscales(target_guiscale)
			return
	push_error("Setting value '%s' invalid for setting [%s] %s" % [load_value, section_name, setting_name])
	selected = default_selected

func _on_option_selected(index : int, by_user : bool) -> void:
	if _valid_index(index):
		GuiScale.set_guiscale(get_item_metadata(index))
	else:
		push_error("Invalid GuiScaleSelector index: %d" % index)
		reset_setting(not by_user)
