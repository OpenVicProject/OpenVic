extends SettingOptionButton
class_name SettingRevertButton

@export_group("Nodes")
@export var revert_dialog : SettingRevertDialog

var previous_index : int = -1

func _ready() -> void:
	super()
	if revert_dialog != null:
		revert_dialog.visibility_changed.connect(_on_revert_dialog_visibility_changed)
		revert_dialog.dialog_accepted.connect(_on_accepted)
		revert_dialog.dialog_reverted.connect(_on_reverted)

func _on_revert_dialog_visibility_changed() -> void:
	disabled = revert_dialog.visible
	if not revert_dialog.visible:
		previous_index = -1

func _on_reverted(button : SettingRevertButton) -> void:
	if button != self: return
	selected = previous_index
	option_selected.emit(selected, false)

func _on_accepted(button : SettingRevertButton) -> void:
	if button != self: return
