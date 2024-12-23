extends EditorExportPlugin

var _repo_hash: StringName = "0000000000000000000000000000000000000000"
var _repo_short_hash: StringName = "0000000"
var _repo_tag: StringName = "<tag missing>"
var _repo_release_name: StringName = "<release name missing>"


func _get_name():
	return "OpenVic-ReleaseExportEditorPlugin"


func _export_file(path: String, type: String, features: PackedStringArray) -> void:
	if path != "res://src/GIT_INFO.gd":
		return
	var GitInfoScript: String = ""
	_get_commit_long()
	_get_commit_short()
	_get_tag()
	_get_release_name()
	GitInfoScript = "class_name _GIT_INFO_\nextends RefCounted\n\n"
	GitInfoScript += 'const commit_hash : StringName = &"' + _repo_hash + '"\n'
	GitInfoScript += 'const short_hash : StringName = &"' + _repo_short_hash + '"\n'
	GitInfoScript += 'const tag : StringName = &"' + _repo_tag + '"\n'
	GitInfoScript += 'const release_name : StringName = &"' + _repo_release_name + '"\n'
	add_file(path, GitInfoScript.to_ascii_buffer(), false)
	skip()


# Based on
# https://github.com/godotengine/godot/blob/6ef2f358c741c993b5cdc9680489e2c4f5da25cc/methods.py#L102-L133
# REQUIREMENTS:
# * UIFUN-298
var _cached_hash: StringName = &""


func _get_commit_hash() -> StringName:
	if not _cached_hash.is_empty():
		return _cached_hash

	var git_hash := OS.get_environment("OPENVIC_COMMIT")
	if not git_hash.is_empty():
		_cached_hash = git_hash
		return git_hash

	var git_folder := "../.git"

	if FileAccess.file_exists(git_folder):
		var module_folder := FileAccess.open(git_folder, FileAccess.READ).get_line().strip_edges()
		if module_folder.begins_with("gitdir: "):
			git_folder = module_folder.substr(8)

	if FileAccess.file_exists(git_folder.path_join("HEAD")):
		var head := (
			FileAccess.open(git_folder.path_join("HEAD"), FileAccess.READ).get_line().strip_edges()
		)
		if head.begins_with("ref: "):
			var ref := head.substr(5)
			var parts := git_folder.split("/")
			if len(parts) > 2 and parts[len(parts) - 2] == "worktrees":
				git_folder = "/".join(parts.slice(0, len(parts) - 2))
			head = git_folder.path_join(ref)
			var packedrefs := git_folder.path_join("packed-refs")
			if FileAccess.file_exists(head):
				git_hash = FileAccess.open(head, FileAccess.READ).get_line().strip_edges()
			elif FileAccess.file_exists(packedrefs):
				for line in FileAccess.open(packedrefs, FileAccess.READ).get_as_text().split(
					"\n", false
				):
					if line.begins_with("#"):
						continue
					var line_split := line.split(" ")
					var line_hash := line_split[0]
					var line_ref = line_split[1]
					if ref == line_ref:
						git_hash = line_hash
						break
		else:
			git_hash = head

	_cached_hash = git_hash

	return git_hash


# REQUIREMENTS:
# * UIFUN-296
func _try_get_tag() -> StringName:
	var result: StringName = OS.get_environment("OPENVIC_TAG")
	if result.is_empty():
		var git_output := []
		if OS.execute("git", ["describe", "--tags", "--abbrev=0"], git_output) == -1:
			push_warning("Could not retrieve repository tag.")
			return &""
		result = git_output[0].trim_suffix("\n")
	return result


func _get_commit_long():
	var result := _get_commit_hash()
	if not result.is_empty():
		_repo_hash = result
	print("Hash: " + _repo_hash)


# REQUIREMENTS:
# * UIFUN-300
func _get_commit_short():
	var result := _get_commit_hash().substr(0, 7)
	if not result.is_empty():
		_repo_short_hash = result
	print("Short Hash: " + _repo_short_hash)


func _get_tag():
	var result := _try_get_tag()
	if not result.is_empty():
		_repo_tag = result
	print("Tag: " + _repo_tag)


# REQUIREMENTS:
# * UIFUN-295
func _get_release_name():
	var result: StringName = OS.get_environment("OPENVIC_RELEASE")
	if result.is_empty():
		result = _try_get_tag()
	if not result.is_empty():
		_repo_release_name = result
	print("Release Name: " + _repo_release_name)
