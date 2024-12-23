extends Node

# Requirements
# * FS-28
const save_directory_setting := &"openvic/data/saves_directory"

var current_save: SaveResource
var current_session_tag: StringName

var _save_dictionary: Dictionary = {}
var _dirty_save: SaveResource


func _ready() -> void:
	var saves_dir_path: String = ProjectSettings.get_setting_with_override(save_directory_setting)
	assert(saves_dir_path != null, "'%s' setting could not be found." % save_directory_setting)

	DirAccess.make_dir_recursive_absolute(saves_dir_path)
	var saves_dir := DirAccess.open(saves_dir_path)
	for file: String in saves_dir.get_files():
		var save := SaveResource.new()
		save.load_save(saves_dir_path.path_join(file))
		add_or_replace_save(save, true)


func get_save_file_name(
	save_name: StringName, session_tag: StringName = current_session_tag
) -> StringName:
	return ("%s - %s" % [save_name, session_tag]).validate_filename()


func make_new_save(
	save_name: String, session_tag: StringName = current_session_tag
) -> SaveResource:
	var file_name := get_save_file_name(save_name, session_tag) + ".tres"
	var new_save := SaveResource.new()
	new_save.set_file_path(
		save_name,
		ProjectSettings.get_setting_with_override(save_directory_setting).path_join(file_name)
	)
	print(new_save.file_path)
	new_save.session_tag = session_tag
	return new_save


func has_save(save_name: StringName, session_tag: StringName = current_session_tag) -> bool:
	return _save_dictionary.has(get_save_file_name(save_name, session_tag))


func add_or_replace_save(save: SaveResource, ignore_dirty: bool = false) -> void:
	var binded_func := _on_save_deleted_or_moved.bind(save)
	save.deleted.connect(binded_func)
	save.trash_moved.connect(binded_func)
	_save_dictionary[get_save_file_name(save.save_name, save.session_tag)] = save
	if not ignore_dirty:
		_dirty_save = save


func delete_save(save: SaveResource) -> void:
	save.delete()


func flush_save() -> void:
	if _dirty_save == null:
		return
	_dirty_save.flush_save()
	_dirty_save = null


func _on_save_deleted_or_moved(save: SaveResource) -> void:
	_save_dictionary.erase(get_save_file_name(save.save_name, save.session_tag))
