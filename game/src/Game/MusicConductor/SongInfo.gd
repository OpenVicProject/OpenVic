extends Resource
class_name SongInfo

var song_path : String = ""
var song_name : String = ""
var song_stream : Resource

func _init(dirname:String, fname:String) -> void:
		song_path = dirname.path_join(fname)
		song_name = fname.get_basename().replace("_", " ")
		song_stream = load(song_path)
