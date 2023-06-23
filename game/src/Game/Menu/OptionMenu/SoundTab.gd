extends HBoxContainer

@export var _startup_music_button : Button

func _ready():
	_startup_music_button.option_selected.connect(func (pressed : bool, by_user : bool): MusicConductor.set_startup_music(pressed))
