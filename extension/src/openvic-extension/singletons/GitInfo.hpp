#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <openvic-simulation/utility/Getters.hpp>

namespace OpenVic {
	class GitInfo : public godot::Object {
		GDCLASS(GitInfo, Object);

		godot::StringName PROPERTY(commit_hash);
		uint64_t PROPERTY(commit_timestamp);
		godot::StringName PROPERTY(short_hash);
		godot::StringName PROPERTY(tag);
		godot::StringName PROPERTY(release_name);

		static inline GitInfo* _singleton = nullptr;

	protected:
		static void _bind_methods();

	public:
		static GitInfo* get_singleton();

		GitInfo();
		~GitInfo();
	};
}
