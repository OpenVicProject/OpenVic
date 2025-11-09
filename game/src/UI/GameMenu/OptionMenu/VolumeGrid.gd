extends GridContainer

const RATIO_FOR_LINEAR : float = 100

var _slider_dictionary : Dictionary

var initial_focus : Control

func get_db_as_volume_value(db : float) -> float:
	# db_to_linear produces a float between 0 and 1 from a db value
	return db_to_linear(db) * RATIO_FOR_LINEAR

func get_volume_value_as_db(value : float) -> float:
	# linear_to_db consumes a float between 0 and 1 to produce the db value
	return linear_to_db(value / RATIO_FOR_LINEAR)

func add_volume_row(bus_name : String, bus_index : int) -> HSlider:
	var volume_label := Label.new()
	if bus_name == &"Master":
		volume_label.text = "MASTER_BUS"
	else:
		volume_label.text = bus_name
	add_child(volume_label)

	var volume_slider := SettingHSlider.new()
	volume_slider.section_name = "audio"
	volume_slider.setting_name = volume_label.text
	volume_slider.custom_minimum_size = Vector2(290, 0)
	volume_slider.size_flags_vertical = Control.SIZE_FILL
	volume_slider.min_value = 0
	volume_slider.default_value = 100
	volume_slider.max_value = 120 # 120 so volume can be boosted somewhat
	volume_slider.value_changed.connect(_on_slider_value_changed.bind(bus_index))
	add_child(volume_slider)

	_slider_dictionary[volume_label.text] = volume_slider
	if not initial_focus: initial_focus = volume_slider
	return volume_slider

# REQUIREMENTS
# * UI-22
func _enter_tree() -> void:
	for bus_index : int in AudioServer.bus_count:
		add_volume_row(AudioServer.get_bus_name(bus_index), bus_index)

func _notification(what : int) -> void:
	match(what):
		NOTIFICATION_VISIBILITY_CHANGED:
			if visible and is_inside_tree() and initial_focus: initial_focus.grab_focus()

# REQUIREMENTS
# * UIFUN-30
func _on_slider_value_changed(value : float, bus_index : int) -> void:
	AudioServer.set_bus_volume_db(bus_index, get_volume_value_as_db(value))
