extends Resource
class_name TextureSetting

@export
var texture : Texture2D:
	get: return texture
	set(v):
		texture = v
		emit_changed()
@export
var draw_center : bool = true:
	get: return draw_center
	set(v):
		draw_center = v
		emit_changed()

@export_group("Texture Margins", "texture_margin_")
@export
var texture_margin_left : float = 0:
	get: return texture_margin_left
	set(v):
		texture_margin_left = v
		emit_changed()
@export
var texture_margin_top : float = 0:
	get: return texture_margin_top
	set(v):
		texture_margin_top = v
		emit_changed()
@export
var texture_margin_right : float = 0:
	get: return texture_margin_right
	set(v):
		texture_margin_right = v
		emit_changed()
@export
var texture_margin_bottom : float = 0:
	get: return texture_margin_bottom
	set(v):
		texture_margin_bottom = v
		emit_changed()

@export_group("Expand Margins", "expand_margin_")
@export
var expand_margin_left : float = 0:
	get: return expand_margin_left
	set(v):
		expand_margin_left = v
		emit_changed()
@export
var expand_margin_top : float = 0:
	get: return expand_margin_top
	set(v):
		expand_margin_top = v
		emit_changed()
@export
var expand_margin_right : float = 0:
	get: return expand_margin_right
	set(v):
		expand_margin_right = v
		emit_changed()
@export
var expand_margin_bottom : float = 0:
	get: return expand_margin_bottom
	set(v):
		expand_margin_bottom = v
		emit_changed()

@export_group("Axis Stretch", "axis_stretch_")
@export
var axis_stretch_horizontal : RenderingServer.NinePatchAxisMode = RenderingServer.NINE_PATCH_STRETCH:
	get: return axis_stretch_horizontal
	set(v):
		axis_stretch_horizontal = v
		emit_changed()
@export
var axis_stretch_vertical : RenderingServer.NinePatchAxisMode = RenderingServer.NINE_PATCH_STRETCH:
	get: return axis_stretch_vertical
	set(v):
		axis_stretch_vertical = v
		emit_changed()

@export_group("Sub-Region", "region_")
@export
var region_rect : Rect2 = Rect2(0, 0, 0, 0):
	get: return region_rect
	set(v):
		region_rect = v
		emit_changed()

@export_group("Modulate", "modulate_")
@export
var modulate_color : Color = Color(1, 1, 1, 1):
	get: return modulate_color
	set(v):
		modulate_color = v
		emit_changed()

@export_group("Content Margins", "content_margin_")
@export
var content_margin_left : float = -1:
	get: return content_margin_left
	set(v):
		content_margin_left = v
		emit_changed()
@export
var content_margin_top : float = -1:
	get: return content_margin_top
	set(v):
		content_margin_top = v
		emit_changed()
@export
var content_margin_right : float = -1:
	get: return content_margin_right
	set(v):
		content_margin_right = v
		emit_changed()
@export
var content_margin_bottom : float = -1:
	get: return content_margin_bottom
	set(v):
		content_margin_bottom = v
		emit_changed()
