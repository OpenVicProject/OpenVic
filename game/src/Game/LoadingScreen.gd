extends Control

signal load_started()
signal load_changed(percentage : float)
signal load_ended()

@export var quote_file_path : String = "res://common/quotes.txt"

@export_subgroup("Nodes")
@export var progress_bar: ProgressBar
@export var quote_label: Label

var thread: Thread
var quotes: PackedStringArray = []

func start_loading_screen(thread_safe_function : Callable) -> void:
	if not is_node_ready():
		await ready
	# set first quote
	progress_bar.value = 0
	quote_label.text = quotes[randi() % quotes.size()]

	if thread != null and thread.is_started():
		thread.wait_to_finish()

	thread.start(thread_safe_function)
	load_started.emit()

func try_update_loading_screen(percent_complete: float, quote_should_change = false):
	# forces the function to behave as if deferred
	await get_tree().process_frame
	progress_bar.value = percent_complete
	if quote_should_change:
		quote_label.text = quotes[randi() % quotes.size()]
	if is_equal_approx(percent_complete, 100):
		thread.wait_to_finish()
		load_ended.emit()
	else:
		load_changed.emit(percent_complete)

func _ready():
	if Engine.is_editor_hint(): return
	thread = Thread.new()
	# FS-3, UI-30, UIFUN-35
	var quotes_file := FileAccess.open(quote_file_path, FileAccess.READ).get_as_text()
	quotes = quotes_file.split("\n",false)
	if quotes.is_empty():
		quotes = [""]

func _exit_tree():
	if thread != null and thread.is_started():
		thread.wait_to_finish()
