extends Node

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass

# Called to Save the Game into a file when button is pressed
static func save_game():
	print("Calling save_game()")
	var save_game = FileAccess.open("res://src/SaveSystem/savegame.ov2", FileAccess.WRITE)
	save_game.store_line("Hello, World")
	
# Should insert a load_game() function
