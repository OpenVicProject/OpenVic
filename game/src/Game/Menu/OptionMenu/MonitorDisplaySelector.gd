extends SettingRevertButton

func _setup_button() -> void:
	clear()
	for screen_index in DisplayServer.get_screen_count():
		# Placeholder option text awaiting _update_monitor_options_text()
		add_item(str(screen_index + 1))
	_update_monitor_options_text()
	default_selected = Resolution.get_current_monitor()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_monitor_options_text()

func _update_monitor_options_text() -> void:
	for index in get_item_count():
		set_item_text(index, tr("OPTIONS_VIDEO_MONITOR").format({ "index": Localisation.tr_number(index + 1) }))

func _on_option_selected(index : int, by_user : bool) -> void:
	if _valid_index(index):
		if by_user:
			print("Start Revert Countdown!")
			revert_dialog.show_dialog.call_deferred(self)
			previous_index = Resolution.get_current_monitor()
		Resolution.set_monitor(index)
	else:
		push_error("Invalid MonitorDisplaySelector index: %d" % index)
		reset_setting(not by_user)
