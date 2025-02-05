extends PanelContainer

signal back_button_pressed

@export var _tab_container : TabContainer
@export var _password_dialog : PasswordDialog
@export var _connection_fail_dialog : ConnectionFailDialog

@export var _host_tab : HostTab
@export var _direct_connection_tab : DirectConnectionTab

var last_ip : StringName = &""
var last_player_name : StringName = &""
var last_validated_ip : PackedInt32Array = []

var last_game_name : StringName = &""
var last_game_password : StringName = &""

@onready var MP : MultiplayerEventsObject = Events.Multiplayer

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	#Server browser not implemented yet
	_tab_container.set_tab_title(0, "MP_DIRECT_CONNECTION")
	_tab_container.set_tab_title(1, "MP_HOST")

	# Prepare options menu before loading user settings
	var tab_bar : TabBar = _tab_container.get_child(0, true)
	# This ends up easier to manage then trying to manually recreate the TabContainer's behavior
	# These buttons can be accessed regardless of the tab
	var button_list := HBoxContainer.new()
	button_list.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	button_list.alignment = BoxContainer.ALIGNMENT_END
	tab_bar.add_child(button_list)
	
	var back_button := Button.new()
	back_button.text = "MP_BACK"
	back_button.pressed.connect(_on_back_button_pressed)
	button_list.add_child(back_button)
	get_viewport().get_window().close_requested.connect(_on_window_close_requested)
	load_saved_ips()

func _input(event : InputEvent) -> void:
	if is_visible_in_tree():
		if event.is_action_pressed("ui_cancel"):
			_on_back_button_pressed()

func _on_back_button_pressed() -> void:
	last_game_name = _host_tab.get_game_name()
	last_game_password = _host_tab.get_password()
	_save_user(last_player_name,last_game_name,last_game_password)
	_direct_connection_tab._on_save_ips_pressed()
	back_button_pressed.emit()

func _on_window_close_requested() -> void:
	last_game_name = _host_tab.get_game_name()
	last_game_password = _host_tab.get_password()
	_save_user(last_player_name,last_game_name,last_game_password)
	_direct_connection_tab._on_save_ips_pressed()

func _notification(what : int) -> void:
	match what:
		NOTIFICATION_CRASH:
			_on_window_close_requested()

func join_game(ip : String, player_name : String) -> bool:
	var success : bool = false
	
	last_ip = ip
	last_player_name = player_name
	
	var ip_segments : PackedInt32Array = validate_ipv4(ip)
	print(ip_segments)
	if(ip_segments.size() != 4 && ip_segments.size() != 5):
		_connection_fail_dialog.display(ConnectionFailDialog.FAIL_REASONS.BAD_IP)
		return false
	last_validated_ip = ip_segments
	
	#TODO: Send a request to the host asking if a password is needed
	var needs_password : bool = check_password_needed(ip)
	#TODO: if the connection fails, show the appropriate fail dialog
	
	if needs_password:
		#Let the password dialog call the connect to game function
		_password_dialog.clear_and_display()
	else:
		connect_to_game("")
	return success

func check_password_needed(ip : String) -> bool:
	#TODO: perform the check
	return true

#This function continues from join_game after the player
#has entered a password in the password dialog
func connect_to_game(password : String) -> bool:
	#Use last_ip and last_player_name
	var success : bool = false
	print("Connecting to a game: IP: %s, Username: %s, Password: %s" % [last_ip,last_player_name,password])
	#TODO: Connect, use last_validated_ip as the ip
	
	#TODO: if successful, move to the game lobby. Otherwise, show the appropriate fail dialog.
	if success:
		return true
	else:
		#TODO: replace this with actual error handling
		if password.is_empty():
			_connection_fail_dialog.display(ConnectionFailDialog.FAIL_REASONS.PASSWORD)
		else:
			_connection_fail_dialog.display(ConnectionFailDialog.FAIL_REASONS.NO_SERVER)
		return false

#cg0: 127.0.0.1:80, cg1 = 123.456.789/localhost, cg2=123... only, cg3=localhost only, cg4=:8052, cg5=8052
static var IP_PATTERN : StringName = &"^((\\d{1,3}.\\d{1,3}.\\d{1,3}.\\d{1,3})|(localhost))(:(\\d+))?$"
static var IP_REGEX : RegEx = RegEx.new()

func _init() -> void:
	if IP_REGEX.compile(IP_PATTERN) != OK:
		push_error("Invalid Regex expression used in IP validation pattern IP_PATTERN")

#returns the subroutes and port in an integer array
func validate_ipv4(ip : String) -> PackedInt32Array:
	var result : RegExMatch = IP_REGEX.search(ip)
	if result == null:
		return []
	var ret : PackedInt32Array = []
	if !result:
		return ret
	var domain : StringName = result.get_string(1)
	if domain == &"localhost":
		ret = [127,0,0,1]
	else:
		var split : PackedStringArray = domain.split(".")
		for val in split:
			ret.push_back(int(val))
	
	if ret.size() == 4:
		var port : String = result.get_string(5)
		if !port.is_empty():
			ret.push_back(int(port))
	
	return ret

func host_game(player_name : String, game_name : String, password : String = "", lan_only : bool = false) -> bool:
	#TODO: Check if connections to us are possible?
	var success : bool = true
	print("Host game with Name: %s, password: %s, Player: %s, Is lan only %s" % [game_name,password,player_name, lan_only])
	#TODO: Go to the lobby
	return success

func load_saved_ips() -> void:
	var file : ConfigFile = MP.get_ips_config_file()

	var player_name : StringName = file.get_value(MP.USER,MP.PLAYER_NAME, MP.DEFAULT_PLAYER_NAME)
	var game_name : StringName = file.get_value(MP.USER,MP.HOST_GAME_NAME, MP.DEFAULT_GAME_NAME)
	var game_password : StringName = file.get_value(MP.USER,MP.HOST_GAME_PASSWORD, MP.DEFAULT_GAME_PASSWORD)

	last_player_name = player_name
	last_game_name = game_name
	last_game_password = game_password

	_host_tab.set_user_values(player_name,game_name,game_password)
	_direct_connection_tab.initial_setup(player_name)

	if !file.has_section(MP.SERVER_NAMES):
		return

	var indices : PackedStringArray = file.get_section_keys(MP.SERVER_NAMES)
	#if the file is malformed, we will just be missing some entries
	for index : String in indices:
		var entry_name : String = file.get_value(MP.SERVER_NAMES,index,&"")
		var entry_ip : String = file.get_value(MP.SERVER_IPS,index,&"")
		_direct_connection_tab.add_ip_entry_to_list(entry_name,entry_ip)

func _save_ips(names : PackedStringArray, ips : PackedStringArray) -> void:
	assert(names.size() == ips.size())
	var file : ConfigFile = MP.get_ips_config_file()
	if file.has_section(MP.SERVER_NAMES):
		file.erase_section(MP.SERVER_NAMES)
	if file.has_section(MP.SERVER_IPS):
		file.erase_section(MP.SERVER_IPS)
	
	for i : int in names.size():
		var index_str : String = String.num_int64(i)
		file.set_value(MP.SERVER_NAMES, index_str, names[i])
		file.set_value(MP.SERVER_IPS, index_str, ips[i])
	MP.save_ips_config_file()
	
func _save_user(player_name : String, game_name : String, game_password : String) -> void:
	var file : ConfigFile = MP.get_ips_config_file()
	file.set_value(MP.USER,MP.PLAYER_NAME,player_name)
	file.set_value(MP.USER,MP.HOST_GAME_NAME,game_name)
	file.set_value(MP.USER,MP.HOST_GAME_PASSWORD,game_password)
	MP.save_ips_config_file()
	
func _on_direct_connection_tab_revert_ips() -> void:
	last_game_name = _host_tab.get_game_name()
	last_game_password = _host_tab.get_password()
	load_saved_ips()
	_direct_connection_tab.set_player_name(last_player_name)
	_host_tab.set_player_name(last_player_name)
	_host_tab.set_game_name(last_game_name)
	_host_tab.set_game_password(last_game_password)

func update_player_name(player_name : String) -> void:
	last_player_name = player_name

#Make sure the player name entry is updated on the new tab
func _on_tab_changed(tab : int) -> void:
	_direct_connection_tab.set_player_name(last_player_name)
	_host_tab.set_player_name(last_player_name)
