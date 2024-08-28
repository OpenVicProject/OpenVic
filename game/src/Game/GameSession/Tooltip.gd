extends GUINode

var _tooltip_label : GUILabel

func _ready() -> void:
	add_gui_element("core", "ToolTip")

	_tooltip_label = get_gui_label_from_nodepath(^"./ToolTip")
	if _tooltip_label:
		_tooltip_label.set_auto_adjust_to_content_size(true)

		MenuSingleton.update_tooltip.connect(update_tooltip)

	hide()

func update_tooltip(text : String, substitution_dict : Dictionary, position : Vector2) -> void:
	if text:
		_tooltip_label.set_text(text)
		_tooltip_label.set_substitution_dict(substitution_dict)
		_tooltip_label.set_position(position)
		show()
	else:
		hide()
