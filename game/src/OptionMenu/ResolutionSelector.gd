extends SettingOptionButton

func _ready():
	print("Resolution selector ready")

	clear()
	var resolution_index := 0
	for resolution in Resolution.get_resolution_name_list():
		add_item(resolution)

		if Vector2(Resolution.get_resolution(resolution)) == Resolution.get_current_resolution():
			if default_value == -1:
				default_value = resolution_index
			_select_int(resolution_index)
			print(resolution)

		resolution_index += 1


func _on_item_selected(index):
	print("Selected index: %d" % index)

	var resolution_size : Vector2i = Resolution.get_resolution(get_item_text(index))
	Resolution.set_resolution(resolution_size)
