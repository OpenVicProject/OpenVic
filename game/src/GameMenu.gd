extends Control

# REQUIREMENTS
# * SS-10
func _ready():
	Events.Options.load_settings_from_file()

func _on_main_menu_new_game_button_pressed():
	$OptionsMenu.toggle_locale_button_visibility(false)
	$LobbyMenu.show()
	$MainMenu.hide()

# REQUIREMENTS
# * SS-6
# * UIFUN-5
func _on_main_menu_options_button_pressed():
	$OptionsMenu.toggle_locale_button_visibility(false)
	$OptionsMenu.show()
	$MainMenu.hide()


func _on_options_menu_back_button_pressed():
	$MainMenu.show()
	$OptionsMenu.hide()
	$OptionsMenu.toggle_locale_button_visibility(true)


func _on_lobby_menu_back_button_pressed():
	$MainMenu.show()
	$LobbyMenu.hide()
	$OptionsMenu.toggle_locale_button_visibility(true)


func _on_credits_back_button_pressed():
	$Credits.hide()
	$MainMenu.show()


func _on_main_menu_credits_button_pressed():
	$Credits.show()
	$MainMenu.hide()



func _on_splash_container_splash_end():
	show()
