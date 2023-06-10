extends Node

# REQUIREMENTS
# * SS-67
@export_dir var music_directory : String
@export var first_song_name : String

@export var _audio_stream_player : AudioStreamPlayer

var _selected_track = 0
var _available_songs : Array[SongInfo] = []
var _auto_play_next_song : bool = true

## True if music player should be visible.
## Used to keep keep consistency between scene changes
var is_music_player_visible : bool = true

var _has_startup_happened : bool = false

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
	var percentInSeconds : float = (percentage / 100.0) * _audio_stream_player.stream.get_length()
	_audio_stream_player.play(percentInSeconds)

func get_current_song_progress_percentage() -> float:
	return 100 * (_audio_stream_player.get_playback_position() / _audio_stream_player.stream.get_length())

func is_paused() -> bool:
	return _audio_stream_player.stream_paused

func set_paused(paused : bool) -> void:
	_audio_stream_player.stream_paused = paused

func toggle_play_pause() -> void:
	_audio_stream_player.stream_paused = !_audio_stream_player.stream_paused

func start_current_song() -> void:
	_audio_stream_player.stream = _available_songs[_selected_track].song_stream
	_audio_stream_player.play()

# REQUIREMENTS
# * SS-70
func start_song_by_index(id: int) -> void:
	_selected_track = id
	start_current_song()

# REQUIREMENTS
# * SS-69
func select_next_song() -> void:
	_selected_track = (_selected_track + 1) % len(_available_songs)
	start_current_song()

func select_previous_song() -> void:
	_selected_track = (len(_available_songs) - 1) if (_selected_track == 0) else (_selected_track - 1)
	start_current_song()

# REQUIREMENTS
# * SND-2
func _ready():
	var dir = DirAccess.open(music_directory)
	for fname in dir.get_files():
		if fname.ends_with(".import"):
			fname = fname.get_basename()
			if fname.get_basename() == first_song_name:
				_selected_track = _available_songs.size()
			_available_songs.append(SongInfo.new(music_directory, fname))
	start_current_song()
	set_paused(true)

func set_startup_music(play : bool) -> void:
	if not _has_startup_happened:
		_has_startup_happened = true
		set_paused(not play)

func _on_audio_stream_player_finished():
	if _auto_play_next_song:
		select_next_song()
		start_current_song()
