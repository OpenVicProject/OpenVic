@tool
extends "res://addons/kenyoni/app_settings/app_settings.gd"

enum SaveGameFormat {
	BINARY,
	TEXT,
}

# gdstyle:ignore=order/class-member-order
const SAVE_GAME_FORMAT_DISPLAY_NAMES: PackedStringArray = [
	"Binary",
	"Text",
]

enum AutoSaveInterval {
	MONTHLY,
	BIMONTHLY,
	YEARLY,
	BIYEARLY,
	NEVER,
}

# gdstyle:ignore=order/class-member-order
const AUTO_SAVE_INTERVAL_DISPLAY_NAMES: PackedStringArray = [
	"Monthly",
	"Bi-Monthly",
	"Yearly",
	"Bi-Yearly",
	"Never",
]

enum RefreshRate {
	VSYNC,
	VSYNC_ADAPTIVE,
	VSYNC_MAILBOX,
	_30HZ,
	_60HZ,
	_90HZ,
	_120HZ,
	_144HZ,
	_365HZ,
	UNLIMITED,
}

# gdstyle:ignore=order/class-member-order
const REFRESH_RATE_DISPLAY_NAMES: PackedStringArray = [
	"VSync",
	"Adaptive VSync",
	"Mailbox VSynx",
	"30hz",
	"60hz",
	"90hz",
	"120hz",
	"144hz",
	"365hz",
	"Unlimited",
]

enum GraphicsDetail {
	LOW,
	MEDIUM,
	HIGH,
	ULTRA,
	CUSTOM,
}

# gdstyle:ignore=order/class-member-order
const GRAPHICS_DETAIL_DISPLAY_NAMES: PackedStringArray = [
	"Low",
	"Medium",
	"High",
	"Ultra",
	"Custom",
]
#
# Resolution
const RESOLUTIONS: Array[Vector2i] = [
	Vector2i(3840, 2160),
	Vector2i(2560, 1080),
	Vector2i(1920, 1080),
	Vector2i(1366, 768),
	Vector2i(1536, 864),
	Vector2i(1280, 720),
	Vector2i(1440, 900),
	Vector2i(1600, 900),
	Vector2i(1024, 600),
	Vector2i(800, 600),
]
const RESOLUTION_DISPLAY_NAMES: PackedStringArray = [
	"3840x2160 (4K)",
	"2560x1080 (UW1080p)",
	"1920x1080 (1080p)",
	"1366x768",
	"1536x864",
	"1280x720 (720p, Default)",
	"1440x900",
	"1600x900",
	"1024x600",
	"800x600",
]
#
# ScreenModes
const SCREEN_MODES: PackedInt32Array = [
	DisplayServer.WINDOW_MODE_FULLSCREEN,
	DisplayServer.WINDOW_MODE_EXCLUSIVE_FULLSCREEN,
	DisplayServer.WINDOW_MODE_WINDOWED,
]
const SCREEN_MODES_DISPLAY_NAMES: PackedStringArray = [
	"Fullscreen",
	"Borderless",
	"Windowed",
]
#
# File
const SETTINGS_FILE := "user://settings.cfg"
#
# General Settings
const GENERAL_SAVE_GAME_FORMAT := &"general/save_game_format"
const GENERAL_AUTO_SAVE_INTERVAL := &"general/auto_save_interval"
const GENERAL_LANGUAGE := &"general/language"
#
# Video Settings
const VIDEO_RESOLUTION := &"video/resolution"
const VIDEO_GUI_SCALING_FACTOR := &"video/gui_scaling_factor"
const VIDEO_SCREEN_MODE := &"video/screen_mode"
const VIDEO_MONITOR_SELECTION := &"video/monitor_selection"
const VIDEO_REFRESH_RATE := &"video/refresh_rate"
const VIDEO_QUALITY_PRESET := &"video/quality_preset"
#
# Audio Settings
const AUDIO_MASTER_VOLUME := &"audio/master_volume"
const AUDIO_MUSIC_VOLUME := &"audio/music_volume"
const AUDIO_SFX_VOLUME := &"audio/sfx_volume"
const AUDIO_MUSIC_START_PLAY := &"audio/music_start_play"
#
# Internal Settings
const INTERNAL_WINDOW_WIDTH = &"display/window/size/viewport_width"
const INTERNAL_WINDOW_HEIGHT = &"display/window/size/viewport_height"

var video_revert_group := RevertGroup.new(
	"Keep Video Changes?",
	"Reverting changes in {time} seconds...",
)


func _init() -> void:
	# General Settings
	self.add_setting(Setting.new(GENERAL_SAVE_GAME_FORMAT, SaveGameFormat.BINARY)
		.set_description("The type of format to save.")
		.set_staged()
		.set_validate_fn(_enum_validate)
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_ENUM)
		.add_meta(&"values", SaveGameFormat.values())
		.add_meta(&"display_values", SAVE_GAME_FORMAT_DISPLAY_NAMES))
	self.add_setting(Setting.new(GENERAL_AUTO_SAVE_INTERVAL, AutoSaveInterval.MONTHLY)
		.set_description("The ingame interval to auto-save by.")
		.set_staged()
		.set_validate_fn(_enum_validate)
		.add_meta(&"display_name", "Auto-Save Interval")
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_ENUM)
		.add_meta(&"values", AutoSaveInterval.values())
		.add_meta(&"display_values", AUTO_SAVE_INTERVAL_DISPLAY_NAMES))
	self.add_setting(Setting.new(GENERAL_LANGUAGE, Localisation.get_default_locale())
		.set_description("The language of the game.")
		.set_validate_fn(
			func(_stg: Setting, val: Variant) -> bool:
				return TranslationServer.has_translation_for_locale(val, false)
				)
		.set_apply_fn(func(stg: Setting) -> void: TranslationServer.set_locale(stg.value() as String))
		.add_meta(&"type", TYPE_STRING)
		.add_meta(&"hint", PROPERTY_HINT_ENUM)
		.add_meta(&"values", _get_loaded_locales())
		.add_meta(&"display_values", _get_loaded_locale_names()))

	# Video Settings
	self.add_setting(Setting.new(VIDEO_RESOLUTION, Vector2i(
			ProjectSettings.get_setting("display/window/size/viewport_width"),
			ProjectSettings.get_setting("display/window/size/viewport_height")
		))
		.set_description("The video resolution for the game.")
		.set_apply_fn(_resolution_apply)
		.add_meta(&"type", TYPE_VECTOR2I)
		.add_meta(&"hint", PROPERTY_HINT_ENUM)
		.add_meta(&"values", RESOLUTIONS)
		.add_meta(&"display_values", RESOLUTION_DISPLAY_NAMES)
		.add_meta(&"no_default", true)
		.add_meta(&"revert_group", video_revert_group))
	self.add_setting(Setting.new(VIDEO_GUI_SCALING_FACTOR, 1)
		.set_description("The scaling factor for the game's GUI.")
		.set_staged()
		.set_apply_fn(_gui_scaling_factor_apply)
		.add_meta(&"type", TYPE_FLOAT)
		.add_meta(&"hint", PROPERTY_HINT_RANGE)
		.add_meta(&"max", 2)
		.add_meta(&"step", 0.1))
	self.add_setting(Setting.new(VIDEO_SCREEN_MODE, DisplayServer.WINDOW_MODE_FULLSCREEN)
		.set_description("The windowing mode of the game.")
		.set_validate_fn(_screen_mode_validate)
		.set_apply_fn(_screen_mode_apply)
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_ENUM)
		.add_meta(&"values", SCREEN_MODES)
		.add_meta(&"display_values", SCREEN_MODES_DISPLAY_NAMES)
		.add_meta(&"revert_group", video_revert_group))
	self.add_setting(Setting.new(VIDEO_MONITOR_SELECTION, 0)
		.set_description("The monitor to display the game to.")
		.set_apply_fn(_monitor_selection_apply)
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_ENUM)
		.add_meta(&"values", range(1, DisplayServer.get_screen_count() + 1))
		.add_meta(&"display_template", "Display {value}")
		.add_meta(&"revert_group", video_revert_group))
	self.add_setting(Setting.new(VIDEO_REFRESH_RATE, RefreshRate.VSYNC)
		.set_description("The refresh rate for the game.")
		.set_staged()
		.set_validate_fn(_enum_validate)
		.set_apply_fn(_refresh_rate_apply)
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_ENUM)
		.add_meta(&"values", RefreshRate.values())
		.add_meta(&"display_values", REFRESH_RATE_DISPLAY_NAMES))
	self.add_setting(Setting.new(VIDEO_QUALITY_PRESET, GraphicsDetail.MEDIUM)
		.set_description("Graphical detail level of the game.")
		.set_staged()
		.set_validate_fn(_enum_validate)
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_ENUM)
		.add_meta(&"values", GraphicsDetail.values())
		.add_meta(&"display_values", GRAPHICS_DETAIL_DISPLAY_NAMES))

	# Audio Settings
	self.add_setting(Setting.new(AUDIO_MASTER_VOLUME, 100)
		.set_description("Game's Master volume.")
		.set_apply_fn(_volume_apply.bind(AudioServer.get_bus_index(&"Master")))
		.set_validate_fn(_volume_validate)
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_RANGE))
	self.add_setting(Setting.new(AUDIO_MUSIC_VOLUME, 100)
		.set_description("Game's Music volume.")
		.set_apply_fn(_volume_apply.bind(AudioServer.get_bus_index(&"MUSIC_BUS")))
		.set_validate_fn(_volume_validate)
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_RANGE))
	self.add_setting(Setting.new(AUDIO_SFX_VOLUME, 100)
		.set_description("Game's Sound Effects volume.")
		.set_apply_fn(_volume_apply.bind(AudioServer.get_bus_index(&"SFX_BUS")))
		.set_validate_fn(_volume_validate)
		.add_meta(&"display_name", "SFX Volume")
		.add_meta(&"type", TYPE_INT)
		.add_meta(&"hint", PROPERTY_HINT_RANGE))
	self.add_setting(Setting.new(AUDIO_MUSIC_START_PLAY, true)
		.set_description("Whether to start the game with music already playing.")
		.add_meta(&"display_name", "Auto-Start Music")
		.add_meta(&"type", TYPE_BOOL))

	self.add_setting(Setting.new(INTERNAL_WINDOW_WIDTH, ProjectSettings.get_setting(INTERNAL_WINDOW_WIDTH))
		.set_internal())
	self.add_setting(Setting.new(INTERNAL_WINDOW_HEIGHT, ProjectSettings.get_setting(INTERNAL_WINDOW_HEIGHT))
		.set_internal())

	if Engine.is_editor_hint(): return

	self.load()
	self.apply_all()

	_set_window_override(get_setting(VIDEO_RESOLUTION).value() as Vector2i)

	await tree_entered
	_resolution_apply(self.get_setting(VIDEO_RESOLUTION))
	# Preserves GUI scaling factor on scene change
	_gui_scaling_factor_apply(self.get_setting(VIDEO_GUI_SCALING_FACTOR))
	get_tree().scene_changed.connect(_gui_scaling_factor_apply.bind(self.get_setting(VIDEO_GUI_SCALING_FACTOR)))


func save() -> Error:
	return self.to_config().save(SETTINGS_FILE)


func load() -> Error:
	var cfg: ConfigFile = ConfigFile.new()
	var err: Error = cfg.load(SETTINGS_FILE)
	if err == Error.ERR_FILE_NOT_FOUND:
		return Error.OK
	if err != Error.OK:
		return err
	self.from_config(cfg)

	return Error.OK


func get_game_resolution() -> Vector2i:
	var window := get_window()
	assert(window != null)
	match window.mode:
		Window.MODE_EXCLUSIVE_FULLSCREEN, Window.MODE_FULLSCREEN:
			return window.content_scale_size
		_:
			return window.size


func _enum_validate(stg: Setting, val: Variant) -> bool:
	return val in stg.get_meta(&"values")


func _get_loaded_locales() -> PackedStringArray:
	var result := TranslationServer.get_loaded_locales()
	var default_locale := Localisation.get_default_locale()
	if default_locale not in result:
		result.push_back(default_locale)
	return result


func _get_loaded_locale_names() -> PackedStringArray:
	var locales_country_rename: Dictionary = ProjectSettings.get_setting(
		"internationalization/locale/country_short_name",
		{},
	)

	var result: PackedStringArray = []
	for locale: String in _get_loaded_locales():
		var locale_name := TranslationServer.get_locale_name(locale)
		var comma_idx := locale_name.find(", ")
		if comma_idx != -1:
			var locale_country_name := locale_name.substr(comma_idx + 2)
			locale_country_name = locales_country_rename.get(locale_country_name, "")
			if not locale_country_name.is_empty():
				locale_name = locale_name.left(comma_idx + 2) + locale_country_name
		result.append(locale_name)
	return result


func _resolution_apply(stg: Setting) -> void:
	if Engine.is_embedded_in_editor():
		_push_embedded_warning(str(stg.value()))
		return

	var window := get_window()
	var mode: Window.Mode = Window.MODE_WINDOWED
	if window != null:
		mode = window.mode

	match mode:
		Window.MODE_EXCLUSIVE_FULLSCREEN, Window.MODE_FULLSCREEN:
			if window != null: window.content_scale_size = stg.value() as Vector2i
		_:
			DisplayServer.window_set_size(stg.value() as Vector2i)
			_set_window_override(DisplayServer.window_get_size())
			if window != null: window.content_scale_size = Vector2i(0, 0)


func _gui_scaling_factor_apply(stg: Setting) -> void:
	if not is_inside_tree(): return
	get_window().content_scale_factor = stg.value()


func _screen_mode_validate(_stg: Setting, val: Variant) -> bool:
	return _get_enum_values(&"DisplayServer", &"WindowMode").find_key(val) != null


func _screen_mode_apply(stg: Setting) -> void:
	if Engine.is_embedded_in_editor():
		var window_mode_name: String = _get_enum_values(
			&"DisplayServer",
			&"WindowMode",
		).find_key(stg.value())
		_push_embedded_warning("DisplayServer." + window_mode_name)
		return
	DisplayServer.window_set_mode(stg.value())
	_set_window_override(DisplayServer.window_get_size())


func _monitor_selection_apply(stg: Setting) -> void:
	if not is_inside_tree(): return
	get_window().current_screen = stg.value() - 1


func _refresh_rate_apply(stg: Setting) -> void:
	var refresh_rate := stg.value() as RefreshRate
	match refresh_rate:
		RefreshRate.VSYNC: DisplayServer.window_set_vsync_mode(DisplayServer.VSyncMode.VSYNC_ENABLED)
		RefreshRate.VSYNC_ADAPTIVE: DisplayServer.window_set_vsync_mode(DisplayServer.VSyncMode.VSYNC_ADAPTIVE)
		RefreshRate.VSYNC_MAILBOX: DisplayServer.window_set_vsync_mode(DisplayServer.VSyncMode.VSYNC_MAILBOX)
		_:
			DisplayServer.window_set_vsync_mode(DisplayServer.VSyncMode.VSYNC_DISABLED)
			match refresh_rate:
				RefreshRate._30HZ: Engine.max_fps = 30
				RefreshRate._60HZ: Engine.max_fps = 60
				RefreshRate._90HZ: Engine.max_fps = 90
				RefreshRate._120HZ: Engine.max_fps = 120
				RefreshRate._144HZ: Engine.max_fps = 144
				RefreshRate._365HZ: Engine.max_fps = 365
				RefreshRate.UNLIMITED: Engine.max_fps = 0


func _volume_apply(stg: Setting, bus_index: int) -> void:
	const RATIO_FOR_LINEAR: float = 100
	AudioServer.set_bus_volume_db(bus_index, linear_to_db(stg.value() / RATIO_FOR_LINEAR))


func _volume_validate(stg: Setting, val: Variant) -> bool:
	return val >= stg.get_meta(&"min", 0) and val <= stg.get_meta(&"max", 120)


func _set_window_override(size: Vector2i) -> void:
	get_setting(INTERNAL_WINDOW_WIDTH).set_value(size.x)
	get_setting(INTERNAL_WINDOW_HEIGHT).set_value(size.y)


func _get_enum_values(clazz: StringName, enum_name: StringName) -> Dictionary[StringName, int]:
	var result: Dictionary[StringName, int] = {}
	for value_name: String in ClassDB.class_get_enum_constants(clazz, enum_name):
		result[value_name] = ClassDB.class_get_integer_constant(clazz, value_name)
	return result


func _push_embedded_warning(value: String) -> void:
	push_warning(
		"Setting screen mode to ",
		value,
		" for an editor embedded window, this will not be reflected in the embedded window.",
	)


class RevertGroup:
	var title: String
	var text: String

	func _init(
		ti: String = "Please Confirm...",
		tex: String = "< reverting in {time} seconds >",
	) -> void:
		title = ti
		text = tex
