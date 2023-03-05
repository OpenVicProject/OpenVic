extends ItemList

# REQUIREMENTS:
# * SS-18
# Called when the node enters the scene tree for the first time.
func _ready():
	var files = get_saved_files()
	print(files)
	for file in files:
		self.add_item(file.get_basename())


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass

func get_saved_files():
	var dir = DirAccess.open("res://src/SaveSystem") # Will store saves here for now refactor later
	var files = []
	
	if dir:
		dir.list_dir_begin()
		var file_name = dir.get_next()
		while file_name != "":
			if !dir.current_is_dir():
				print("Found file: " + file_name)
				if file_name.get_extension() == "ov2":
					files.append(file_name)
			file_name = dir.get_next()
	else:
		print("An error occurred when trying to access the path.")
		
	
	return files
