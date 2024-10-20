#include "SoundSingleton.hpp"

#include <string_view>

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/stream_peer_buffer.hpp>

#include "openvic-simulation/utility/StringUtils.hpp"
#include <openvic-extension/utility/Utilities.hpp>
#include <openvic-extension/utility/ClassBindings.hpp>
#include <openvic-extension/singletons/GameSingleton.hpp>
#include <openvic-dataloader/v2script/AbstractSyntaxTree.hpp>
#include <openvic-simulation/dataloader/Dataloader.hpp>
#include <openvic-simulation/dataloader/NodeTools.hpp>

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_to_godot_string;

using namespace godot;
using namespace OpenVic;
using namespace OpenVic::NodeTools;

//ov_bind_method is used to make a method visible to godot
void SoundSingleton::_bind_methods() {
	OV_BIND_METHOD(SoundSingleton::load_music);
	OV_BIND_METHOD(SoundSingleton::get_song, {"song_name"});
	OV_BIND_METHOD(SoundSingleton::get_song_list);

	ADD_PROPERTY(PropertyInfo(
		Variant::ARRAY,
		"song_list", PROPERTY_HINT_ARRAY_TYPE,
		"AudioStreamMP3"),
	"", "get_song_list");

	OV_BIND_METHOD(SoundSingleton::load_sounds);
	OV_BIND_METHOD(SoundSingleton::get_sound_stream, {"sound_name"});
	OV_BIND_METHOD(SoundSingleton::get_sound_base_volume, {"sound_name"});
	OV_BIND_METHOD(SoundSingleton::get_sound_list);

	ADD_PROPERTY(PropertyInfo(
		Variant::ARRAY,
		"sound_list",
		PROPERTY_HINT_ARRAY_TYPE,
		"AudioStreamWAV"),
	"", "get_sound_list");

	OV_BIND_METHOD(SoundSingleton::load_title_theme);
	OV_BIND_METHOD(SoundSingleton::get_title_theme);

	ADD_PROPERTY(PropertyInfo(
		Variant::STRING,
		"title_theme"),
	"", "get_title_theme");

}

SoundSingleton* SoundSingleton::get_singleton() {
	return _singleton;
}

SoundSingleton::SoundSingleton() {
	ERR_FAIL_COND(_singleton != nullptr);
	_singleton = this;
}

SoundSingleton::~SoundSingleton() {
	ERR_FAIL_COND(_singleton != this);
	_singleton = nullptr;
}

//Load a sound from the path
Ref<AudioStreamMP3> SoundSingleton::_load_godot_mp3(String const& path) const {
	const Ref<FileAccess> file = FileAccess::open(path, FileAccess::ModeFlags::READ);

	Error err = FileAccess::get_open_error();
	ERR_FAIL_COND_V_MSG(
		err != OK || file.is_null(), nullptr,
		vformat("Failed to open mp3 file %s", path) //named %s, path,
	);

	const PackedByteArray data = file->get_buffer(file->get_length());

	Ref<AudioStreamMP3> sound = Ref<AudioStreamMP3>();
	sound.instantiate();
	sound->set_data(data);

	return sound;
}

//slices a path down to after the base_folder, keeps the extension
//this is because the defines refer to audio files using this format,
//so we might as well use this form as the key for the "name"->audiostream map
String SoundSingleton::to_define_file_name(String const& path, std::string_view const& base_folder) const {
	String name = path.replace("\\","/");
	return name.get_slice(base_folder.data(),1); //get file name with extension
}

//Load a sound from the cache, or directly if its not in the cache
//take in a path, extract just the file name for the cache (and defines)
Ref<AudioStreamMP3> SoundSingleton::get_song(String const& path){
	String name = to_define_file_name(path, music_folder);

	const song_asset_map_t::const_iterator it = tracks.find(name);
	if (it != tracks.end()) { //it->first = key, it->second = value
		return it->second;
	}

	const Ref<AudioStreamMP3> song = _load_godot_mp3(path);

	ERR_FAIL_NULL_V_MSG(
		song, nullptr,
		vformat("Failed to load music file: %s", path)
	);
	tracks.emplace(std::move(name), song);

	return song;

}

//loading music is actually one of the slower things to do, and we want the title theme
//playing the whole time. Solution: load it first and separately
bool SoundSingleton::load_title_theme(){
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V_MSG(game_singleton, false, vformat("Error retrieving GameSingleton"));

	static constexpr std::string_view music_directory = "music";
	bool ret = false;

	Dataloader::path_vector_t music_files = game_singleton->get_dataloader()
		.lookup_files_in_dir_recursive(music_directory, ".mp3");

	if(music_files.size() < 1){
		Logger::error("failed to load title theme: no files in music directory");
	}

	for(std::filesystem::path const& file_name : music_files) {
		//the path
		String file = std_to_godot_string(file_name.string());
		//file name
		String file_stem = to_define_file_name(file,music_folder);

		if(file_stem == title_theme_name.data()){
			if(!get_song(file).is_valid()){
				Logger::error("failed to load title theme song at path ",file_name);
				break; //don't try to append a null pointer to the list
			}
			else{
				String name = to_define_file_name(file,music_folder);
				title_theme = name;
				ret = true;
				break;
			}
		}

	}

	if(!ret) Logger::error("Failed to load title theme!");

	return ret;
}

bool SoundSingleton::load_music() {

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V_MSG(game_singleton, false, vformat("Error retrieving GameSingleton"));

	static constexpr std::string_view music_directory = "music";
	bool ret = true;

	Dataloader::path_vector_t music_files = game_singleton->get_dataloader()
		.lookup_files_in_dir_recursive(music_directory, ".mp3");

	if(music_files.size() < 1){
		Logger::error("failed to load music: no files in music directory");
		ret = false;
	}

	for(std::filesystem::path const& file_name : music_files) {
		String file = std_to_godot_string(file_name.string());
		String name = to_define_file_name(file,music_folder);
		if(name == title_theme_name.data()) continue;

		if(!get_song(file).is_valid()){
			Logger::error("failed to load song at path ",file_name);
			ret = false;
			continue; //don't try to append a null pointer to the list
		}
		song_list.append(name);
	}

	return ret;
}

//Load a sound into the sound cache, accessed via its file path
Ref<AudioStreamWAV> SoundSingleton::get_sound(String const& path){
	String name = to_define_file_name(path, sound_folder);

	const sfx_asset_map_t::const_iterator it = sfx.find(name);
	if (it != sfx.end()) { //it->first = key, it->second = value
		return it->second;
	}

	const Ref<AudioStreamWAV> sound = _load_godot_wav(path);

	ERR_FAIL_NULL_V_MSG(
		sound, nullptr,
		vformat("Failed to load sound file %s", path) //named %s, path
	);

	sfx.emplace(std::move(name), sound);
	return sound;
}

//Get a sound by its define name
Ref<AudioStreamWAV> SoundSingleton::get_sound_stream(String const& path) {
	if(sfx_define[path].audioStream.has_value()){
		return sfx_define[path].audioStream.value();
	}

	ERR_FAIL_V_MSG(
		nullptr,
		vformat("Attempted to retrieve sound stream at invalid index ", path)
	);

}

//get the base volume of a sound from its define name
float SoundSingleton::get_sound_base_volume(String const& path) {
	if(sfx_define[path].volume.has_value()){
		return sfx_define[path].volume.value().to_float();
	}
	return 1.0;
}

//Requires the sound defines to already be loaded by the dataloader
//then build the define map (define_identifier->{audiostream,volume})
bool SoundSingleton::load_sounds() {
	static constexpr std::string_view sound_directory = "sound";
	bool ret = true;

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V_MSG(game_singleton, false, vformat("Error retrieving GameSingleton"));

	SoundEffectManager const& sound_manager = game_singleton->get_definition_manager().get_sound_effect_manager();

	if(sound_manager.sound_effects_empty()){
		Logger::error("failed to load music: no identifiers in sounds.sfx");
		ret = false;
	}

	for(SoundEffect const& sound_inst : sound_manager.get_sound_effects()){
		std::string folder_path = StringUtils::append_string_views(sound_directory, "/", sound_inst.get_file());
		fs::path full_path = game_singleton->get_dataloader().lookup_file(folder_path, false);

		//UI_Cavalry_Selected.wav doesn't exist (paradox mistake, UI_Cavalry_Select.wav does), just keep going
		//the define its associated with also isn't used in game
		if(full_path.empty()){
			Logger::warning("The sound define ",sound_inst.get_identifier()," points to an non-existing file ", folder_path);
			continue;
		}

		Ref<AudioStreamWAV> stream = get_sound(std_to_godot_string(full_path.string()));
		if(stream.is_null()){
			Logger::error("failed to load sound ",sound_inst.get_identifier()," at path ",full_path);
			ret = false;
			continue; //don't try to append a null pointer to the list
		}

		String name = to_define_file_name(std_to_godot_string(full_path.string()), sound_folder);

		StringName define_gd_name = std_to_godot_string(sound_inst.get_identifier());
		sfx_define[define_gd_name].audioStream = get_sound(name);
		sfx_define[define_gd_name].volume = sound_inst.get_volume();

		sound_list.append(define_gd_name);
	}

	return ret;

}

Ref<AudioStreamWAV> SoundSingleton::_load_godot_wav(String const& path) const {
	const Ref<FileAccess> file = FileAccess::open(path, FileAccess::ModeFlags::READ);

	Error err = FileAccess::get_open_error();
	ERR_FAIL_COND_V_MSG(
		err != OK || file.is_null(), nullptr,
		vformat("Failed to open wav file %s", path)
	);

	Ref<AudioStreamWAV> sound = Ref<AudioStreamWAV>();
	sound.instantiate();

	//RIFF file header
	String riff_id = read_riff_str(file); //RIFF
	int riff_size = std::min(static_cast<uint64_t>(file->get_32()), file->get_length());
	String form_type = read_riff_str(file); //WAVE

	//ie. 16, 24, 32 bit audio
	int bits_per_sample = 0;

	//godot audiostreamwav has: data,format,loop_begin,loop_end,loop_mode,mix_rate,stereo

	//RIFF reader
	while(file->get_position() < riff_size){
		String id = read_riff_str(file);
		int size = file->get_32();
		if(id=="LIST"){
			String list_type = read_riff_str(file);
		}
		else if(id=="JUNK"){
			const PackedByteArray junk = file->get_buffer(size);
		}
		else if(id=="fmt "){
			//what fields to read depends on the fmt chunk variant (can be 16, 18, or 40 bytes long)
			//basic fields

			//2bytes: type of format can be 1=PCM, 3=IEEE float, 6=8bit Alaw, 7=8bit mu-law, FFFE=go by subformat
			int formatTag = file->get_16();
			int channels = file->get_16();
			int samplesPerSec = file->get_32();
			int avgBytesPerSec = file->get_32();
			int blockAlign = file->get_16();

			bits_per_sample = file->get_16();
			ERR_FAIL_COND_V_MSG(
				bits_per_sample == 24 || bits_per_sample == 32, nullptr,
				vformat("Unsupported wav file sample rate %s", bits_per_sample)
			);

			if(size > 16){
				int extensionSize = file->get_16();
			}
			if(size > 18){
				//extensible format
				int validBitsPerSample = file->get_16();
				int channelMask = file->get_32();

				//16 byte subformat
				int subFormat = file->get_16();
				String subFormatString = read_riff_str(file,14);
			}

			//set godot properties
			sound->set_stereo(channels==2);
			switch(formatTag){ //TODO: verify, looks from 1 doc like these should be 0x0101, 0x102, ...
				case 0:{
					sound->set_format(sound->FORMAT_8_BITS);
					break;
				}
				case 1:{
					sound->set_format(sound->FORMAT_16_BITS);
					break;
				}
				case 2:{
					sound->set_format(sound->FORMAT_IMA_ADPCM);
					break;
				}
				default:{
					Logger::warning("unknown WAV format tag %x",formatTag);
					sound->set_format(sound->FORMAT_16_BITS);
					break;
				}
			}

			sound->set_mix_rate(samplesPerSec);
		}
		else if(id=="data"){
			PackedByteArray audio_data = file->get_buffer(size);

			if(bits_per_sample == 24 || bits_per_sample == 32){
				//sound->set_data(to_16bit_wav_data(audio_data,bits_per_sample));
				Logger::error("WAV file ",godot_to_std_string(path), " uses an unsupported sample rate ", bits_per_sample);
			}
			else{
				sound->set_data(audio_data);
			}
		}
		else if(id=="fact"){ //for compressed formats that aren't PCM
			//TODO: Handle these other cases
			int sampleLen = file->get_32(); //# samples/channel
			Logger::warning("WAV fact header, indicates likely unhandled case");
		}
		else{
			//Logger::warning("skipping Unhandled RIFF chunk of id ",godot_to_std_string(id));
			//known chunks that cause this: "smpl", "labl", "cue ", "ltxt", info chunks (IART, ICOP, IENG, ...)
			//these don't seem to be needed for our uses
			const PackedByteArray junk = file->get_buffer(size); //just try and skip this chunk

		}
		if(file->get_position() % 2 != 0){ //align to even bytes
			file->get_8();
		}

	}

	sound->set_loop_end(file->get_length()/4);
	return sound;
}

//set size if its an info string, otherwise leaving
String SoundSingleton::read_riff_str(Ref<FileAccess> const& file, int size) const {
	return file->get_buffer(size).get_string_from_ascii();
}