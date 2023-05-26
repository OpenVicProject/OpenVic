extends Panel
class_name RichToolTip

@onready var label:RichTextLabel = $RichTextLabel

#TODO: Temporary solution based on https://github.com/godotengine/godot/issues/18260
# this will be unnecessary (and should be removed) in godot 4.1 due to 
# this addition: https://github.com/godotengine/godot/pull/71330 
#essentially the background panel doesn't scale to fit onto a richtext label
# so this script makes an estimate for the size, which will be fixed in 4.1


#This doesn't handle non-default font sizes
func updateTextWidth(font_size:int, max_text_len:int, lines:int) -> void:
	
	var default_font_size =  label.get_theme_default_font_size()
	var font_ratio:float = 1.0
	if font_size != -1:
		font_ratio = font_size as float / default_font_size as float

	var total_len: int = label.text.length()
	
	var length_ratio = (max_text_len as float / total_len as float)

	#the 0.8, -27 and 2.5 are magic numbers
	#-27 subtracts the bb code formatting text from the length
	custom_minimum_size = Vector2( 0.8 *(max_text_len - 27) * font_ratio * font_size, lines*2.5*font_size*font_ratio)

func updateText(textIn:String, font_size:int = -1) -> void:
	var text:String = textIn
	var max_text_len:int = -1
	for line in label.text.split("\n"):
		max_text_len = max(max_text_len,line.length())
	var lines = label.text.split("\n").size()
	
	if font_size != -1:
		text = "[font_size={size}]".format({"size":font_size}) + text + "[/font_size]"
	label.text = text
	updateTextWidth(font_size, max_text_len, lines)
