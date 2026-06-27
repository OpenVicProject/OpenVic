class_name SettingsContainer
extends ScrollContainer

@export var _container: GridContainer

var section_key: StringName
var _revert_group_to_dialog: Dictionary[GameSettings.RevertGroup, RevertDialog] = {}


func _ready() -> void:
	GameSettings.changed.connect(_try_update)
	GameSettings.staged_changed.connect(_try_update)
	GameSettings.applied.connect(_on_setting_applied)


func add_section(section: String) -> void:
	if _container.get_child_count() > 0:
		var spacer: Control = Control.new()
		spacer.custom_minimum_size = Vector2(0, 32)
		_container.add_child(spacer)
		_container.add_child(Control.new())
		_container.add_child(Control.new())

	var label: Label = Label.new()
	_container.add_child(label)
	label.text = section
	label.theme_type_variation = &"HeaderLarge"
	_container.add_child(Control.new())
	_container.add_child(Control.new())


func add_setting(setting: GameSettings.Setting) -> void:
	_add_label(setting)
	_add_edit(setting)
	_add_reset_button(setting)


func _add_label(setting: GameSettings.Setting) -> void:
	var label := SettingLabel.new(setting)
	_container.add_child(label)


func _add_edit(setting: GameSettings.Setting) -> void:
	var typ: Variant.Type = setting.get_meta(&"type", TYPE_NIL)
	var type_hint: PropertyHint = setting.get_meta(&"hint", PROPERTY_HINT_NONE)

	var control: Control
	if typ == TYPE_BOOL and type_hint == PROPERTY_HINT_ENUM:
		control = BoolEnumButton.new(setting)
	elif typ == TYPE_BOOL:
		control = BoolButton.new(setting)
	elif typ != TYPE_NIL and type_hint == PROPERTY_HINT_ENUM:
		control = EnumOptionButton.create_button(setting)
	elif (typ == TYPE_INT or typ == TYPE_FLOAT) and type_hint == PROPERTY_HINT_RANGE:
		control = RangeSlider.new(setting)
	elif type_hint == PROPERTY_HINT_DIR\
		or type_hint == PROPERTY_HINT_FILE\
		or type_hint == PROPERTY_HINT_FILE_PATH:
		control = FileSelectEdit.new(setting)
	else:
		control = PlaceholderLabel.new(setting)

	if setting.has_meta(&"revert_group"):
		_add_revert_dialog(setting)

	control.name = setting.key().validate_node_name()
	_container.add_child(control)


func _add_reset_button(setting: GameSettings.Setting) -> void:
	# hide reset button if setting is marked having no default value
	if setting.get_meta(&"no_default", false):
		_container.add_child(Control.new())
		return
	var reset_button: ResetButton = ResetButton.new(setting)
	_container.add_child(reset_button)


func _add_revert_dialog(setting: GameSettings.Setting) -> void:
	var revert_group: GameSettings.RevertGroup = setting.get_meta(&"revert_group")
	var revert_dialog: RevertDialog = _revert_group_to_dialog.get(revert_group)
	if revert_dialog == null:
		revert_dialog = RevertDialog.new(revert_group)
		_revert_group_to_dialog[revert_group] = revert_dialog
		add_child(revert_dialog)
	setting.set_meta(&"revert_value", setting.value())


func _try_update(key: StringName) -> void:
	if not key.begins_with(section_key + "/"): return
	var child := _container.get_node(key.validate_node_name())
	var child_key: StringName = child.get_meta(&"setting_key", "")
	if child_key != key: return
	child.update()
	var button := (_container.get_child(child.get_index() + 1) as ResetButton)
	if button: button.update()


func _on_setting_applied(key: StringName) -> void:
	if not key.begins_with(section_key + "/"): return
	var stg := GameSettings.get_setting(key)
	if not stg.has_meta(&"revert_value"): return

	var previous_value: Variant = stg.get_meta(&"revert_value")
	var revert_group: GameSettings.RevertGroup = stg.get_meta(&"revert_group")
	var revert_dialog: RevertDialog = _revert_group_to_dialog[revert_group]
	var callable := _on_revert_dialog_revert.bind(stg, previous_value)
	revert_dialog.reverted.connect(callable)
	stg.set_meta(&"revert_value", stg.value())

	revert_dialog.show_dialog()
	GameSettings.applied.disconnect(_on_setting_applied)
	revert_dialog.visibility_changed.connect(func() -> void:
		revert_dialog.reverted.disconnect(callable)
		GameSettings.applied.connect(_on_setting_applied),
	CONNECT_ONE_SHOT)


func _on_revert_dialog_revert(stg: GameSettings.Setting, prev_value: Variant) -> void:
	stg.set_value(prev_value)
	stg.set_meta(&"revert_value", prev_value)


class SettingLabel extends Label:
	var setting: GameSettings.Setting

	func _init(stg: GameSettings.Setting) -> void:
		setting = stg
		text = setting.get_meta(
			&"display_name",
			"OPTIONS_" + setting.key().replace("/", "_").to_upper())
		tooltip_text = setting.description()
		custom_minimum_size = Vector2(145, 0)
		autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		mouse_filter = Control.MOUSE_FILTER_PASS
		set_meta(&"setting_key", setting.key())


class BoolEnumButton extends OptionButton:
	var setting: GameSettings.Setting

	func _init(stg: GameSettings.Setting) -> void:
		setting = stg
		set_meta(&"setting_key", setting.key())
		size_flags_horizontal = Control.SIZE_EXPAND_FILL
		add_item("On")
		add_item("Off")
		update()
		item_selected.connect(func(idx: int) -> void: setting.set_value(idx == 0))

	func update() -> void:
		disabled = setting.is_readonly()
		if setting.staged_or_value():
			select(0)
		else:
			select(1)


class BoolButton extends CheckButton:
	var setting: GameSettings.Setting

	func _init(stg: GameSettings.Setting) -> void:
		setting = stg
		set_meta(&"setting_key", setting.key())
		size_flags_horizontal = Control.SIZE_EXPAND_FILL
		update()
		toggled.connect(func(toggled_on: bool) -> void: setting.set_value(toggled_on))

	func update() -> void:
		disabled = setting.is_readonly()
		button_pressed = setting.staged_or_value()


class EnumOptionButton extends OptionButton:
	var setting: GameSettings.Setting


	func _init(stg: GameSettings.Setting) -> void:
		setting = stg
		var values: Array[Variant] = setting.get_meta(&"values", [])
		var display_values: Array[String] = []
		display_values.assign(setting.get_meta(&"display_values", []))
		var translate_function: Callable = setting.get_meta(
			&"translate_value_function",
			func(_s, _v, display_value: String) -> String: return display_value)

		set_meta(&"setting_key", setting.key())
		size_flags_horizontal = Control.SIZE_EXPAND_FILL
		var sel_idx: int = 0
		for idx: int in range(len(values)):
			var translated_display: String = translate_function.call(
				setting,
				values[idx],
				display_values[idx],
			)
			add_item(translated_display)
			if setting.staged_or_value() == values[idx]:
				sel_idx = idx
		update(sel_idx)
		item_selected.connect(func(idx: int) -> void:
			setting.set_value(setting.get_meta(&"values")[idx])
		)


	static func create_button(stg: GameSettings.Setting) -> Control:
		var values: Array[Variant] = stg.get_meta(&"values", [])
		var display_values: Array[String] = []
		display_values.assign(stg.get_meta(&"display_values", []))
		if values.size() != display_values.size():
			push_error("Setting %s has mismatched values and display_values size.".format(stg.key()))
			return PlaceholderLabel.new(stg)
		return EnumOptionButton.new(stg)


	func update(index: int = -1) -> void:
		disabled = setting.is_readonly()
		if index == -1:
			var values: Array[Variant] = setting.get_meta(&"values", [])
			select(values.find(setting.staged_or_value()))
		else:
			select(index)


class RangeSlider extends HBoxContainer:
	var setting: GameSettings.Setting
	var _slider := HSlider.new()
	var _spinbox := SpinBox.new()

	func _init(stg: GameSettings.Setting) -> void:
		setting = stg
		set_meta(&"setting_key", setting.key())
		size_flags_horizontal = Control.SIZE_EXPAND_FILL
		add_child(_slider)
		_spinbox.share(_slider)
		_slider.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		_slider.size_flags_vertical = Control.SIZE_EXPAND_FILL
		_slider.min_value = setting.get_meta(&"min", 0)
		_slider.max_value = setting.get_meta(&"max", 120)
		_slider.step = setting.get_meta(&"step", 1)
		update()
		_slider.value_changed.connect(func(val: float) -> void:
			setting.set_value(val)
		)
		add_child(_spinbox)

	func update() -> void:
		_slider.editable = not setting.is_readonly()
		_slider.value = setting.staged_or_value()


class FileSelectEdit extends HBoxContainer:
	var setting: GameSettings.Setting
	var _line_edit := LineEdit.new()
	var _dialog_button := Button.new()
	var _dialog := FileDialog.new()

	func _init(stg: GameSettings.Setting) -> void:
		setting = stg
		set_meta(&"setting_key", setting.key())
		size_flags_horizontal = Control.SIZE_EXPAND_FILL
		add_child(_line_edit)
		_line_edit.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		_line_edit.size_flags_vertical = Control.SIZE_EXPAND_FILL

		add_child(_dialog_button)
		_dialog_button.text = "🗂️"
		_dialog_button.tooltip_text = "Open file dialog"

		add_child(_dialog)
		var type_hint: PropertyHint = setting.get_meta(&"hint")
		var use_file_mode := type_hint == PROPERTY_HINT_FILE or type_hint == PROPERTY_HINT_GLOBAL_FILE
		_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE if use_file_mode else FileDialog.FILE_MODE_OPEN_DIR
		var is_global := type_hint == PROPERTY_HINT_GLOBAL_FILE or type_hint == PROPERTY_HINT_GLOBAL_DIR
		_dialog.access = FileDialog.ACCESS_FILESYSTEM if is_global else FileDialog.ACCESS_USERDATA
		_dialog.show_hidden_files = true
		_dialog.disable_3d = true

		update()
		_line_edit.text_submitted.connect(_on_path_selected)
		_dialog_button.pressed.connect(_dialog.popup_file_dialog)
		if use_file_mode:
			_dialog.file_selected.connect(_on_path_selected)
		else:
			_dialog.dir_selected.connect(_on_path_selected)

	func update() -> void:
		_line_edit.editable = not setting.is_readonly()
		_line_edit.placeholder_text = setting.staged_or_value()

	func _on_path_selected(path: String) -> void:
		setting.set_value(path)
		update()


class PlaceholderLabel extends Label:
	var setting: GameSettings.Setting

	func _init(stg: GameSettings.Setting) -> void:
		setting = stg
		set_meta(&"setting_key", setting.key())
		size_flags_horizontal = Control.SIZE_EXPAND_FILL
		text = str(setting.staged_or_value())

	func update() -> void:
		pass


class ResetButton extends Button:
	var setting: GameSettings.Setting

	func _init(stg: GameSettings.Setting) -> void:
		setting = stg
		set_meta(&"setting_key", setting.key())
		tooltip_text = "Reset to default"
		text = "↩"
		update()
		pressed.connect(func() -> void: setting.reset())

	func _ready() -> void:
		add_theme_font_size_override(&"font_size", 20)

	func update() -> void:
		disabled = setting.staged_or_value() == setting.default_value()


class RevertDialog extends ConfirmationDialog:
	signal accepted()


	signal reverted()


	var _timer := Timer.new()


	var _revert_group: GameSettings.RevertGroup


	func _init(revert_group: GameSettings.RevertGroup) -> void:
		_revert_group = revert_group
		title = _revert_group.title
		cancel_button_text = "DIALOG_CANCEL"
		ok_button_text = "DIALOG_OK"
		disable_3d = true
		size = Vector2i(730, 100)
		confirmed.connect(_on_confirmed)
		canceled.connect(_on_canceled)
		close_requested.connect(_on_canceled)

		add_child(_timer)
		_timer.wait_time = 5
		_timer.one_shot = true
		_timer.timeout.connect(_on_timer_timeout)


	func _process(_delta: float) -> void:
		dialog_text = tr(_revert_group.text).format({ "time": Localisation.tr_number(int(_timer.time_left)) })


	func _notification(what: int) -> void:
		match what:
			NOTIFICATION_VISIBILITY_CHANGED:
				set_process(visible)
			NOTIFICATION_WM_CLOSE_REQUEST, NOTIFICATION_CRASH:
				_on_canceled()


	func show_dialog() -> void:
		_timer.start()
		popup_centered(Vector2(1, 1))


	func _on_confirmed() -> void:
		_timer.stop()
		accepted.emit()


	func _on_canceled() -> void:
		_timer.stop()
		reverted.emit()


	func _on_timer_timeout() -> void:
		hide()
		reverted.emit()
