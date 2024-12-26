extends GUINode

var _tooltip_label: GUILabel


func _ready() -> void:
	add_gui_element("core", "ToolTip")

	_tooltip_label = get_gui_label_from_nodepath(^"./ToolTip")
	if _tooltip_label:
		_update_tooltip_max_size()
		_tooltip_label.set_auto_adjust_to_content_size(true)

		MenuSingleton.update_tooltip.connect(update_tooltip)

	hide()


func _notification(what: int) -> void:
	match what:
		NOTIFICATION_RESIZED:
			_update_tooltip_max_size()


func _update_tooltip_max_size() -> void:
	if _tooltip_label:
		var max_size: Vector2 = _tooltip_label.get_base_max_size()
		var window_size: Vector2 = get_size()
		_tooltip_label.set_max_size(Vector2(min(max_size.x, window_size.x), window_size.y))


func update_tooltip(text: String, substitution_dict: Dictionary, position: Vector2) -> void:
	if text and _tooltip_label:
		_tooltip_label.set_text(text)
		_tooltip_label.set_substitution_dict(substitution_dict)
		_tooltip_label.force_update_lines()

		var adjusted_rect: Rect2 = _tooltip_label.get_adjusted_rect()

		# Shift position so that the tooltip doesn't go past the bottom or right sides of the window
		var bottom_right: Vector2 = (
			position + adjusted_rect.position + adjusted_rect.size - get_size()
		)
		if bottom_right.x > 0:
			position.x -= bottom_right.x
		if bottom_right.y > 0:
			position.y -= bottom_right.y

		# Shift position so that the tooltip doesn't go past the top or left sides of the window
		var top_left: Vector2 = position + adjusted_rect.position
		if top_left.x < 0:
			position.x -= top_left.x
		if top_left.y < 0:
			position.y -= top_left.y

		_tooltip_label.set_position(position)

		show()
	else:
		hide()
