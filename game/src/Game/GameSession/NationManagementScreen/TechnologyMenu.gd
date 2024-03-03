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
var tech_windows = generate_tech_windows()

#populate godot dictionaries from the simulation backend
var tech_folder_dict : Dictionary = GameSingleton.get_tech_folders()
var tech_area_dict : Dictionary = GameSingleton.get_tech_areas()
var tech_dict : Dictionary = GameSingleton.get_technologies()

#defines the selected tech UI so we arent running expensive get_node repeatedly
var selected_tech_ui : Control
var selected_tech_folder : String

func _ready() -> void:
	GameSingleton.gamestate_updated.connect(_update_info)
	add_child(country_technology)
	Events.NationManagementScreens.update_active_nation_management_screen.connect(_on_update_active_nation_management_screen)

	generate_interface()
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
	
	selected_tech_ui = get_node("/root/GameSession/Topbar/TechnologyMenu/country_technology/selected_tech_window")
	selected_tech_folder = "army_tech"
	populate_areas(selected_tech_folder)
	
	#sets initial status of progress bar
	get_node("/root/GameSession/Topbar/TechnologyMenu/country_technology/research_progress_name").text = "No research selected"
	get_node("/root/GameSession/Topbar/TechnologyMenu/country_technology/research_progress_category").text = ""
	
	var start_research_button : Button = get_button_from_nodepath(^"./country_technology/selected_tech_window/start")
	start_research_button.connect("pressed",Callable(self,"_tech_start_button_pressed").bind())
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
		populate_techs(selected_tech_folder)
		var date = GameSingleton.get_longform_date().right(4).to_int()
		var date2 = GameSingleton.get_longform_date()
		show()
	else:
		hide()
		
func generate_interface():
	#generate the initial UI elements
	for i in 5:
		var temp = folder_window.duplicate()
		temp.name = tech_folder_dict[i].identifier
		temp.get_child(0).connect("pressed",Callable(self,"_folder_button_pressed").bind(temp.name))
		temp.get_child(2).text = tech_folder_dict[i].identifier
		temp.get_child(5).text = "0/30"
		temp.position.x = 28+(194*i)
		temp.position.y = 55
		folder_windows.append(temp)
	for i in 5:
		var temp = tech_group.duplicate()
		temp.name = temp.name+str(i)
		temp.position.x = 28+(194*i)
		temp.position.y = 107
		tech_groups.append(temp)
		
func populate_areas(identifier):
	var offset
	match identifier:
		"army_tech":
			offset = 0
		"navy_tech":
			offset = 5
		"commerce_tech":
			offset = 10
		"culture_tech":
			offset = 15
		"industry_tech":
			offset = 20
	for i in 5:
		var node = get_node("/root/GameSession/Topbar/TechnologyMenu/country_technology/tech_group"+str(i))
		node.get_child(0).text = tech_area_dict[offset+i].identifier


func populate_techs(identifier):
	var offset
	match identifier:
		"army_tech":
			offset = 0
		"navy_tech":
			offset = 120
		"commerce_tech":
			offset = 30
		"culture_tech":
			offset = 60
		"industry_tech":
			offset = 90
	var shift = 0
	for x in 5:
		for y in 6:
			var node = get_node("/root/GameSession/Topbar/TechnologyMenu/country_technology/tech_window"+str(x)+"_"+str(y))
			node.get_child(1).text = tech_dict[offset+(shift+y)].identifier
			if(node.get_child(0).is_connected("pressed",Callable(self,"_tech_button_pressed").bind(tech_dict[offset+(shift+y)])) == false):
				node.get_child(0).connect("pressed",Callable(self,"_tech_button_pressed").bind(tech_dict[offset+(shift+y)]))
			if (GameSingleton.get_longform_date().right(4).to_int() >= tech_dict[offset+(shift+y)].year ):
				node.get_child(0).disabled = false
			else:
				node.get_child(0).disabled = true
		shift = shift + 6

func generate_tech_windows():
	var matrix = []
	for xi in range(5):
		var row = []
		for yi in range(6):
			var temp = tech_window.duplicate()
			temp.name = temp.name+str(xi)+"_"+str(yi)
			temp.position.x = 28+(194*xi)
			temp.position.y = 122+(40*yi)
			row.append(temp)
		matrix.append(row)
	return matrix
	
func _folder_button_pressed(selected_folder):
	selected_tech_folder = selected_folder
	populate_areas(selected_tech_folder)
	populate_techs(selected_tech_folder)
	#selected_tech_ui.get_child(1).text = "NO_TECH_SELECTED"
	
func _tech_button_pressed(item):
	selected_tech_ui.get_child(1).text = item.identifier
	selected_tech_ui.get_child(6).text = str(item.cost)
	selected_tech_ui.get_child(8).text = str(item.year)
	#selected_tech_ui.get_child(8).text = GameSingleton.get_longform_date().right(4)

func _tech_start_button_pressed():
	print("WIP")
	#get_node(^"./Topbar/TechnologyMenu/country_technology/research_progress_name").text = selected_tech_ui.get_child(1).text
	#get_node(^"./Topbar/TechnologyMenu/country_technology/research_progress_category").text = selected_tech_folder+", "+""
	
