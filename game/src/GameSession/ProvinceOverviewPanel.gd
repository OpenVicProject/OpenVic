extends Panel
@export var ProvinceID: String = "ID not loaded"

# Called when the node enters the scene tree for the first time.
func _ready():
	set_id()

func set_id():
	$VBoxContainer/ProvinceName.text = str(ProvinceID)+"_NAME"


func _on_button_pressed():
	queue_free()
