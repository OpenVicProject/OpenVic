extends Node

var _loaded_sfx = {}

func _ready():
	var dir = DirAccess.open("res://audio/sfx/")
	for fname in dir.get_files():
		if fname.get_extension() == "ogg":
			_loaded_sfx[fname.get_basename()] = load("res://audio/sfx/" + fname) # SND-10

# SND-7
func play(sound):
	var player = AudioStreamPlayer.new()
	player.bus = "SFX"
	player.stream = _loaded_sfx[sound]
	add_child(player)
	player.play()
	await player.finished
	remove_child(player)
	
