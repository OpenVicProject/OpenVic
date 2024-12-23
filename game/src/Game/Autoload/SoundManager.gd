extends Node

# REQUIREMENTS:
# * SS-68

const _audio_directory_path: StringName = &"res://assets/audio/sfx/"

var _loaded_sound: Dictionary = {}

var _bus_to_stream_player: Dictionary = {}


# REQUIREMENTS:
# * SND-10
func _ready() -> void:
	var dir := DirAccess.open(_audio_directory_path)
	for fname: String in dir.get_files():
		match fname.get_extension():
			"ogg", "wav", "mp3":
				_loaded_sound[fname.get_basename()] = load(_audio_directory_path.path_join(fname))  # SND-10


func play_stream(sound: AudioStream, bus_type: String, volume: float = 1.0) -> void:
	var player: AudioStreamPlayer = _bus_to_stream_player.get(bus_type)
	if player == null:
		player = AudioStreamPlayer.new()
		player.bus = bus_type
		player.stream = AudioStreamPolyphonic.new()
		_bus_to_stream_player[bus_type] = player
		add_child(player)
		player.play()
	var poly_playback: AudioStreamPlaybackPolyphonic = player.get_stream_playback()
	player.volume_db = linear_to_db(volume)
	poly_playback.play_stream(sound)


func play(sound: String, bus_type: String) -> void:
	play_stream(_loaded_sound[sound], bus_type)


# REQUIREMENTS:
# * SND-7
func play_effect_stream(sound: AudioStream, volume: float = 1.0) -> void:
	play_stream(sound, "SFX", volume)


func play_effect(sound: String) -> void:
	play(sound, "SFX")


func play_effect_compat(sfx: String, fallback: AudioStream = null) -> void:
	var sound: AudioStreamWAV = SoundSingleton.get_sound_stream(sfx)
	var volume: float = SoundSingleton.get_sound_base_volume(sfx)

	if sound != null:
		play_effect_stream(sound, volume)
	elif fallback != null:
		push_warning("Failed to find sound %s, playing fallback instead" % sfx)
		play_effect_stream(fallback)
	else:
		push_warning("Failed to find sound %s" % sfx)
