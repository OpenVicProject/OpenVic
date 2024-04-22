@tool
@icon("res://addons/MusicMetadata/icon.svg")
extends Control

## The current [MusicMetadata] object being displayed at this time.
@export var metadata:MusicMetadata:
	get:
		return get_metadata()
	set(_value):
		set_metadata(_value)
# ## This is intended to be private.
# ## A backing varible for [metadata].
var _metadata = null

## A reference to the [TextureRect] used to display the [member MusicMetadata.cover].
@export var art_display:TextureRect = null
## A reference to the [Label] used to display the [member MusicMetadata.title].
@export var title_display:Label = null
## A reference to the [Label] used to display the [member MusicMetadata.artist] and the [member MusicMetadata.album_artist].
@export var artist_band_display:Label = null
## A reference to the [Label] used to display the [member MusicMetadata.album].
@export var album_display:Label = null
## A reference to the [Label] used to display the [member MusicMetadata.comments].
@export var description_display:Label = null
## A reference to the [Label] used to display the [member MusicMetadata.urls].
@export var url_display:Label = null

## Used to format the contents of the [member album_display].
## It's a string only formatted with one value, [member MusicMetadata.album].
@export var album_format:String = "From %s"
## Used to format the contents of the [member artist_band_display].
## It's a string formatted with two values, [member MusicMetadata.artist] and [member MusicMetadata.album_artist].
@export var artist_band_format:String = "By %s / %s"
## Used to format the contents of the [member description_display].
## It's a string only formatted with one value, [member MusicMetadata.comments].
@export var description_format:String = "%s"
## Used to format the contents of the [member description_display].
## This string is formatted once each with url type and url (in that respective order)
## in [member MusicMetadata.urls]; each of those becoming a new line in [member description_display].
@export var url_format:String = "%s link: %s" #a string only formatted with two values, the string of the url name and the string of the url

func _enter_tree():
	_hook_property_changed()
	update_UI()

func _ready():
	_hook_property_changed()
	update_UI()

func _exit_tree():
	_unhook_property_changed()

## Used to set the displayed [member metadata] from the given [param data].
func set_metadata_from_data(data:PackedByteArray):
	metadata = metadata.new(data)

## Used to set the displayed [member metadata] from the given [param stream].
func set_metadata_from_stream(stream:AudioStream):
	metadata = metadata.new(stream)

## Sets the displayed [member metadata] from the given [param metadata].
func set_metadata(metadata:MusicMetadata):
	_unhook_property_changed()
	_metadata = metadata
	_hook_property_changed()
	update_UI()

## Returns the displayed [member metadata].
func get_metadata() -> MusicMetadata:
	return _metadata

## Updates the UI status form the current state of [member metadata].
## Used internally when [member metadata] is changed or modified.
func update_UI():
	if art_display != null:
		if metadata != null and metadata.cover != null:
			art_display.visible = true
			art_display.texture = metadata.cover
		else:
			art_display.visible = false
	
	if title_display != null:
		if metadata != null and metadata.title != "":
			title_display.visible = true
			title_display.text = metadata.title
		else:
			title_display.visible = false
	
	if artist_band_display != null:
		if metadata != null and (metadata.artist != "" or metadata.album_artist != ""):
			artist_band_display.visible = true
			artist_band_display.text = artist_band_format % [metadata.artist, metadata.album_artist]
		else:
			artist_band_display.visible = false
		
	if album_display != null:
		if metadata != null and metadata.album != "":
			album_display.visible = true
			album_display.text = album_format % [metadata.album]
		else:
			album_display.visible = false
	
	if description_display != null:
		if metadata != null and metadata.comments != "":
			description_display.visible = true
			description_display.text = description_format % metadata.comments
		else:
			description_display.visible = false

	if url_display != null:
		if metadata != null and len(metadata.urls) > 0:
			url_display.visible = true
			url_display.text = ""
			for url_type in metadata.urls.keys():
				url_display.text += url_format % [url_type, metadata.urls[url_type]]
		else:
			url_display.visible = false

# ## This method is intended to be private.
# ## Used to hook [method update_UI] to [member metadata]'s [signal MusicMetadata.changed] signal.
func _hook_property_changed():
	if metadata != null and not metadata.changed.is_connected(update_UI):
		metadata.changed.connect(update_UI)

# ## This method is intended to be private.
# ## Used to unhook [method update_UI] to [member metadata]'s [signal MusicMetadata.changed] signal.
func _unhook_property_changed():
	if metadata != null and metadata.changed.is_connected(update_UI):
		metadata.changed.disconnect(update_UI)
