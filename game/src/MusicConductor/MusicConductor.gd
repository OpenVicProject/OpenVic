extends Node

# SS-67
@export_dir var musicDir : String
@export_file var firstSongName : String

var selectedTrack = 0
var availableSongs : Array[SongInfo] = []
var autoPlayNextSong : bool = true

func getAllSongNames() -> Array[String]:
	var songNames : Array[String] = []
	for si in availableSongs:
		songNames.append(si.readableName)
	return songNames

func getCurrentSongIndex() -> int:
	return selectedTrack

func getCurrentSongName() -> String:
	return availableSongs[selectedTrack].readableName

func scrubSongByPercentage(percentage: float) -> void:
	var percentInSeconds : float = (percentage / 100.0) * $AudioStreamPlayer.stream.get_length()
	$AudioStreamPlayer.play(percentInSeconds)

func getCurrentSongProgressPercentage() -> float:
	return 100 * ($AudioStreamPlayer.get_playback_position() / $AudioStreamPlayer.stream.get_length())

func isPaused() -> bool:
	return $AudioStreamPlayer.stream_paused

func togglePlayPause() -> void:
	$AudioStreamPlayer.stream_paused = !$AudioStreamPlayer.stream_paused
	
func startCurrentSong() -> void:
	$AudioStreamPlayer.stream = availableSongs[selectedTrack].loadedStream
	$AudioStreamPlayer.play()

# SS-70
func startSongByIndex(id: int) -> void:
	selectedTrack = id
	startCurrentSong()

# SS-69
func nextSong() -> void:
	selectedTrack = (selectedTrack + 1) % len(availableSongs)
	startCurrentSong()

func prevSong() -> void:
	selectedTrack = (len(availableSongs) - 1) if (selectedTrack == 0) else (selectedTrack - 1)
	startCurrentSong()

# Called when the node enters the scene tree for the first time.
func _ready():
	var dir = DirAccess.open(musicDir)
	for fname in dir.get_files():
		if !fname.ends_with(".import"):
			if fname == firstSongName:
				selectedTrack = availableSongs.size()
			availableSongs.append(SongInfo.new(musicDir, fname))
	startCurrentSong()


func _on_audio_stream_player_finished():
	if autoPlayNextSong:
		nextSong()
		startCurrentSong()
