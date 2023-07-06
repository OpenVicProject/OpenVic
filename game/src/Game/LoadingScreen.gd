extends Control
class_name LoadingScreen

@export var progress_bar: ProgressBar
@export var quote_label: Label

var loadthread: Thread
var quotes: PackedStringArray = []

func update_loading_screen(percent_complete: int, quote_should_change = false):
	# forces the function to behave as if deferred
	await get_tree().process_frame
	progress_bar.value = percent_complete
	if quote_should_change:
		quote_label.text = quotes[randi() % quotes.size()]

func _on_splash_container_splash_end():
	show()

func _ready():
	# FS-3, UI-30, UIFUN-35
	loadthread = Thread.new()
	loadthread.start(Events.load_events.bind(self))
	var quotes_file = FileAccess.open("res://common/quotes.txt", FileAccess.READ).get_as_text()
	quotes = quotes_file.split("\n",false)
	if quotes.is_empty():
		quotes = [""]
	# set first quote
	quote_label.text = quotes[randi() % quotes.size()]

func _exit_tree():
	loadthread.wait_to_finish()
