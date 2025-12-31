#pragma once

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_mp3.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <openvic-simulation/types/IdentifierRegistry.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

namespace OpenVic {

	class SoundSingleton : public godot::Object {
		GDCLASS(SoundSingleton, godot::Object);

		static inline SoundSingleton* _singleton = nullptr;

		// cache of songs
		// names will be like "subfolder/songname", with "music/" base folder and the extension (.mp3) being excluded
		using song_asset_map_t = godot::HashMap<godot::StringName, godot::Ref<godot::AudioStreamMP3>>;
		song_asset_map_t tracks;

		// cache of sfx (map file name to an audio stream), only used temporarily until the sfx_define_map is built
		using sfx_asset_map_t = godot::HashMap<godot::StringName, godot::Ref<godot::AudioStreamWAV>>;
		sfx_asset_map_t sfx;

		// define name, stream ref, volume for sound effects so we can get these properties with a simple call in godot
		struct sound_asset_t {
			godot::Ref<godot::AudioStreamWAV> audio_stream;
			std::optional<fixed_point_t> volume;
		};
		using sfx_define_map_t = godot::HashMap<godot::StringName, sound_asset_t>;
		sfx_define_map_t sfx_define;

		static constexpr std::string_view title_theme_name = "thecoronation_titletheme.mp3";
		static constexpr std::string_view music_folder = "music/";
		static constexpr std::string_view sound_folder = "sound/";

		// property for gd scripts to access song names
		godot::Array PROPERTY(song_list);
		godot::String PROPERTY(title_theme);

		// property for gd scripts to access sound names
		godot::Array PROPERTY(sound_list);

	public:
		SoundSingleton();
		~SoundSingleton();
		static SoundSingleton* get_singleton();

	protected:
		static void _bind_methods();

		static godot::String to_define_file_name(godot::String const& path, std::string_view const& base_folder);

	public:
		// gets a song from the cache ('tracks' variable), or if not, then from the files
		godot::Ref<godot::AudioStreamMP3> get_song(godot::String const& name);
		godot::Ref<godot::AudioStreamWAV> get_sound(godot::String const& path);

		// load the files into memory
		bool load_music();
		bool load_sounds();
		bool load_title_theme();

		// for sound effects, get the stream and relative volume it should play at from the sfx map
		godot::Ref<godot::AudioStreamWAV> get_sound_stream(godot::String const& path);
		float get_sound_base_volume(godot::String const& path);
	};
}
