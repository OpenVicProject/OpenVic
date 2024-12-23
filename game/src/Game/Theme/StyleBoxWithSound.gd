## WARNING: This will not work with togglable UI elements, a special implementation is needed for them.
@tool
extends StyleBox
class_name StyleBoxWithSound

@export var style_box: StyleBox:
	get:
		return style_box
	set(v):
		style_box = v
		emit_changed()

@export var sound: AudioStream:
	get:
		return sound
	set(v):
		sound = v
		emit_changed()


func _get_draw_rect(rect: Rect2) -> Rect2:
	if style_box == null:
		return Rect2()
	return style_box._get_draw_rect(rect)


func _draw(to_canvas_item: RID, rect: Rect2) -> void:
	# This is a hack
	# Works fine for simple non-normal style cases
	# Normal styles being drawn immediately tho will trigger sound on startup
	# This would require further work to be applicable for release sounds
	# Is there any other reason aside from release sounds (might be useful for toggles?)
	# This should be fast enough to not cause draw issues
	if sound != null:
		SoundManager.play_effect_compat("click", sound)
	if style_box != null:
		style_box.draw(to_canvas_item, rect)
