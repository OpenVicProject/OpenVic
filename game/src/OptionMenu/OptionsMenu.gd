extends Control

@export
var user_settings_file_path : String = "settings.cfg"

signal back_button_pressed

signal save_settings(save_file: ConfigFile)
signal load_settings(load_file: ConfigFile)
signal reset_settings()

@onready
var _settings_file_path := "user://" + user_settings_file_path
var _settings_file := ConfigFile.new()

func _ready():
	# Prepare options menu before loading user settings

	print("TODO: Load user settings!")

	if FileAccess.file_exists(_settings_file_path):
		_settings_file.load(_settings_file_path)
	load_settings.emit(_settings_file)

	var tab_bar : TabBar = $Margin/Tab.get_child(0, true)

	# This ends up easier to manage then trying to manually recreate the TabContainer's behavior
	# These buttons can be accessed regardless of the tab
	var button_list := HBoxContainer.new()
	button_list.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	button_list.alignment = BoxContainer.ALIGNMENT_END
	tab_bar.add_child(button_list)

	var reset_button := Button.new()
	reset_button.text = "R"
	reset_button.pressed.connect(func(): reset_settings.emit())
	button_list.add_child(reset_button)

	var back_button := Button.new()
	back_button.text = "X"
	back_button.pressed.connect(_on_back_button_pressed)
	button_list.add_child(back_button)

	get_viewport().get_window().close_requested.connect(_on_window_close_requested)

func _notification(what):
	match what:
		NOTIFICATION_CRASH:
			_on_window_close_requested()

# Could pass the LocaleButton between the MainMenu and OptionsMenu
# but that seems a bit excessive
func toggle_locale_button_visibility(locale_visible : bool):
	print("Toggling locale button: %s" % locale_visible)
	$LocaleVBox/LocaleHBox/LocaleButton.visible = locale_visible

func _on_ear_exploder_toggled(button_pressed):
	print("KABOOM!!!" if button_pressed else "DEFUSED!!!")


func _on_back_button_pressed():
	save_settings.emit(_settings_file)
	_settings_file.save(_settings_file_path)
	back_button_pressed.emit()


func _on_spin_box_value_changed(value):
	print("Spinbox: %d" % value)

func _on_window_close_requested() -> void:
	if visible:
		_on_back_button_pressed()
