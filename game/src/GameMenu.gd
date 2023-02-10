extends Control


func _on_main_menu_options_button_pressed():
	$OptionsMenu.toggle_locale_button_visibility(false)
	$OptionsMenu.show()
	$MainMenu.hide()


func _on_options_menu_back_button_pressed():
	$MainMenu.show()
	$OptionsMenu.hide()
	$OptionsMenu.toggle_locale_button_visibility(true)
