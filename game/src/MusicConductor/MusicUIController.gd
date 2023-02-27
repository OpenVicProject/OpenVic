extends Control


# Called when the node enters the scene tree for the first time.
func _ready():
	updateMusicLabel()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	$VBoxContainer/ProgressSlider.value = MusicConductor.getCurrentSongProgressPercentage()

func updateMusicLabel():
	$VBoxContainer/MusicNameLabel.text = MusicConductor.getCurrentSongName()

func updatePlayPauseButtonVisual():
	$VBoxContainer/HBoxContainer/PlayPauseButton.text = "||" if MusicConductor.isPaused() else ">"

func _on_play_pause_button_pressed():
	MusicConductor.togglePlayPause()
	updatePlayPauseButtonVisual()

func _on_next_song_button_pressed():
	MusicConductor.nextSong()
	updateMusicLabel()
	updatePlayPauseButtonVisual()

func _on_previous_song_button_pressed():
	MusicConductor.prevSong()
	updateMusicLabel()
	updatePlayPauseButtonVisual()
