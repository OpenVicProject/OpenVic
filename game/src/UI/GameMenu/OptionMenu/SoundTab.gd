extends HBoxContainer

@export var _startup_music_button : SettingCheckBox

func _ready() -> void:
	_startup_music_button.option_selected.connect(func (pressed : bool, _by_user : bool) -> void: MusicManager.set_startup_music(pressed))
