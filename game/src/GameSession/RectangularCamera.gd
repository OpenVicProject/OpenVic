extends Control


@export var pos_x = 100.0
@export var pos_y = 100.0
@export var zoom_x = 10
@export var zoom_y = 10
@onready var _frame: TextureRect = $"../TextureRect"

var RectangularCamera = Rect2(pos_x,pos_y,zoom_x,zoom_y)

func _draw() -> void:
	draw_rect(RectangularCamera,Color.BLACK,false,-1)

func _on_map_view_map_view_camera_change(camera_position):

	zoom_x = camera_position.y*(_frame.size.x/20)*1.77
	zoom_y = camera_position.y*(_frame.size.y/20)*1.77
	
	pos_x = camera_position.x*(_frame.size.x/20) + _frame.size.x/2 - zoom_x/2 # + _frame.size.x/2 - zoom_x/2 - Translation from camera_position.x to pos_x (i.e. 0px for center on camera_pos -> 175px for center on rect_camera)
	pos_y = camera_position.z*(_frame.size.y/10) + _frame.size.y/2 - zoom_y/2 # + _frame.size.y/2 - zoom_y/2 - Translation from camera_position.y to pos_y (i.e. 0px for center on camera_pos -> 87.5px for center on rect_camera)

	RectangularCamera = Rect2(pos_x,pos_y,zoom_x,zoom_y)
	queue_redraw()
