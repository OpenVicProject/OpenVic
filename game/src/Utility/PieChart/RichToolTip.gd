extends Panel
class_name RichToolTip

@onready var label:RichTextLabel = $RichTextLabel

#TODO: Temporary solution from https://github.com/godotengine/godot/issues/18260
# this will be unnecessary in godot 4.1 due to this:
# https://github.com/godotengine/godot/pull/71330 addition

var font_size:int = -1
var max_text_len:int = -1

#This doesn't handle non-default font sizes
func updateTextWidth() -> void:
	var text_size = label.get_theme_font("normal_font").get_multiline_string_size(label.text, HORIZONTAL_ALIGNMENT_RIGHT)
	var default_font_size =  label.get_theme_default_font_size()

	#Set the custom minimum size to be some product of how many chars * the font size
	var font_ratio:float = 1.0
	if font_size != -1:
		font_ratio = font_size as float / default_font_size as float
	custom_minimum_size = text_size * font_ratio

	#set the maximum width specifically to be a product of the maximum chars on any given
	#line
	var total_len: int = label.text.length()
	custom_minimum_size.x = custom_minimum_size.x * (max_text_len as float / total_len as float)
	label.custom_minimum_size = custom_minimum_size

func updateText(textIn:String, font_sizeIn:int = -1) -> void:
	var text:String = textIn
	font_size = font_sizeIn
	
	for line in label.text.split("\n"):
		max_text_len = max(max_text_len,line.length())
	
	if font_size != -1:
		text = "[font_size={size}]".format({"size":font_size}) + text + "[/font_size]"
	label.text = text
	updateTextWidth()
