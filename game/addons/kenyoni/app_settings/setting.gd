extends RefCounted
## A single configurable value identified by a hierarchical key.
##
## Each setting holds a current effective value, a default value, optional validation and apply callables, and metadata.
## Settings are owned by a `Registry`, which must be assigned before `apply()` is called.
##
## **Staged mode** — when enabled, `set_value()` stores a pending value rather than applying it immediately.
## The pending value becomes the effective value only when `apply()` is called.
##
## **Readonly mode** — when enabled, all writes via `set_value()` and `reset()` are silently ignored.
class_name _KenyoniAppSettingSetting

const Registry := preload("res://addons/kenyoni/app_settings/registry.gd")

## The `Registry` this setting belongs to. Assigned by `Registry.add_setting()`.
var _registry: Registry = null
## Current effective value.
var _value: Variant
## Called every time `apply()` executes. Signature: `func(setting: Setting) -> void`.
var _apply_fn: Callable = Callable()
## Called to validate a candidate value before assignment. Must return `true` to accept the value. Signature: `func(setting: Setting, value: Variant) -> bool`.
var _validate_fn: Callable = Callable()
## Unique hierarchical key, e.g. `"graphics/display/fullscreen"`.
var _key: StringName
## Value restored by `reset()`. Set at construction and never changed afterwards.
var _default_value: Variant = null

## Whether staged mode is active.
var _is_staged: bool = false
## The pending staged value. Only meaningful when `_has_staged_value` is `true`.
var _staged_value: Variant = null
## `true` when a staged value is pending.
var _has_staged_value: bool = false
## When `true`, all write operations are silently ignored.
var _is_readonly: bool = false
## When `true`, the setting is excluded from auto-generated UIs.
var _is_internal: bool = false

## Create a setting with the given `key_` and `default_value_`.
## The default value is assigned as the initial effective value; `apply()` is not called.
func _init(key_: StringName, default_value_: Variant) -> void:
	self._key = key_
	self._default_value = default_value_
	self._value = default_value_

## Assign `new_value` after passing it through the validator.
##
## - If the setting is readonly, the call is ignored.
## - If `new_value` equals the current effective value and no staged value is pending, nothing happens.
## - If `new_value` equals the current effective value but a staged value is pending, the staged value is cleared.
## - In staged mode, `new_value` is stored as a pending staged value and `staged_changed` is emitted on the registry.
## - In normal mode, `new_value` is set immediately, `changed` is emitted, and `apply()` is called.
func set_value(new_value: Variant) -> void:
	if !self.validate(new_value):
		return
	self._set_value_no_validation(new_value)

## Return the current effective value. Does not reflect any pending staged value.
func value() -> Variant:
	return self._value

## Return the hierarchical key that uniquely identifies this setting.
func key() -> StringName:
	return self._key

## Return the default value supplied at construction time.
func default_value() -> Variant:
	return self._default_value

## Return true if the value is valid according to the validate callable or if no validate callable is set.
func validate(value_: Variant) -> bool:
	if self._validate_fn.is_valid():
		return self._validate_fn.call(self, value_)
	return true

## Commit the current state and triggers side-effects.
##
## If a staged value is pending:
## - It replaces the effective value.
## - `staged_changed` is emitted on the registry.
## - `changed` is emitted on the registry only if the staged value differed from the previous effective value.
##
## Regardless of staged state, the apply callable is invoked (if set) and `applied` is emitted on the registry.
func apply() -> void:
	if self._has_staged_value:
		var is_different: bool = self._staged_value != self._value
		self._value = self._staged_value
		self._staged_value = null
		self._has_staged_value = false
		if self._registry != null:
			self._registry.staged_changed.emit(self._key)
			if is_different:
				self._registry.changed.emit(self._key)
	if self._apply_fn.is_valid():
		self._apply_fn.call(self)
	self._registry.applied.emit(self._key)

## Reset the setting to its default value. Has no effect when readonly.
func reset() -> void:
	self._set_value_no_validation(self._default_value)

## Enable or disable staged mode.
## When `staged` is `false`, any pending staged value is discarded.
## Return `self` to allow method chaining.
func set_staged(staged: bool = true) -> _KenyoniAppSettingSetting:
	self._is_staged = staged
	if !staged:
		self.discard_staged_value()
	return self

## Set the validate callable. Signature: `func(setting: Setting, value: Variant) -> bool`.
## Return `self` to allow method chaining.
func set_validate_fn(fn: Callable) -> _KenyoniAppSettingSetting:
	self._validate_fn = fn
	return self

## Set the apply callable. Signature: `func(setting: Setting) -> void`.
## Return `self` to allow method chaining.
func set_apply_fn(fn: Callable) -> _KenyoniAppSettingSetting:
	self._apply_fn = fn
	return self

## Return `true` when staged mode is active.
func is_staged_mode() -> bool:
	return self._is_staged

## Return `true` when a staged value is pending and has not yet been applied.
func has_staged_value() -> bool:
	return self._has_staged_value

## Discard any pending staged value.
## Emit `staged_changed` on the registry if a value was actually cleared.
func discard_staged_value() -> void:
	if !self._has_staged_value:
		return
	self._staged_value = null
	self._has_staged_value = false
	if self._registry != null:
		self._registry.staged_changed.emit(self._key)

## Return the pending staged value, or `null` if none exists.
## Use `has_staged_value()` to distinguish a stored `null` from the absence of a staged value.
func staged_value() -> Variant:
	return self._staged_value

## Return the staged value if one is pending, otherwise return the current effective value.
func staged_or_value() -> Variant:
	if self._has_staged_value:
		return self._staged_value
	return self._value

## Enable or disables readonly mode. While readonly, `set_value()` and `reset()` are silently ignored.
## Returns `self` to allow method chaining.
func set_readonly(readonly: bool = true) -> _KenyoniAppSettingSetting:
	self._is_readonly = readonly
	return self

## Return `true` when the setting is readonly.
func is_readonly() -> bool:
	return self._is_readonly

## Mark or unmarks the setting as internal. Internal settings should be excluded from auto-generated UIs.
## Returns `self` to allow method chaining.
func set_internal(internal: bool = true) -> _KenyoniAppSettingSetting:
	self._is_internal = internal
	return self

## Return `true` when the setting is marked as internal.
func is_internal() -> bool:
	return self._is_internal

# Metadata
#
# Metadata follows Godot's `property_info` conventions where applicable.
#
# Built-in keys:
# - `exported` (`bool`): whether the setting is included by `Registry.to_config()`. Defaults to `true`.
# - `description` (`String`): human-readable label for display in a settings UI.

## Set whether the setting should be exported when generating configuration files.
## Returns `self` to allow method chaining.
func set_exported(exported: bool = true) -> _KenyoniAppSettingSetting:
	self.set_meta(&"exported", exported)
	return self

## Return `true` when the setting is exported to config files. `true` by default.
func is_exported() -> bool:
	return self.get_meta(&"exported", true)

## Set a human-readable description string.
## Returns `self` to allow method chaining.
func set_description(text: String) -> _KenyoniAppSettingSetting:
	self.set_meta(&"description", text)
	return self

## Return the description string, or an empty string if none is set.
func description() -> String:
	return self.get_meta(&"description", "")

## Store an arbitrary metadata value under `meta_key`. Fluent wrapper around `Object.set_meta()`.
## Returns `self` to allow method chaining.
func add_meta(meta_key: StringName, val: Variant) -> _KenyoniAppSettingSetting:
	self.set_meta(meta_key, val)
	return self

## Internal setter that skips the validate callable.
##
## - If the setting is readonly, the call is ignored.
## - If `new_value` equals the current effective value and no staged value is pending, nothing happens.
## - If `new_value` equals the current effective value but a staged value is pending, the staged value is cleared.
## - In staged mode, `new_value` is stored as a pending staged value and `staged_changed` is emitted.
## - In normal mode, `new_value` is set immediately, `changed` is emitted, and `apply()` is called.
func _set_value_no_validation(new_value: Variant) -> void:
	if self._is_readonly:
		return
	if self._value == new_value:
		if self._has_staged_value:
			self.discard_staged_value()
		return

	if self._is_staged:
		if self._has_staged_value && self._staged_value == new_value:
			return
		self._staged_value = new_value
		self._has_staged_value = true
		if self._registry != null:
			self._registry.staged_changed.emit(self._key)
		return
	else:
		self._value = new_value

	if self._registry != null:
		self._registry.changed.emit(self._key)
	self.apply()
