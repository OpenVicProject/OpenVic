#include "SoundSingleton.hpp"

#include <string_view>

#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/stream_peer_buffer.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/string.hpp>

#include <openvic-dataloader/v2script/AbstractSyntaxTree.hpp>

#include <openvic-simulation/core/string/Utility.hpp>
#include <openvic-simulation/dataloader/Dataloader.hpp>
#include <openvic-simulation/dataloader/NodeTools.hpp>

#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/Convert.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;
using namespace OpenVic::NodeTools;

// ov_bind_method is used to make a method visible to godot
void SoundSingleton::_bind_methods() {
	OV_BIND_METHOD(SoundSingleton::load_music);
	OV_BIND_METHOD(SoundSingleton::get_song, { "song_name" });
	OV_BIND_METHOD(SoundSingleton::get_song_list);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "song_list", PROPERTY_HINT_ARRAY_TYPE, "AudioStreamMP3"), "", "get_song_list");

	OV_BIND_METHOD(SoundSingleton::load_sounds);
	OV_BIND_METHOD(SoundSingleton::get_sound_stream, { "sound_name" });
	OV_BIND_METHOD(SoundSingleton::get_sound_base_volume, { "sound_name" });
	OV_BIND_METHOD(SoundSingleton::get_sound_list);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "sound_list", PROPERTY_HINT_ARRAY_TYPE, "AudioStreamWAV"), "", "get_sound_list");

	OV_BIND_METHOD(SoundSingleton::load_title_theme);
	OV_BIND_METHOD(SoundSingleton::get_title_theme);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title_theme"), "", "get_title_theme");
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

// slices a path down to after the base_folder, keeps the extension
// this is because the defines refer to audio files using this format,
// so we might as well use this form as the key for the "name"->audiostream map
String SoundSingleton::to_define_file_name(String const& path, std::string_view const& base_folder) {
	String name = path.replace("\\", "/");
	return name.get_slice(base_folder.data(), 1); // get file name with extension
}

// Load a sound from the cache, or directly if its not in the cache
// take in a path, extract just the file name for the cache (and defines)
Ref<AudioStreamMP3> SoundSingleton::get_song(String const& path) {
	String name = to_define_file_name(path, music_folder);

	const song_asset_map_t::Iterator it = tracks.find(name);
	if (it != tracks.end()) { // it->first = key, it->second = value
		return it->value;
	}

	const Ref<AudioStreamMP3> song = AudioStreamMP3::load_from_file(path);

	ERR_FAIL_NULL_V_MSG(song, Ref<AudioStreamMP3>(), Utilities::format("Failed to load music file: %s", path));
	tracks.insert(std::move(name), song);

	return song;
}

// loading music is actually one of the slower things to do, and we want the title theme
// playing the whole time. Solution: load it first and separately
bool SoundSingleton::load_title_theme() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V_MSG(game_singleton, false, Utilities::format("Error retrieving GameSingleton"));

	static constexpr std::string_view music_directory = "music";
	bool ret = false;

	Dataloader::path_vector_t music_files =
		game_singleton->get_dataloader().lookup_files_in_dir_recursive(music_directory, ".mp3");

	if (music_files.size() < 1) {
		ERR_PRINT("Failed to load title theme: No files in music directory.");
	}

	for (std::filesystem::path const& file_name : music_files) {
		// the path
		String file = convert_to<String>(file_name.string());
		// file name
		String file_stem = to_define_file_name(file, music_folder);

		if (file_stem == title_theme_name.data()) {
			ERR_BREAK_MSG(!get_song(file).is_valid(), Utilities::format("Failed to load title theme song at path %s.", convert_to<String>(file_name.string())));

			String name = to_define_file_name(file, music_folder);
			title_theme = name;
			ret = true;
			break;
		}
	}

	if (!ret) {
		ERR_PRINT("Failed to load title theme!");
	}

	return ret;
}

bool SoundSingleton::load_music() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V_MSG(game_singleton, false, Utilities::format("Error retrieving GameSingleton"));

	static constexpr std::string_view music_directory = "music";
	bool ret = true;

	Dataloader::path_vector_t music_files =
		game_singleton->get_dataloader().lookup_files_in_dir_recursive(music_directory, ".mp3");

	if (music_files.size() < 1) {
		ERR_PRINT("Failed to load music: No files in music directory.");
		ret = false;
	}

	for (std::filesystem::path const& file_name : music_files) {
		String file = convert_to<String>(file_name.string());
		String name = to_define_file_name(file, music_folder);
		if (name == title_theme_name.data()) {
			continue;
		}

		if (!get_song(file).is_valid()) {
			ERR_PRINT(Utilities::format("Failed to load song at path %s.", convert_to<String>(file_name.string())));
			ret = false;
			continue; // don't try to append a null pointer to the list
		}
		song_list.append(name);
	}

	return ret;
}

// Load a sound into the sound cache, accessed via its file path
Ref<AudioStreamWAV> SoundSingleton::get_sound(String const& path) {
	String name = to_define_file_name(path, sound_folder);

	const sfx_asset_map_t::Iterator it = sfx.find(name);
	if (it != sfx.end()) { // it->first = key, it->second = value
		return it->value;
	}

	const Ref<AudioStreamWAV> sound = AudioStreamWAV::load_from_file(path);

	ERR_FAIL_NULL_V_MSG(
		sound, Ref<AudioStreamMP3>(), Utilities::format("Failed to load sound file %s", path) // named %s, path
	);

	sfx.insert(std::move(name), sound);
	return sound;
}

// Get a sound by its define name
Ref<AudioStreamWAV> SoundSingleton::get_sound_stream(String const& path) {
	sfx_define_map_t::Iterator it = sfx_define.find(path);
	ERR_FAIL_COND_V_MSG(
		it == sfx_define.end(), Ref<AudioStreamMP3>(), Utilities::format("Attempted to retrieve sound stream at invalid index %s.", path)
	);

	return it->value.audio_stream;
}

// get the base volume of a sound from its define name
float SoundSingleton::get_sound_base_volume(String const& path) {
	if (sfx_define[path].volume.has_value()) {
		return static_cast<float>(sfx_define[path].volume.value());
	}
	return 1.0;
}

// Requires the sound defines to already be loaded by the dataloader
// then build the define map (define_identifier->{audiostream,volume})
bool SoundSingleton::load_sounds() {
	static constexpr std::string_view sound_directory = "sound";
	bool ret = true;

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V_MSG(game_singleton, false, Utilities::format("Error retrieving GameSingleton"));

	SoundEffectManager const& sound_manager = game_singleton->get_definition_manager().get_sound_effect_manager();

	if (sound_manager.sound_effects_empty()) {
		ERR_PRINT("Failed to load music: No identifiers in sounds.sfx.");
		ret = false;
	}

	for (SoundEffect const& sound_inst : sound_manager.get_sound_effects()) {
		fs::path const& full_path = sound_inst.get_file();

		// UI_Cavalry_Selected.wav doesn't exist (paradox mistake, UI_Cavalry_Select.wav does), just keep going
		// the define its associated with also isn't used in game
		if (full_path.empty()) {
			WARN_PRINT(Utilities::format("The sound define %s points to non-existent file.", convert_to<String>(sound_inst.get_identifier())));
			continue;
		}

		Ref<AudioStreamWAV> stream = get_sound(convert_to<String>(full_path.string()));
		if (stream.is_null()) {
			ERR_PRINT(Utilities::format("Failed to load sound %s at path %s.", convert_to<String>(sound_inst.get_identifier()), convert_to<String>(full_path.string())));
			ret = false;
			continue; // don't try to append a null pointer to the list
		}

		String name = to_define_file_name(convert_to<String>(full_path.string()), sound_folder);

		StringName define_gd_name = convert_to<String>(sound_inst.get_identifier());
		sfx_define[define_gd_name].audio_stream = get_sound(name);
		sfx_define[define_gd_name].volume = sound_inst.get_volume();

		sound_list.append(define_gd_name);
	}

	return ret;
}
