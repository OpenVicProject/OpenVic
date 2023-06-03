extends Resource
class_name SaveResource

signal file_flushed(path : String)
signal file_loaded
signal file_moved_to_trash
signal file_deleted
signal trash_moved
signal deleted

var save_name : StringName:
	get: return save_name
	set(v):
		save_name = v
		file.set_value("Save", "name", save_name)
		emit_changed()
var session_tag : StringName:
	get: return session_tag
	set(v):
		session_tag = v
		file.set_value("Save", "session_tag", v)
		emit_changed()
var file_path : String:
	get: return file_path
	set(v):
		file_path = v
		emit_changed()
var file : ConfigFile = ConfigFile.new()

func set_file_path(name : StringName, path : String):
	file_path = path
	save_name = name

func flush_save() -> Error:
	file_flushed.emit(file_path)
	var result := file.save(file_path)
	file.clear()
	return result

func load_save(path : String = file_path) -> Error:
	file_loaded.emit()
	var result := file.load(path)
	session_tag = file.get_value("Save", "session_tag", session_tag)
	if path != file_path:
		set_file_path(file.get_value("Save", "name", save_name), path)
	return result

func get_save_file_time() -> int:
	return FileAccess.get_modified_time(file_path)

func move_to_trash() -> Error:
	trash_moved.emit()
	file_moved_to_trash.emit()
	return OS.move_to_trash(file_path)

func delete() -> Error:
	deleted.emit()
	file_deleted.emit()
	return DirAccess.remove_absolute(file_path)
