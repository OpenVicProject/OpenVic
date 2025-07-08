extends Control

@export var quote_file_path : String = "res://assets/localisation/quotes.txt"

@export_subgroup("Nodes")
@export var progress_bar: ProgressBar
@export var quote_label: Label
@export var animation_player: AnimationPlayer

var thread: Thread
var quotes: PackedStringArray = []

func start_loading_screen(thread_safe_function : Callable) -> void:
	if not is_node_ready():
		await ready
	# set first quote
	progress_bar.value = 0
	if quotes.size() > 0:
		quote_label.text = quotes[randi() % quotes.size()]
	else:
		quote_label.text = "NO QUOTES DEFINED!"

	if thread != null and thread.is_started():
		thread.wait_to_finish()

	thread.start(thread_safe_function)

func try_update_loading_screen(percent_complete: float, quote_should_change := false) -> void:
	# forces the function to behave as if deferred
	await get_tree().process_frame
	progress_bar.value = percent_complete
	if quote_should_change and quotes.size() > 0:
		quote_label.text = quotes[randi() % quotes.size()]

func _ready() -> void:
	if Engine.is_editor_hint(): return
	thread = Thread.new()
	# FS-3, UI-30, UIFUN-35
	var quotes_file := FileAccess.open(quote_file_path, FileAccess.READ).get_as_text()
	quotes = quotes_file.split("\n", false)
	if quotes.is_empty():
		quotes = [""]
	animation_player.play("loadingscreen_gear")

func _exit_tree() -> void:
	if thread != null and thread.is_started():
		thread.wait_to_finish()
