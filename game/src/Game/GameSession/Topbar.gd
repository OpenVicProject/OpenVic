extends GUINode

var _speed_up_button : Button
var _speed_down_button : Button
var _speed_indicator_button : Button
var _speed_indicator_texture : GFXIconTexture
var _date_label : Label
var _country_name_label : Label

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)
	GameSingleton.clock_state_changed.connect(_update_speed_controls)

	add_gui_element("topbar.gui", "topbar")

	hide_nodes([
		^"./topbar/topbar_outlinerbutton_bg",
		^"./topbar/topbar_outlinerbutton"
	])

	const player_country : String = "SLV"

	var player_flag_texture : GFXMaskedFlagTexture = get_gfx_masked_flag_texture_from_nodepath(^"./topbar/player_flag")
	if player_flag_texture:
		player_flag_texture.set_flag_country_name(player_country)

	_speed_up_button = get_button_from_nodepath(^"./topbar/button_speedup")
	if _speed_up_button:
		_speed_up_button.pressed.connect(_on_increase_speed_button_pressed)

	_speed_down_button = get_button_from_nodepath(^"./topbar/button_speeddown")
	if _speed_down_button:
		_speed_down_button.pressed.connect(_on_decrease_speed_button_pressed)

	var pause_bg_button : Button = get_button_from_nodepath(^"./topbar/pause_bg")
	if pause_bg_button:
		pause_bg_button.pressed.connect(_on_play_pause_button_pressed)

	_date_label = get_label_from_nodepath(^"./topbar/DateText")

	_country_name_label = get_label_from_nodepath(^"./topbar/CountryName")
	if _country_name_label:
		_country_name_label.text = player_country

	_speed_indicator_button = get_button_from_nodepath(^"./topbar/speed_indicator")
	if _speed_indicator_button:
		_speed_indicator_button.pressed.connect(_on_play_pause_button_pressed)
		_speed_indicator_texture = GUINode.get_gfx_icon_texture_from_node(_speed_indicator_button)

	_update_info()
	_update_speed_controls()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()

func _update_info() -> void:
	if _date_label:
		_date_label.text = GameSingleton.get_longform_date()

func _update_speed_controls() -> void:
	#  TODO - decide whether to disable these or not
	# (they don't appear to get disabled in the base game)
	#if _speed_up_button:
	#	_speed_up_button.disabled = not GameSingleton.can_increase_speed()

	#if _speed_down_button:
	#	_speed_down_button.disabled = not GameSingleton.can_decrease_speed()

	if _speed_indicator_button and _speed_indicator_texture:
		var index : int = 1
		if not GameSingleton.is_paused():
			index += GameSingleton.get_speed() + 1
		_speed_indicator_texture.set_icon_index(index)
		_speed_indicator_button.queue_redraw()

# REQUIREMENTS:
# * UIFUN-71
func _on_play_pause_button_pressed() -> void:
	print("Toggling pause!")
	GameSingleton.toggle_paused()

# REQUIREMENTS:
# * UIFUN-72
func _on_increase_speed_button_pressed() -> void:
	print("Speed up!")
	GameSingleton.increase_speed()

# REQUIREMENTS:
# * UIFUN-73
func _on_decrease_speed_button_pressed() -> void:
	print("Speed down!")
	GameSingleton.decrease_speed()
