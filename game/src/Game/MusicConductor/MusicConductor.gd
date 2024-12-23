extends Node

signal song_paused(paused: bool)
signal song_started(track_id: int)
## Only triggers when song naturally ends
signal song_finished(track_id: int)
signal song_scrubbed(percentage: float, seconds: float)

# REQUIREMENTS
# * SS-67
@export_dir var music_directory: String
@export var first_song_name: String

@export var _audio_stream_player: AudioStreamPlayer

var _selected_track := 0
var _available_songs: Array[SongInfo] = []
var _auto_play_next_song: bool = true

var playlist: Array[int] = []
var playlist_index: int = 0
var preferred_playlist_len: int = 7
var last_played: int = -1

## True if music player should be visible.
## Used to keep keep consistency between scene changes
var is_music_player_visible: bool = true

var _has_startup_happened: bool = false


func get_all_song_names() -> PackedStringArray:
	var songNames: PackedStringArray = []
	for si: SongInfo in _available_songs:
		songNames.append(si.song_name)
	return songNames


func get_all_song_paths() -> PackedStringArray:
	var songPaths: PackedStringArray = []
	for si: SongInfo in _available_songs:
		songPaths.append(si.song_path)
	return songPaths


func get_current_song_index() -> int:
	return _selected_track


func get_current_song_name() -> String:
	return _available_songs[_selected_track].song_name


func scrub_song_by_percentage(percentage: float) -> void:
	var percentInSeconds: float = (percentage / 100.0) * _audio_stream_player.stream.get_length()
	_audio_stream_player.play(percentInSeconds)
	song_scrubbed.emit(percentage, percentInSeconds)


func get_current_song_progress_percentage() -> float:
	return (
		100
		* (_audio_stream_player.get_playback_position() / _audio_stream_player.stream.get_length())
	)


func is_paused() -> bool:
	return _audio_stream_player.stream_paused


func set_paused(paused: bool) -> void:
	_audio_stream_player.stream_paused = paused
	song_paused.emit(paused)


func toggle_play_pause() -> void:
	set_paused(not is_paused())


func start_current_song() -> void:
	_audio_stream_player.stream = _available_songs[_selected_track].song_stream
	_audio_stream_player.play()
	song_started.emit(_selected_track)


# REQUIREMENTS
# * SS-70
func start_song_by_index(id: int) -> void:
	_selected_track = id
	start_current_song()


# REQUIREMENTS
# * SS-69
func select_next_song() -> void:
	#_selected_track = (_selected_track + 1) % len(_available_songs)
	if playlist_index >= preferred_playlist_len or playlist_index >= len(playlist):
		generate_playlist()
		playlist_index = 0
	_selected_track = playlist[playlist_index]
	playlist_index += 1
	last_played = playlist_index
	start_current_song()


func select_previous_song() -> void:
	_selected_track = (
		(len(_available_songs) - 1) if (_selected_track == 0) else (_selected_track - 1)
	)
	start_current_song()


func setup_compat_song(file_name) -> void:
	var song = SongInfo.new()
	var stream = SoundSingleton.get_song(file_name)

	if stream == null:
		push_error("Audio Stream for compat song %s was null" % file_name)
		return

	var metadata = MusicMetadata.new()
	metadata.set_from_stream(stream)
	var title = metadata.title

	if title == "":
		#use the file name without the extension if there's no metadata
		title = file_name.split(".")[0]
	song.init_stream(file_name, title, stream)
	_available_songs.append(song)


func add_compat_songs() -> void:
	for file_name: String in SoundSingleton.song_list:
		setup_compat_song(file_name)


func add_ootb_music() -> void:
	var dir := DirAccess.open(music_directory)
	for fname: String in dir.get_files():
		if fname.ends_with(".import"):
			fname = fname.get_basename()
			if fname.get_basename() == first_song_name:
				_selected_track = _available_songs.size()
			var song = SongInfo.new()
			song.init_file_path(music_directory, fname)
			_available_songs.append(song)


func generate_playlist() -> void:
	var song_names = MusicConductor.get_all_song_paths()
	var possible_indices = range(len(song_names) - 1)

	var title_index = song_names.find(SoundSingleton.title_theme)
	possible_indices.remove_at(title_index)

	var actual_playlist_len = min(preferred_playlist_len, len(possible_indices))

	#if the playlist size is too large or small, make it the same size as what we
	#need to support
	if len(playlist) != actual_playlist_len:
		playlist.resize(actual_playlist_len)
		playlist.fill(0)

	#The song we just played can be in the playlist, just not the first one
	if last_played != -1:
		possible_indices.remove_at(last_played)

	#essentially shuffle-bag randomness, picking from a list of song indices
	for i in range(actual_playlist_len):
		var ind = randi_range(0, len(possible_indices) - 1)
		#add back the last song we just played as an option
		if i == 2:
			possible_indices.append(last_played)

		playlist[i] = possible_indices[ind]
		possible_indices.remove_at(ind)


# REQUIREMENTS
# * SND-2, SND-3
func _ready() -> void:
	add_ootb_music()
	#don't start the current song for compat mode, do that from
	#GameStart so we can wait until the music is loaded


func set_startup_music(play: bool) -> void:
	if not _has_startup_happened:
		_has_startup_happened = true
		set_paused(not play)


func _on_audio_stream_player_finished() -> void:
	song_finished.emit(_selected_track)
	if _auto_play_next_song:
		select_next_song()
		start_current_song()
