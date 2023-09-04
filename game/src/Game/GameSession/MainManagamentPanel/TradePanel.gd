extends Button
var PopPanel = load("res://src/Game/GameSession/MainManagamentPanel/PopupPanel.tscn")
 
func _on_pressed():
	var spawn = PopPanel.instantiate()
	spawn.PanelName = "EXAMPLE_POPUP"
	$"../../..".add_child(spawn) # Spawning window on MainManagamentPanel
