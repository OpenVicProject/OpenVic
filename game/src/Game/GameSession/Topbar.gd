extends GUINode

@export var _outliner_guinode : GUINode

var _speed_up_button : Button
var _speed_down_button : Button
var _speed_indicator_button : Button
var _speed_indicator_texture : GFXSpriteTexture
var _date_label : Label
var _country_name_label : Label

# NationManagement.Screen-Button
var _nation_management_buttons : Dictionary
# NationManagement.Screen-GFXSpriteTexture
var _nation_management_button_textures : Dictionary

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)
	GameSingleton.clock_state_changed.connect(_update_speed_controls)

	add_gui_element("topbar", "topbar")

	hide_nodes([
		^"./topbar/topbar_outlinerbutton_bg",
		^"./topbar/topbar_outlinerbutton"
	])

	const player_country : String = "SLV"

	# Player country info
	var player_flag_texture : GFXMaskedFlagTexture = get_gfx_masked_flag_texture_from_nodepath(^"./topbar/player_flag")
	if player_flag_texture:
		player_flag_texture.set_flag_country_name(player_country)

	# Time controls
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
		_speed_indicator_texture = GUINode.get_gfx_sprite_texture_from_node(_speed_indicator_button)

	# Nation management screens
	const screen_nodepaths : Dictionary = {
		NationManagement.Screen.PRODUCTION : ^"./topbar/topbarbutton_production",
		NationManagement.Screen.BUDGET     : ^"./topbar/topbarbutton_budget",
		NationManagement.Screen.TECHNOLOGY : ^"./topbar/topbarbutton_tech",
		NationManagement.Screen.POLITICS   : ^"./topbar/topbarbutton_politics",
		NationManagement.Screen.POPULATION : ^"./topbar/topbarbutton_pops",
		NationManagement.Screen.TRADE      : ^"./topbar/topbarbutton_trade",
		NationManagement.Screen.DIPLOMACY  : ^"./topbar/topbarbutton_diplomacy",
		NationManagement.Screen.MILITARY   : ^"./topbar/topbarbutton_military"
	}
	for screen : NationManagement.Screen in screen_nodepaths:
		var button : Button = get_button_from_nodepath(screen_nodepaths[screen])
		if button:
			button.pressed.connect(
				Events.NationManagementScreens.toggle_nation_management_screen.bind(screen)
			)
			var icon : GFXSpriteTexture = GUINode.get_gfx_sprite_texture_from_node(button)
			if icon:
				_nation_management_buttons[screen] = button
				_nation_management_button_textures[screen] = icon
	Events.NationManagementScreens.update_active_nation_management_screen.connect(
		_on_update_active_nation_management_screen
	)

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

func _on_update_active_nation_management_screen(active_screen : NationManagement.Screen) -> void:
	for screen : NationManagement.Screen in _nation_management_buttons:
		_nation_management_button_textures[screen].set_icon_index(1 + int(screen == active_screen))
