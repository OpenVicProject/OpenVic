extends Node

# SS-67
@export_dir var music_directory : String
@export var first_song_name : String

var _selected_track = 0
var _available_songs : Array[SongInfo] = []
var _auto_play_next_song : bool = true

## True if music player should be visible.
## Used to keep keep consistency between scene changes
var is_music_player_visible : bool = true

func get_all_song_names() -> Array[String]:
	var songNames : Array[String] = []
	for si in _available_songs:
		songNames.append(si.song_name)
	return songNames

func get_current_song_index() -> int:
	return _selected_track

func get_current_song_name() -> String:
	return _available_songs[_selected_track].song_name

func scrub_song_by_percentage(percentage: float) -> void:
	var percentInSeconds : float = (percentage / 100.0) * $AudioStreamPlayer.stream.get_length()
	$AudioStreamPlayer.play(percentInSeconds)

func get_current_song_progress_percentage() -> float:
	return 100 * ($AudioStreamPlayer.get_playback_position() / $AudioStreamPlayer.stream.get_length())

func is_paused() -> bool:
	return $AudioStreamPlayer.stream_paused

func toggle_play_pause() -> void:
	$AudioStreamPlayer.stream_paused = !$AudioStreamPlayer.stream_paused
	
func start_current_song() -> void:
	$AudioStreamPlayer.stream = _available_songs[_selected_track].song_stream
	$AudioStreamPlayer.play()

# SS-70
func start_song_by_index(id: int) -> void:
	_selected_track = id
	start_current_song()

# SS-69
func select_next_song() -> void:
	_selected_track = (_selected_track + 1) % len(_available_songs)
	start_current_song()

func select_previous_song() -> void:
	_selected_track = (len(_available_songs) - 1) if (_selected_track == 0) else (_selected_track - 1)
	start_current_song()

func _ready():
	var dir = DirAccess.open(music_directory)
	for fname in dir.get_files():
		if !fname.ends_with(".import"):
			if fname.get_basename() == first_song_name:
				_selected_track = _available_songs.size()
			_available_songs.append(SongInfo.new(music_directory, fname))
	start_current_song()


func _on_audio_stream_player_finished():
	if _auto_play_next_song:
		select_next_song()
		start_current_song()
