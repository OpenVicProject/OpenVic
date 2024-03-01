extends GUINode

var _active : bool = false

const _screen : NationManagement.Screen = NationManagement.Screen.TECHNOLOGY
#generates the necessary base elements
var country_technology : Control = generate_gui_element("country_technology", "country_technology")
var folder_window : Control = generate_gui_element("country_technology", "folder_window")
var folder_windows = []
var tech_group : Control = generate_gui_element("country_technology", "tech_group")
var tech_groups = []
var tech_window : Control = generate_gui_element("country_technology", "tech_window")
var tech_windows = generate_tech_windows(5,6)



func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)
	add_child(country_technology)
	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)
	generate_interface()
	#in vic2, army tech is the initial window selected
	var selected_tech_window = folder_windows[0]
	#setup the window since the gui file doesnt store positions, can probably be optimised
	for folder_item in folder_windows:
		country_technology.add_child(folder_item)
	folder_windows.clear()
	for tech_group_item in tech_groups:
		country_technology.add_child(tech_group_item)
	tech_groups.clear()
	for research_item_column in tech_windows:
		for research_item_row in research_item_column:
			country_technology.add_child(research_item_row)
	tech_windows.clear()
	

	var close_button : Button = get_button_from_nodepath(^"./country_technology/close_button")
	if close_button:
		close_button.pressed.connect(Events.NationManagementScreens.close_nation_management_screen.bind(_screen))

	_update_info()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_TRANSLATION_CHANGED:
			_update_info()

func _on_update_active_nation_management_screen(active_screen : NationManagement.Screen) -> void:
	_active = active_screen == _screen
	_update_info()

func _update_info() -> void:
	if _active:
		
		show()
	else:
		hide()
		
func generate_interface():
	#generate the initial UI elements
	for i in 5:
		var temp = folder_window.duplicate()
		temp.name = temp.name+str(i)
		temp.position.x = 28+(194*i)
		temp.position.y = 55
		folder_windows.append(temp)
	for i in 5:
		var temp = tech_group.duplicate()
		temp.name = temp.name+str(i)
		temp.position.x = 28+(194*i)
		temp.position.y = 107
		tech_groups.append(temp)

func generate_tech_windows(x,y):
	var matrix = []
	for xi in range(x):
		var row = []
		for yi in range(y):
			var temp = tech_window.duplicate()
			temp.name = temp.name+str(xi)+"_"+str(yi)
			temp.position.x = 28+(194*xi)
			temp.position.y = 122+(40*yi)
			row.append(temp)
		matrix.append(row)
	return matrix
	
