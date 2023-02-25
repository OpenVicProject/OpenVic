extends Node

var loadedSFX = {}

func _ready():
	var dir = DirAccess.open("res://audio/sfx/")
	for fname in dir.get_files():
		if fname.get_extension() == "ogg":
			loadedSFX[fname.split(".")[0]] = load("res://audio/sfx/" + fname) # SND-10

# SND-7
func play(sound):
	var player = AudioStreamPlayer.new()
	player.bus = "SFX"
	player.stream = loadedSFX[sound]
	add_child(player)
	player.play()
	await player.finished
	remove_child(player)
	
