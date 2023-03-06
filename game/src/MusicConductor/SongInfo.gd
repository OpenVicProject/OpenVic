extends Node
class_name SongInfo

var fullyQualifiedPath : String = ""
var readableName : String = ""
var loadedStream : Resource

func _init(dirname:String, fname:String):
		fullyQualifiedPath = dirname + fname
		readableName = fname.get_basename().replace("_", " ")
		loadedStream = load(fullyQualifiedPath)
