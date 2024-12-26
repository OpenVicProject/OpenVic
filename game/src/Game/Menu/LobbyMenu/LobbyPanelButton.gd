@tool
class_name LobbyPanelButton
extends Container

signal button_down
signal button_up
signal pressed
signal toggled(button_pressed: bool)

var is_start_date: bool:
	get = _is_start_date


func _is_start_date() -> bool:
	return true


@export_group("Nodes")
@export var background_button: BaseButton
@export var name_label: Label

var text: StringName:
	get = get_text,
	set = set_text


func get_text() -> StringName:
	return name_label.text


func set_text(value: StringName) -> void:
	name_label.text = value


func _get_minimum_size() -> Vector2:
	var result := Vector2()
	for child: Control in get_children():
		if child == null or not child.visible:
			continue
		if child.top_level:
			continue

		var minsize: Vector2 = child.get_combined_minimum_size()
		result.x = max(result.x, minsize.x)
		result.y = max(result.y, minsize.y)

	var draw_style := _get_draw_mode_style()
	if draw_style != null:
		result += draw_style.get_minimum_size()

	return result


func _get_draw_mode_name(support_rtl: bool = true) -> StringName:
	var rtl := support_rtl and background_button != null and background_button.is_layout_rtl()
	match (
		background_button.get_draw_mode()
		if background_button != null
		else BaseButton.DrawMode.DRAW_NORMAL
	):
		BaseButton.DrawMode.DRAW_NORMAL:
			if rtl:
				return &"normal_mirrored"
			return &"normal"
		BaseButton.DrawMode.DRAW_PRESSED:
			if rtl:
				return &"pressed_mirrored"
			return &"pressed"
		BaseButton.DrawMode.DRAW_HOVER:
			if rtl:
				return &"hover_mirrored"
			return &"hover"
		BaseButton.DrawMode.DRAW_DISABLED:
			if rtl:
				return &"disabled_mirrored"
			return &"disabled"
		BaseButton.DrawMode.DRAW_HOVER_PRESSED:
			if rtl:
				return &"hover_pressed_mirrored"
			return &"hover_pressed"
	return &""


func _get_draw_mode_style() -> StyleBox:
	if background_button == null:
		return null
	var result := background_button.get_theme_stylebox(_get_draw_mode_name())
	if result == null:
		return background_button.get_theme_stylebox(_get_draw_mode_name(false))
	return result


func _notification(what: int) -> void:
	if what == NOTIFICATION_SORT_CHILDREN:
		var _size := size
		var offset := Vector2()
		var style := _get_draw_mode_style()
		if style != null:
			_size -= style.get_minimum_size()
			offset += style.get_offset()

		for child: Control in get_children():
			if child == null or not child.is_visible_in_tree() or child.top_level:
				continue

			fit_child_in_rect(child, Rect2(offset, _size))


func _on_background_button_button_down() -> void:
	button_down.emit()


func _on_background_button_button_up() -> void:
	button_up.emit()


func _on_background_button_pressed() -> void:
	pressed.emit()


func _on_background_button_toggled(button_pressed: bool) -> void:
	toggled.emit(button_pressed)
