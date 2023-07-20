extends HBoxContainer

@export var _startup_music_button : SettingCheckBox

func _ready():
	_startup_music_button.option_selected.connect(func (pressed : bool, _by_user : bool): MusicConductor.set_startup_music(pressed))
