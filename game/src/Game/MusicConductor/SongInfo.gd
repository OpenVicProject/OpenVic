extends Resource
class_name SongInfo

var song_path : String = ""
var song_name : String = ""
var song_stream : AudioStream

#Initialize from a file path
func init_file_path(dirname : String, fname : String) -> void:
		song_path = dirname.path_join(fname)
		song_name = fname.get_basename().replace("_", " ")
		song_stream = load(song_path)

#Initialize from an audio stream
func init_stream(dirpath : String, name : String, stream : AudioStream) -> void:
		song_path = dirpath
		song_name = name
		song_stream = stream
