extends Control

signal splash_end

@export var _splash_finish : TextureRect
@export var _splash_image : TextureRect
@export var _splash_video : VideoStreamPlayer

func _process(_delta):
	var stream_texture := _splash_video.get_video_texture()
	if stream_texture != null and not stream_texture.get_image().is_invisible():
		_splash_image.hide()
		_splash_finish.show()
		set_process(false)

func _input(event):
	if (event is InputEventKey\
		or event is InputEventMouse\
		or event is InputEventScreenTouch\
		or event is InputEventJoypadButton) and event.is_pressed():
		_splash_finish.hide()
		_on_splash_startup_finished()
		accept_event()

func _on_splash_startup_finished():
	set_process_input(false)
	splash_end.emit()
	var tween := create_tween()
	tween.tween_property(self, "modulate:a", 0, 0.5)
	tween.tween_callback(self.queue_free)
