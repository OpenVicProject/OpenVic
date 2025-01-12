extends ConfirmationDialog
class_name PasswordDialog

@export var secret : SecretEdit

signal pword_confirmed(password : String)

func _on_confirmed() -> void:
	pword_confirmed.emit(secret.get_text())

func clear_and_display() -> void:
	secret.set_text("")
	popup_centered()
