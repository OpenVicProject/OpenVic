extends GUINode

var _speed_up_button : Button
var _speed_down_button : Button
var _speed_indicator_button : Button
var _speed_indicator_texture : GFXIconTexture
var _date_label : Label
var _country_name_label : Label

func _ready():
	GameSingleton.state_updated.connect(_update_info)

	add_gui_element("topbar.gui", "topbar")

	hide_nodes([
		^"./topbar/topbar_outlinerbutton_bg",
		^"./topbar/topbar_outlinerbutton"
	])

	const player_country : String = "SLV"

	var player_flag_texture : GFXMaskedFlagTexture = get_gfx_masked_flag_texture_from_node(^"./topbar/player_flag")
	if player_flag_texture:
		player_flag_texture.set_flag_country_name_and_type(player_country, &"")

	_speed_up_button = get_button_node(^"./topbar/button_speedup")
	if _speed_up_button:
		_speed_up_button.pressed.connect(_on_increase_speed_button_pressed)

	_speed_down_button = get_button_node(^"./topbar/button_speeddown")
	if _speed_down_button:
		_speed_down_button.pressed.connect(_on_decrease_speed_button_pressed)

	var pause_bg_button : Button = get_button_node(^"./topbar/pause_bg")
	if pause_bg_button:
		pause_bg_button.pressed.connect(_on_play_pause_button_pressed)

	_date_label = get_label_node(^"./topbar/DateText")

	_country_name_label = get_label_node(^"./topbar/CountryName")
	if _country_name_label:
		_country_name_label.text = player_country

	_speed_indicator_button = get_button_node(^"./topbar/speed_indicator")
	_speed_indicator_texture = get_gfx_icon_texture_from_node(^"./topbar/speed_indicator")

func _update_info() -> void:
	if _date_label:
		_date_label.text = GameSingleton.get_longform_date()

	#  TODO - add disabled state textures so this doesn't hide the buttons
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
func _on_play_pause_button_pressed():
	print("Toggling pause!")
	GameSingleton.toggle_paused()
	_update_info()

# REQUIREMENTS:
# * UIFUN-72
func _on_increase_speed_button_pressed():
	print("Speed up!")
	GameSingleton.increase_speed()
	_update_info()

# REQUIREMENTS:
# * UIFUN-73
func _on_decrease_speed_button_pressed():
	print("Speed down!")
	GameSingleton.decrease_speed()
	_update_info()
