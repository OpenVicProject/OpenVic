extends Node

var selectedTrack = 0
var availableSongs : Array[SongInfo] = []
var autoPlayNextSong : bool = true

func getCurrentSongName() -> String:
	return availableSongs[selectedTrack].readableName

func getCurrentSongTime() -> float:
	return $AudioStreamPlayer.get_playback_position()

func getCurrentSongLength() -> float:
	return $AudioStreamPlayer.stream.get_length()

func getCurrentSongProgressPercentage() -> float:
	return 100 * (getCurrentSongTime() / getCurrentSongLength())

func isPaused() -> bool:
	return $AudioStreamPlayer.stream_paused

func togglePlayPause() -> void:
	$AudioStreamPlayer.stream_paused = !$AudioStreamPlayer.stream_paused
	
func startCurrentSong() -> void:
	$AudioStreamPlayer.stream = availableSongs[selectedTrack].loadedStream
	$AudioStreamPlayer.play()

func nextSong() -> void:
	selectedTrack = (selectedTrack + 1) % len(availableSongs)
	startCurrentSong()

func prevSong() -> void:
	selectedTrack = (len(availableSongs) - 1) if (selectedTrack == 0) else (selectedTrack - 1)
	startCurrentSong()

# Called when the node enters the scene tree for the first time.
func _ready():
	var dir = DirAccess.open("res://audio/music/")
	for fname in dir.get_files():
		if fname.ends_with(".mp3"):
			if fname == "The_Crown.mp3":
				selectedTrack = availableSongs.size()
			availableSongs.append(SongInfo.new("res://audio/music/", fname))
	startCurrentSong()


func _on_audio_stream_player_finished():
	if autoPlayNextSong:
		nextSong()
		startCurrentSong()
