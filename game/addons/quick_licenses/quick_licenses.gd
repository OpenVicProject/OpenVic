# ####################################################################################
# ##                      This file is part of Quick Licenses.                      ##
# ##              https://github.com/ProgrammerOnCoffee/Quick-Licenses              ##
# ####################################################################################
# ## Copyright (c) 2025 ProgrammerOnCoffee.                                         ##
# ##                                                                                ##
# ## Permission is hereby granted, free of charge, to any person obtaining a copy   ##
# ## of this software and associated documentation files (the "Software"), to deal  ##
# ## in the Software without restriction, including without limitation the rights   ##
# ## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      ##
# ## copies of the Software, and to permit persons to whom the Software is          ##
# ## furnished to do so, subject to the following conditions:                       ##
# ##                                                                                ##
# ## The above copyright notice and this permission notice shall be included in all ##
# ## copies or substantial portions of the Software.                                ##
# ##                                                                                ##
# ## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     ##
# ## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       ##
# ## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    ##
# ## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         ##
# ## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  ##
# ## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  ##
# ## SOFTWARE.                                                                      ##
# ####################################################################################

@icon("res://addons/quick_licenses/icon.svg")
class_name QuickLicenses
extends Control
## Displays a list of the game's third-party components.
##
## Displays a list of the game's third-party components.

## Emitted when the [QuickLicenses] node should be closed.
signal close_requested()

@export_group("Components", "components_")
## If [code]true[/code], the components list will be loaded when the
## [QuickLicenses] node is ready.[br]
## If [code]false[/code], the components list will be loaded the first time the
## [QuickLicenses] node is shown.
@export var components_preload := false
## If [code]true[/code], the components list will be unloaded when the
## [QuickLicenses] node is hidden.
@export var components_unload_when_hidden := true

@export_group("Theme Types", "theme_type_")
## The [member Control.theme_type_variation] applied to small buttons.
@export var theme_type_small_button := &"SButton"
## The [member Control.theme_type_variation] applied to small labels.
@export var theme_type_small_label := &"SLabel"
## The [member Control.theme_type_variation] applied to large labels.
@export var theme_type_large_label := &"LLabel"

## If [code]true[/code], the components list is currently loaded.
var _is_loaded := false
## If [code]true[/code], [QuickLicenses] is currently transitioning between two controls.
var _is_transitioning := false

# Automatic Button Feedback integration
## The [ButtonFeedback] node, if installed.[br]
## See the [url=https://github.com/ProgrammerOnCoffee/Button-Feedback]Button Feedback[/url]
## repository.
@onready var _button_feedback := get_node_or_null(^"/root/ButtonFeedback")
## The [VBoxContainer] that displays all components.
@onready var _component_list := $List/ScrollContainer/MarginContainer/VBoxContainer as VBoxContainer
## The [VBoxContainer] that displays information about the selected component.
@onready var _component_vbox := $Component/ScrollContainer/MarginContainer/VBoxContainer as VBoxContainer
## The [Label] that displays the selected license's text.
@onready var _license_label := $License/ScrollContainer/MarginContainer/Text as Label
## The [VBoxContainer] that displays information about the selected component.
@onready var _contributors_vbox := $Contributors/ScrollContainer/MarginContainer/VBoxContainer as VBoxContainer


func _ready() -> void:
	# Ensure no components have an invalid license
	var engine_licenses := Engine.get_license_info()
	var game_licenses := OVGame.get_license_info()
	var sim_licenses := OVSimulation.get_license_info()

	# Set control theme type variations
	for label in [$List/Label, $Component/Title, $License/Title]:
		label.theme_type_variation = theme_type_large_label
	_license_label.theme_type_variation = theme_type_small_label

	if components_preload or is_visible_in_tree():
		load_components()
	if not _is_loaded or components_unload_when_hidden:
		visibility_changed.connect(_on_visibility_changed)


## Loads the components list if [member _is_loaded] is [code]false[/code].
func load_components() -> void:
	if _is_loaded:
		return

	var components := preload("res://addons/quick_licenses/components.json").data

	_add_contributor_button("Contributors", OVGame.get_author_info())
	_component_list.add_child(HSeparator.new())

	var game_header := Label.new()
	game_header.name = &"GameHeader"
	game_header.text = "Game Components"
	_component_list.add_child(game_header)
	_component_list.add_child(HSeparator.new())
	for component in OVGame.get_copyright_info():
		var index := components.find_custom(func(c : Dictionary) -> bool: return c.name == component.name)
		if index != -1 and not components[index].source.is_empty():
			component.source = components[index].source
		_add_component(component)

	_component_list.add_child(HSeparator.new())
	_add_contributor_button("Simulation Contributors", OVSimulation.get_author_info())
	_component_list.add_child(HSeparator.new())

	var sim_header := Label.new()
	sim_header.name = &"SimHeader"
	sim_header.text = "Simulation Components"
	_component_list.add_child(sim_header)
	_component_list.add_child(HSeparator.new())
	for component in OVSimulation.get_copyright_info():
		var index := components.find_custom(func(c : Dictionary) -> bool: return c.name == component.name)
		if index != -1 and not components[index].source.is_empty():
			component.source = components[index].source
		_add_component(component)

	_component_list.add_child(HSeparator.new())
	_add_contributor_button("Godot Contributors", Engine.get_author_info())
	_component_list.add_child(HSeparator.new())

	_component_list.add_child(HSeparator.new())
	var godot_header := Label.new()
	godot_header.name = &"GodotHeader"
	godot_header.text = "Godot Engine Components"
	_component_list.add_child(godot_header)
	_component_list.add_child(HSeparator.new())
	for component in Engine.get_copyright_info():
		var index := components.find_custom(func(c : Dictionary) -> bool: return c.name == component.name)
		if index != -1 and not components[index].source.is_empty():
			component.source = components[index].source
		_add_component(component)

	_is_loaded = true


## Unloads the components list if [member _is_loaded] is [code]true[/code].
func unload_components() -> void:
	if _is_loaded:
		for child in _component_list.get_children():
			child.queue_free()
		_is_loaded = false


func _add_contributor_button(name, author_info) -> void:
	var button := Button.new()
	if _button_feedback:
		_button_feedback.setup_button(button)
	button.name = name
	button.text = name
	button.custom_minimum_size.x = 192
	button.theme_type_variation = theme_type_small_button
	button.alignment = HORIZONTAL_ALIGNMENT_LEFT
	# Button.autowrap_mode was introduced in 4.3
	# Check engine version for compatibility with earlier versions
	if Engine.get_version_info().hex >= 0x040300:
		button.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	button.pressed.connect(_on_contributors_button_pressed.bind(name, author_info))
	_component_list.add_child(button)


## Adds a component to the component list.
func _add_component(component) -> void:
	var button := Button.new()
	if _button_feedback:
		_button_feedback.setup_button(button)
	button.name = component.name
	button.text = component.name
	button.custom_minimum_size.x = 192
	button.theme_type_variation = theme_type_small_button
	button.alignment = HORIZONTAL_ALIGNMENT_LEFT
	# Button.autowrap_mode was introduced in 4.3
	# Check engine version for compatibility with earlier versions
	if Engine.get_version_info().hex >= 0x040300:
		button.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	button.pressed.connect(_on_component_button_pressed.bind(component))
	_component_list.add_child(button)


## Transitions controls in and out of view.
func _transition(from: Control, to: Control) -> void:
	_is_transitioning = true
	var tween := create_tween().set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_OUT)
	tween.tween_property(from, ^":modulate:a", 0.0, 0.3)
	tween.tween_callback(from.hide)
	# Set modulate in case to isn't already transparent
	tween.tween_callback(to.set.bind(&"modulate", Color.TRANSPARENT))
	tween.tween_callback(to.show)
	tween.tween_property(to, ^":modulate:a", 1.0, 0.3)
	await tween.finished
	_is_transitioning = false


func _on_visibility_changed() -> void:
	if is_visible_in_tree():
		load_components()
	elif components_unload_when_hidden:
		unload_components()


func _on_component_button_pressed(component) -> void:
	var engine_licenses := Engine.get_license_info()
	var game_licenses := OVGame.get_license_info()
	var sim_licenses := OVSimulation.get_license_info()

	if not _is_transitioning:
		$Component/Title.text = component.name
		# Link to asset source
		if "source" in component:
			var source_button := LinkButton.new()
			source_button.text = "Source"
			source_button.uri = component.source
			source_button.tooltip_text = component.source
			source_button.size_flags_horizontal = SIZE_SHRINK_BEGIN
			_component_vbox.add_child(source_button)

		for part in component.parts:
			var copyright_label := Label.new()
			copyright_label.theme_type_variation = theme_type_small_label
			copyright_label.custom_minimum_size.x = 192.0
			copyright_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
			for copyright in part.copyright:
				copyright_label.text += "\n\u00A9 " + copyright
			# Remove the first newline
			copyright_label.text = copyright_label.text.substr(1)
			_component_vbox.add_child(copyright_label)

			var license_hbox := HBoxContainer.new()
			var license_label := Label.new()
			license_label.theme_type_variation = theme_type_small_label
			license_label.text = "License:"
			license_hbox.add_child(license_label)

			var split := (
					" and " if " and " in part.license
					else " or " if " or " in part.license
					else ""
			)
			for license in part.license.split(split) if split else [part.license]:
				# Workaround for versions between GH-106218 GH-109339
				if license == "BSD-3-Clause":
					license = "BSD-3-clause"

				# Ensures MIT/Expat license is called MIT and only referenced once
				if license == "Expat":
					license = "MIT"

				if license in engine_licenses or license in game_licenses or license in sim_licenses:
					var button := Button.new()
					button.theme_type_variation = theme_type_small_button
					button.text = license
					if _button_feedback:
						_button_feedback.setup_button(button)
					button.pressed.connect(_on_license_button_pressed.bind(license))
					license_hbox.add_child(button)
				else:
					var label := Label.new()
					label.theme_type_variation = theme_type_small_label
					label.text = license
					license_hbox.add_child(label)

				if split:
					var split_label := Label.new()
					split_label.theme_type_variation = theme_type_small_label
					split_label.text = split
					license_hbox.add_child(split_label)
			# Remove the last split_label
			if split:
				license_hbox.get_child(-1).queue_free()
			_component_vbox.add_child(license_hbox)

		_transition($List, $Component)


func _on_license_button_pressed(license: String) -> void:
	if not _is_transitioning:
		$License/Title.text = license + " License"
		var engine_licenses := Engine.get_license_info()
		var game_licenses := OVGame.get_license_info()
		var sim_licenses := OVSimulation.get_license_info()
		if license in engine_licenses:
			_license_label.text = engine_licenses[license]
		elif license in game_licenses:
			_license_label.text = game_licenses[license]
		elif license in sim_licenses:
			_license_label.text = sim_licenses[license]
		else:
			_license_label.text = preload("res://addons/quick_licenses/licenses.json").data[license]
		$License/ScrollContainer.scroll_vertical = 0.0
		_transition($Component, $License)


func _on_contributors_button_pressed(project_name: String, author_info) -> void:
	if not _is_transitioning:
		$Contributors/Title.text = project_name + " Contributors"

		var skip_seperator := true
		for type in author_info:
			if skip_seperator: skip_seperator = false
			else: _contributors_vbox.add_child(HSeparator.new())
			var author_type_label := Label.new()
			author_type_label.theme_type_variation = theme_type_small_label
			author_type_label.custom_minimum_size.x = 192.0
			author_type_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
			author_type_label.text = type.capitalize()
			_contributors_vbox.add_child(author_type_label)
			_contributors_vbox.add_child(HSeparator.new())

			for author in author_info[type]:
				var label := Label.new()
				label.theme_type_variation = theme_type_small_label
				label.custom_minimum_size.x = 192.0
				label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
				label.text = author
				_contributors_vbox.add_child(label)

		$Contributors/ScrollContainer.scroll_vertical = 0.0
		_transition($List, $Contributors)


func _on_list_back_pressed() -> void:
	close_requested.emit()


func _on_component_back_pressed() -> void:
	if not _is_transitioning:
		await _transition($Component, $List)
		$Component/Title.text = ""
		for child in _component_vbox.get_children():
			child.queue_free()


func _on_license_back_pressed() -> void:
	if not _is_transitioning:
		await _transition($License, $Component)
		$License/Title.text = ""
		_license_label.text = ""


func _on_contributors_back_pressed() -> void:
	if not _is_transitioning:
		await _transition($Contributors, $List)
		$License/Title.text = ""
		_license_label.text = ""
