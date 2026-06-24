extends RefCounted
## Container that owns and manages a collection of `Setting` objects.

const Setting := preload("res://addons/kenyoni/app_settings/setting.gd")

## Emitted immediately after a setting's `apply()` completes. `key` is the key of the applied setting.
signal applied(key: StringName)
## Emitted when a setting's effective value changes, either by direct assignment or when a staged value is committed. `key` is the key of the changed setting.
signal changed(key: StringName)
## Emitted when a setting's staged value is set or cleared. `key` is the key of the affected setting.
signal staged_changed(key: StringName)

var _settings: Dictionary[StringName, Setting] = {}

## Register `setting` with this registry and sets its internal registry reference to `self`.
##
## Fails with a push_error if:
## - The key ends with `/` or contains `//`.
## - The setting is already registered with a different registry.
## - A setting with the same key already exists in this registry.
func add_setting(setting: Setting) -> void:
	if setting.key().ends_with("/") || setting.key().contains("//"):
		push_error("Setting keys should not end with a slash or contain an empty section.")
		return
	if setting._registry != null && setting._registry != self:
		push_error("Setting with key '%s' already belongs to another registry.".format(setting.key()))
		return
	if self.has_setting(setting.key()):
		push_error("Setting with key '%s' already exists.".format(setting.key()))
		return
	setting._registry = self
	self._settings[setting.key()] = setting

## Return `true` if a setting with `key` exists in this registry.
func has_setting(key: StringName) -> bool:
	return self._settings.has(key)

## Return the `Setting` for `key`, or `null` if no such setting exists.
func get_setting(key: StringName) -> Setting:
	return self._settings.get(key, null)

## Remove the setting identified by `key`. Does nothing if the key does not exist.
func remove_setting(key: StringName) -> void:
	var setting: Setting = self.get_setting(key)
	if setting != null:
		self._settings.erase(key)

## Return all `Setting` objects whose keys begin with `section`.
##
## `section` is a key prefix; pass an empty string to match all settings.
## `depth` limits how many additional `/`-separated levels below `section` are included; `-1` means unlimited.
## `filter` is a `Callable` with signature `func(setting: Setting) -> bool`; only settings for which it returns `true` are included. Defaults to `_exclude_internal`.
func get_section(section: String, depth: int = -1, filter: Callable = _exclude_internal) -> Array[Setting]:
	assert(!section.ends_with("/"), "key should not end with a slash")
	
	var max_level: int = -1
	if depth != -1:
		max_level = section.count("/") + depth
	if section != "":
		section += "/"

	var settings: Array[Setting] = []
	for key: StringName in self._settings:
		if max_level != -1 && key.count("/") > max_level:
			continue

		if !key.begins_with(section):
			continue

		var setting: Setting = self._settings[key]
		if filter.is_valid() && !filter.call(setting):
			continue
		
		settings.append(setting)
	
	return settings

## Return the keys of all settings whose keys begin with `section`.
##
## `section` is a key prefix; pass an empty string to match all settings.
## `depth` limits how many additional `/`-separated levels below `section` are included; `-1` means unlimited.
## `filter` is a `Callable` with signature `func(setting: Setting) -> bool`; only matching settings contribute keys. Defaults to `_exclude_internal`.
func get_section_keys(section: String, depth: int = -1, filter: Callable = _exclude_internal) -> PackedStringArray:
	assert(!section.ends_with("/"), "key should not end with a slash")
	
	var max_level: int = -1
	if depth != -1:
		max_level = section.count("/") + depth
	if section != "":
		section += "/"

	var keys: PackedStringArray = []
	for key: StringName in self._settings:
		if max_level != -1 && key.count("/") > max_level:
			continue

		if !key.begins_with(section):
			continue

		if filter.is_valid() && !filter.call(self._settings[key]):
			continue
		
		keys.append(key)

	return keys

## Return the names of the immediate child sections under `parent_section`. The names order is not guaranteed to be stable.
##
## A child section name is the single path component that follows `parent_section` in a matching key.
## For example, given a key `"graphics/display/vsync"` and `parent_section = "graphics"`, this returns `["display"]`.
## Pass an empty string for `parent_section` to get the top-level section names.
## `filter` is a `Callable` with signature `func(setting: Setting) -> bool`.
func get_sub_sections(parent_section: String = "", filter: Callable = _exclude_internal) -> PackedStringArray:
	assert(!parent_section.ends_with("/"), "key should not end with a slash")
	var sub_section_level: int = parent_section.count("/")
	if parent_section != "":
		sub_section_level += 1
	
	if parent_section != "":
		parent_section += "/"

	var seen: Dictionary[String, bool] = {}
	for key: StringName in self._settings:
		if key.count("/") <= sub_section_level || !key.begins_with(parent_section):
			continue
		
		var setting: Setting = self._settings[key]
		if filter.is_valid() && !filter.call(setting):
			continue

		seen[key.get_slice("/", sub_section_level)] = true

	return PackedStringArray(seen.keys())

## Set the value of the setting identified by `key`.
## Emits a warning if `key` does not exist. Staged mode, readonly mode, and validation all apply.
func set_value(key: StringName, value: Variant) -> void:
	var setting: Setting = self.get_setting(key)
	if setting == null:
		push_warning("Setting with key '%s' not found."%key)
		return
	setting.set_value(value)

## Return the current effective value of the setting identified by `key`.
## Emits a warning and returns `null` if `key` does not exist.
func get_value(key: StringName) -> Variant:
	var setting: Setting = self.get_setting(key)
	if setting == null:
		push_warning("Setting with key '%s' not found."%key)
		return null
	return setting.value()

## Call `apply()` on every registered setting unconditionally.
func apply_all() -> void:
	for setting: Setting in self._settings.values():
		setting.apply()

## Call `apply()` only on settings that have a pending staged value.
func apply_staged_values() -> void:
	for setting: Setting in self._settings.values():
		if setting._has_staged_value:
			setting.apply()

## Discard all pending staged values. `staged_changed` is emitted for each setting that had a staged value.
func discard_staged_values() -> void:
	for setting: Setting in self._settings.values():
		if setting.has_staged_value():
			setting.discard_staged_value()

## Return `true` if at least one registered setting has a pending staged value.
func has_staged_values() -> bool:
	for setting: Setting in self._settings.values():
		if setting.has_staged_value():
			return true
	return false

## Load values from `config` into matching settings.
##
## For each `section`/`key` pair in the `ConfigFile`, the corresponding setting key is constructed as `"section/key"` and looked up in the registry.
## Only settings that exist, are exported, and are not readonly are updated.
## Unknown keys emit a warning.
func from_config(config: ConfigFile) -> void:
	for section: String in config.get_sections():
		for key: String in config.get_section_keys(section):
			var setting: Setting = self.get_setting(section.path_join(key))
			if setting == null:
				push_warning("Setting '%s/%s' not found.".format(section, key))
				continue
			# checking readonly is duplicated here, but it might be possible that set_value might push a warning when trying to set a readonly setting
			if setting.is_exported() && !setting.is_readonly():
				setting.set_value(config.get_value(section, key))

## Serialize settings to a `ConfigFile`.
##
## Each setting's key is split on the last `/`: the left part becomes the `ConfigFile` section and the right part becomes the config key. Settings without a `/` in their key are placed in the empty-string section.
## `filter` is a `Callable` with signature `func(setting: Setting) -> bool` that controls which settings are included. Defaults to `_include_exported`.
func to_config(filter: Callable = _include_exported) -> ConfigFile:
	var config: ConfigFile = ConfigFile.new()
	for key: StringName in self._settings:
		var setting: Setting = self.get_setting(key)
		if filter.is_valid() && !filter.call(setting):
			continue
		if key.count("/") > 0:
			var keys: PackedStringArray = key.rsplit("/", true, 1)
			config.set_value(keys[0], keys[1], setting.value())
		else:
			config.set_value("", key, setting.value())
	return config

## Return `true` for non-internal settings.
static func _exclude_internal(setting: Setting) -> bool:
	return !setting.is_internal()

## Return `true` for exported settings.
static func _include_exported(setting: Setting) -> bool:
	return setting.is_exported()
