extends ConfirmationDialog
class_name SettingRevertDialog

signal dialog_accepted(button : SettingRevertButton)
signal dialog_reverted(button : SettingRevertButton)

@export var dialog_text_key : String = "< reverting in {time} seconds >"

@export_group("Nodes")
@export var timer : Timer

var _revert_node : SettingRevertButton = null

func show_dialog(button : SettingRevertButton, time : float = 0) -> void:
	timer.start(time)
	popup_centered(Vector2(1,1))
	_revert_node = button

func _notification(what : int) -> void:
	if what == NOTIFICATION_VISIBILITY_CHANGED:
		set_process(visible)
		if not visible: _revert_node = null

func _process(_delta : float) -> void:
	dialog_text = tr(dialog_text_key).format({ "time": Localisation.tr_number(int(timer.time_left)) })

func _on_canceled_or_close_requested() -> void:
	timer.stop()
	dialog_reverted.emit(_revert_node)

func _on_confirmed() -> void:
	timer.stop()
	dialog_accepted.emit(_revert_node)

func _on_resolution_revert_timer_timeout() -> void:
	dialog_reverted.emit(_revert_node)
	hide()
