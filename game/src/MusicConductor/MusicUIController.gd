extends Control

@export var songSelectorButton : OptionButton
@export var progressSlider : HSlider
@export var prevSongButton : Button
@export var playPauseButton : Button
@export var nextSongButton : Button
@export var widgetVisibilityButton : Button

var isMusicPlayerVisible : bool = true
var isUserDraggingProgressSlider : bool = false

# Called when the node enters the scene tree for the first time.
func _ready():
	for songName in MusicConductor.getAllSongNames():
		songSelectorButton.add_item(songName, songSelectorButton.item_count)
	updateSongNameVisual()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	if !isUserDraggingProgressSlider:
		progressSlider.value = MusicConductor.getCurrentSongProgressPercentage()

func updateSongNameVisual():
	songSelectorButton.selected = MusicConductor.getCurrentSongIndex()

func updatePlayPauseButtonVisual():
	playPauseButton.text = "||" if MusicConductor.isPaused() else ">"

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

func _on_option_button_item_selected(index):
	# UIFUN-92
	MusicConductor.startSongByIndex(index)


func _on_progress_slider_drag_started():
	isUserDraggingProgressSlider = true


func _on_progress_slider_drag_ended(_value_changed):
	MusicConductor.scrubSongByPercentage(progressSlider.value)
	isUserDraggingProgressSlider = false
	updatePlayPauseButtonVisual()

func _on_music_ui_visibility_button_pressed():
	isMusicPlayerVisible = !isMusicPlayerVisible
	widgetVisibilityButton.text = "Hide Player" if isMusicPlayerVisible else "Show Player"
	songSelectorButton.visible = isMusicPlayerVisible
	progressSlider.visible = isMusicPlayerVisible
	prevSongButton.visible = isMusicPlayerVisible
	playPauseButton.visible = isMusicPlayerVisible
	nextSongButton.visible = isMusicPlayerVisible
