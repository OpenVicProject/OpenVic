extends Control

@export var _song_selector_button : OptionButton
@export var _progress_slider : HSlider
@export var _previous_song_button : Button
@export var _play_pause_button : Button
@export var _next_song_button : Button
@export var _visibility_button : Button

var _is_user_dragging_progress_slider : bool = false

func _ready() -> void:
	for songName : String in MusicManager.get_all_song_names():
		_song_selector_button.add_item(songName, _song_selector_button.item_count)
	_on_song_set(MusicManager.get_current_song_index())
	MusicManager.song_started.connect(_on_song_set)
	MusicManager.song_paused.connect(_update_play_pause_button)
	MusicManager.song_scrubbed.connect(_update_play_pause_button)
	_set_music_player_visible(MusicManager.is_music_player_visible)

func _on_song_set(track_id : int) -> void:
	_song_selector_button.selected = track_id
	_update_play_pause_button()

func _process(_delta : float) -> void:
	if !_is_user_dragging_progress_slider:
		_progress_slider.value = MusicManager.get_current_song_progress_percentage()

func _update_play_pause_button(_arg1 : Variant = null, _arg2 : Variant = null) -> void:
	_play_pause_button.text = "▶️" if MusicManager.is_paused() else "❚❚"

func _on_play_pause_button_pressed() -> void:
	MusicManager.toggle_play_pause()

# REQUIREMENTS
# * UIFUN-93
func _on_next_song_button_pressed() -> void:
	MusicManager.select_next_song()

# REQUIREMENTS
# * UIFUN-94
func _on_previous_song_button_pressed() -> void:
	MusicManager.select_previous_song()

# REQUIREMENTS
# * UIFUN-95
func _on_option_button_item_selected(index : int) -> void:
	MusicManager.start_song_by_index(index)

func _on_progress_slider_drag_started() -> void:
	_is_user_dragging_progress_slider = true

func _on_progress_slider_drag_ended(_value_changed : bool) -> void:
	MusicManager.scrub_song_by_percentage(_progress_slider.value)
	_is_user_dragging_progress_slider = false

func _set_music_player_visible(is_player_visible : bool) -> void:
	MusicManager.is_music_player_visible = is_player_visible
	_visibility_button.text = "⬆️" if is_player_visible else "⬇"
	_song_selector_button.visible = is_player_visible
	_progress_slider.visible = is_player_visible
	_previous_song_button.visible = is_player_visible
	_play_pause_button.visible = is_player_visible
	_next_song_button.visible = is_player_visible

# REQUIREMENTS
# * UIFUN-91
func _on_music_ui_visibility_button_pressed() -> void:
	_set_music_player_visible(not MusicManager.is_music_player_visible)
