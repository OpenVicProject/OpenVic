extends Control

@export var _song_selector_button : OptionButton
@export var _progress_slider : HSlider
@export var _previous_song_button : Button
@export var _play_pause_button : Button
@export var _next_song_button : Button
@export var _visbility_button : Button

var _is_user_dragging_progress_slider : bool = false

func _ready():
	for songName in MusicConductor.get_all_song_names():
		_song_selector_button.add_item(songName, _song_selector_button.item_count)
	_update_song_name_visual()
	_update_play_pause_button()
	_set_music_player_visible(MusicConductor.is_music_player_visible)


func _process(_delta):
	if !_is_user_dragging_progress_slider:
		_progress_slider.value = MusicConductor.get_current_song_progress_percentage()

func _update_song_name_visual():
	_song_selector_button.selected = MusicConductor.get_current_song_index()

func _update_play_pause_button():
	_play_pause_button.text = "◼" if MusicConductor.is_paused() else "▶"

func _on_play_pause_button_pressed():
	MusicConductor.toggle_play_pause()
	_update_play_pause_button()

# REQUIREMENTS
# * UIFUN-93
func _on_next_song_button_pressed():
	MusicConductor.select_next_song()
	_update_song_name_visual()
	_update_play_pause_button()

# REQUIREMENTS
# * UIFUN-94
func _on_previous_song_button_pressed():
	MusicConductor.select_previous_song()
	_update_song_name_visual()
	_update_play_pause_button()

# REQUIREMENTS
# * UIFUN-95
func _on_option_button_item_selected(index):
	MusicConductor.start_song_by_index(index)
	_update_song_name_visual()
	_update_play_pause_button()


func _on_progress_slider_drag_started():
	_is_user_dragging_progress_slider = true


func _on_progress_slider_drag_ended(_value_changed):
	MusicConductor.scrub_song_by_percentage(_progress_slider.value)
	_is_user_dragging_progress_slider = false
	_update_play_pause_button()

func _set_music_player_visible(is_player_visible : bool) -> void:
	MusicConductor.is_music_player_visible = is_player_visible
	_visbility_button.text = "⬆️" if is_player_visible else "⬇"
	_song_selector_button.visible = is_player_visible
	_progress_slider.visible = is_player_visible
	_previous_song_button.visible = is_player_visible
	_play_pause_button.visible = is_player_visible
	_next_song_button.visible = is_player_visible

# REQUIREMENTS
# * UIFUN-91
func _on_music_ui_visibility_button_pressed():
	_set_music_player_visible(not MusicConductor.is_music_player_visible)
