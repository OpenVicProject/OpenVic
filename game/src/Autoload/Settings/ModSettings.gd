@tool
extends "res://addons/kenyoni/app_settings/app_settings.gd"

const SETTINGS_FILE: String = "user://mods.cfg"

const MODS_LOAD_LIST := &"mods/load_list"

func _init() -> void:
	self.add_setting(Setting.new(MODS_LOAD_LIST, PackedStringArray())
		.set_validate_fn(_load_list_validate)
		.add_meta(&"type", TYPE_PACKED_STRING_ARRAY))

	if Engine.is_editor_hint(): return
	
	self.load()
	self.apply_all()

func _notification(what: int) -> void:
	if Engine.is_editor_hint(): return
	if what != NOTIFICATION_WM_CLOSE_REQUEST: return
	save()

func save() -> Error:
	return self.to_config().save(SETTINGS_FILE)

func load() -> Error:
	var cfg: ConfigFile = ConfigFile.new()
	var err: Error = cfg.load(SETTINGS_FILE)
	if err == Error.ERR_FILE_NOT_FOUND:
		return Error.OK
	if err != Error.OK:
		return err
	self.from_config(cfg)

	return Error.OK

func get_load_list() -> PackedStringArray:
	return get_setting(MODS_LOAD_LIST).value() as PackedStringArray

func _load_list_validate(_stg: Setting, val: Variant) -> bool:
	var result := true
	var array := val as PackedStringArray
	for path: String in array:
		result = result and (path.is_relative_path() or path.is_absolute_path())
	return result
