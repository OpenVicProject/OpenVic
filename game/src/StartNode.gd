extends Node

func _ready():
	print("From GDScript")
	TestSingleton.hello_singleton()
