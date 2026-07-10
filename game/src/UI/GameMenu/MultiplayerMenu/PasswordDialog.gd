class_name PasswordDialog
extends ConfirmationDialog

signal pword_confirmed(password: String)

@export var secret: SecretEdit


func _on_confirmed() -> void:
	pword_confirmed.emit(secret.get_text())


func clear_and_display() -> void:
	secret.set_text("")
	popup_centered()
