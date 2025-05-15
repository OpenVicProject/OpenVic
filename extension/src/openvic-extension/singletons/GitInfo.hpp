#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace OpenVic {
	class GitInfo : public godot::Object {
		GDCLASS(GitInfo, Object);

		godot::StringName _game_commit_hash;
		godot::StringName _game_commit_short_hash;
		godot::StringName _game_tag;
		godot::StringName _game_release;

		static inline GitInfo* _singleton = nullptr;

	protected:
		static void _bind_methods();

	public:
		godot::StringName get_commit_hash() const;
		uint64_t get_commit_timestamp() const;
		godot::StringName get_short_hash() const;
		godot::StringName get_tag() const;
		godot::StringName get_release_name() const;

		static GitInfo* get_singleton();

		GitInfo();
		~GitInfo();
	};
}
