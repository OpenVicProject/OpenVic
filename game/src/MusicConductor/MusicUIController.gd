extends Control
# UI-104

var isMusicPlayerVisible : bool = true
var isUserDraggingProgressSlider : bool = false

# Called when the node enters the scene tree for the first time.
func _ready():
	for songName in MusicConductor.getAllSongNames():
		$VBoxContainer/SongSelectorButton.add_item(songName, $VBoxContainer/SongSelectorButton.item_count)
	updateSongNameVisual()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	if !isUserDraggingProgressSlider:
		$VBoxContainer/ProgressSlider.value = MusicConductor.getCurrentSongProgressPercentage()

func updateSongNameVisual():
	$VBoxContainer/SongSelectorButton.selected = MusicConductor.getCurrentSongIndex()

func updatePlayPauseButtonVisual():
	$VBoxContainer/HBoxContainer/PlayPauseButton.text = "||" if MusicConductor.isPaused() else ">"

func _on_play_pause_button_pressed():
	MusicConductor.togglePlayPause()
	updatePlayPauseButtonVisual()

func _on_next_song_button_pressed():
	MusicConductor.nextSong()
	updateSongNameVisual()
	updatePlayPauseButtonVisual()

func _on_previous_song_button_pressed():
	MusicConductor.prevSong()
	updateSongNameVisual()
	updatePlayPauseButtonVisual()

# UI-107
func _on_option_button_item_selected(index):
	# UIFUN-92
	MusicConductor.startSongByIndex(index)


func _on_progress_slider_drag_started():
	isUserDraggingProgressSlider = true


func _on_progress_slider_drag_ended(_value_changed):
	MusicConductor.scrubSongByPercentage($VBoxContainer/ProgressSlider.value)
	isUserDraggingProgressSlider = false

# UI-107
func _on_music_ui_visibility_button_pressed():
	isMusicPlayerVisible = !isMusicPlayerVisible
	$VBoxContainer/MusicUIVisibilityButton.text = "Hide Player" if isMusicPlayerVisible else "Show Player"
	$VBoxContainer/SongSelectorButton.visible = isMusicPlayerVisible
	$VBoxContainer/ProgressSlider.visible = isMusicPlayerVisible
	$VBoxContainer/HBoxContainer.visible = isMusicPlayerVisible
