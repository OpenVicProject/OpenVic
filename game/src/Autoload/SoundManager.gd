extends Node

# REQUIREMENTS:
# * SS-68

const _audio_directory_path : StringName = &"res://audio/sfx/"

var _loaded_sound : Dictionary = {}

var _bus_to_stream_player : Dictionary = {}

# REQUIREMENTS:
# * SND-10
func _ready():
	var dir = DirAccess.open(_audio_directory_path)
	for fname in dir.get_files():
		match fname.get_extension():
			"ogg", "wav", "mp3":
				_loaded_sound[fname.get_basename()] = load(_audio_directory_path.path_join(fname)) # SND-10

func play_stream(sound : AudioStream, bus_type : String) -> void:
	var player : AudioStreamPlayer = _bus_to_stream_player.get(bus_type)
	if player == null:
		player = AudioStreamPlayer.new()
		player.bus = bus_type
		player.stream = AudioStreamPolyphonic.new()
		_bus_to_stream_player[bus_type] = player
		add_child(player)
		player.play()
	var poly_playback : AudioStreamPlaybackPolyphonic = player.get_stream_playback()
	poly_playback.play_stream(sound)

func play(sound : String, bus_type : String) -> void:
	play_stream(_loaded_sound[sound], bus_type)

# REQUIREMENTS:
# * SND-7
func play_effect_stream(sound : AudioStream) -> void:
	play_stream(sound, "SFX")

func play_effect(sound : String) -> void:
	play(sound, "SFX")
