extends Node
## Node that wraps a `Registry` and adds per-frame batched signals.

const Registry := preload("res://addons/kenyoni/app_settings/registry.gd")
const Setting := preload("res://addons/kenyoni/app_settings/setting.gd")

## Emitted once per frame when one or more settings were applied during that frame. Inspect `get_changed_applied_settings()` for the affected keys.
signal settings_applied()
## Emitted once per frame when one or more effective setting values changed during that frame. Inspect `get_changed_settings()` for the affected keys.
signal settings_changed()
## Emitted once per frame when one or more staged values were set or cleared during that frame. Inspect `get_changed_staged_settings()` for the affected keys.
signal settings_staged_changed()

## Emitted immediately after a setting is applied. `key` is the key of the applied setting.
signal applied(key: StringName)
## Emitted immediately when a setting's effective value changes. `key` is the key of the changed setting.
signal changed(key: StringName)
## Emitted immediately when a staged value is set or cleared. `key` is the key of the affected setting.
signal staged_changed(key: StringName)

var _registry: Registry = Registry.new()

## Keys of settings that were applied at least once this frame.
var _settings_applied: PackedStringArray = []
## Keys of settings whose effective value changed at least once this frame.
var _settings_changed: PackedStringArray = []
## Keys of settings whose staged value changed or was cleared at least once this frame.
var _settings_staged_changed: PackedStringArray = []

func _ready() -> void:
	self._registry.applied.connect(self._on_applied)
	self._registry.changed.connect(self._on_changed)
	self._registry.staged_changed.connect(self._on_staged_changed)

func _process(_delta: float) -> void:
	if !self._settings_staged_changed.is_empty():
		self.settings_staged_changed.emit()
		self._settings_staged_changed = []
	if !self._settings_changed.is_empty():
		self.settings_changed.emit()
		self._settings_changed = []
	if !self._settings_applied.is_empty():
		self.settings_applied.emit()
		self._settings_applied = []

## Add a new setting. The setting's key must be unique.
func add_setting(setting: Setting) -> void:
	self._registry.add_setting(setting)

## Return `true` if a setting with `key` exists.
func has_setting(key: StringName) -> bool:
	return self._registry.has_setting(key)

## Return the `Setting` for `key`, or `null` if it does not exist.
func get_setting(key: StringName) -> Setting:
	return self._registry.get_setting(key)

## Remove the setting identified by `key`. Does nothing if the key does not exist.
func remove_setting(key: StringName) -> void:
	self._registry.remove_setting(key)

## Set the value of the setting identified by `key`. See `Setting.set_value()` for details on how values are assigned and validated.
func set_value(key: StringName, value: Variant) -> void:
	self._registry.set_value(key, value)

## Return the effective value of the setting identified by `key`. See `Setting.value()` for details on how values are determined.
func get_value(key: StringName) -> Variant:
	return self._registry.get_value(key)

## Return all `Setting` objects whose keys begin with `section`.
## `depth` limits the number of `/`-separated levels below `section` that are included; `-1` means unlimited.
## `filter` is a `Callable` with signature `func(setting: Setting) -> bool`. Defaults to `Registry._exclude_internal`.
## See `Registry.get_section()` for full details.
func get_section(section: String, depth: int = -1, filter: Callable = Registry._exclude_internal) -> Array[Setting]:
	return self._registry.get_section(section, depth, filter)

## Return the keys of all settings whose keys begin with `section`.
## `depth` limits the number of `/`-separated levels below `section` that are included; `-1` means unlimited.
## `filter` is a `Callable` with signature `func(setting: Setting) -> bool`. Defaults to `Registry._exclude_internal`.
## See `Registry.get_section_keys()` for full details.
func get_section_keys(section: String, depth: int = -1, filter: Callable = Registry._exclude_internal) -> PackedStringArray:
	return self._registry.get_section_keys(section, depth, filter)

## Return the names of the immediate child sections under `parent_section`.
## Pass an empty string for `parent_section` to get the top-level section names.
## `filter` is a `Callable` with signature `func(setting: Setting) -> bool`. Defaults to `Registry._exclude_internal`.
## See `Registry.get_sub_sections()` for full details.
func get_sub_sections(parent_section: String = "", filter: Callable = Registry._exclude_internal) -> PackedStringArray:
	return self._registry.get_sub_sections(parent_section, filter)

## Call `apply()` on every registered setting unconditionally.
func apply_all() -> void:
	self._registry.apply_all()

## Call `apply()` only on settings that have a pending staged value.
func apply_staged_values() -> void:
	self._registry.apply_staged_values()

## Discard all pending staged values across every registered setting.
func discard_staged_values() -> void:
	self._registry.discard_staged_values()

## Return `true` if at least one registered setting has a pending staged value.
func has_staged_values() -> bool:
	return self._registry.has_staged_values()

## Return the keys of settings that were applied at least once since the previous `_process()` call.
## The array is cleared after each frame, so it only contains settings applied during the current frame.
func get_changed_applied_settings() -> PackedStringArray:
	return self._settings_applied

## Return the keys of settings whose effective values changed at least once since the previous `_process()` call.
## The array is cleared after each frame, so it only contains settings changed during the current frame.
func get_changed_settings() -> PackedStringArray:
	return self._settings_changed

## Return the keys of settings whose staged values were set or cleared at least once since the previous `_process()` call.
## The array is cleared after each frame, so it only contains settings changed during the current frame.
func get_changed_staged_settings() -> PackedStringArray:
	return self._settings_staged_changed

## Load values from `config` into matching settings. See `Registry.from_config()` for full details.
func from_config(config: ConfigFile) -> void:
	self._registry.from_config(config)

## Serialize settings to a `ConfigFile` and returns it.
## `filter` controls which settings are included; defaults to exported settings only.
## See `Registry.to_config()` for full details.
func to_config(filter: Callable = Registry._include_exported) -> ConfigFile:
	return self._registry.to_config(filter)

func _on_applied(key: StringName) -> void:
	self._settings_applied.append(key)
	self.applied.emit(key)

func _on_changed(key: StringName) -> void:
	self._settings_changed.append(key)
	self.changed.emit(key)

func _on_staged_changed(key: StringName) -> void:
	self._settings_staged_changed.append(key)
	self.staged_changed.emit(key)
