
#include "GitInfo.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/core/error_macros.hpp>

#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

#include "gen/commit_info.gen.hpp"

using namespace godot;
using namespace OpenVic;

GitInfo::GitInfo() {
	ERR_FAIL_COND(_singleton != nullptr);
	_singleton = this;

	commit_hash = Utilities::std_to_godot_string(GAME_COMMIT_HASH);
	commit_timestamp = GAME_COMMIT_TIMESTAMP;
	short_hash = commit_hash.substr(0, 7);
	tag = Utilities::std_to_godot_string(GAME_TAG);
	release_name = Utilities::std_to_godot_string(GAME_RELEASE);
}

GitInfo::~GitInfo() {
	ERR_FAIL_COND(_singleton != this);
	_singleton = nullptr;
}

void GitInfo::_bind_methods() {
	OV_BIND_METHOD(GitInfo::get_commit_hash);
	OV_BIND_METHOD(GitInfo::get_commit_timestamp);
	OV_BIND_METHOD(GitInfo::get_short_hash);
	OV_BIND_METHOD(GitInfo::get_tag);
	OV_BIND_METHOD(GitInfo::get_release_name);

	ADD_PROPERTY(
		PropertyInfo(Variant::STRING_NAME, "commit_hash", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), //
		"", "get_commit_hash"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::INT, "commit_timestamp", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), //
		"", "get_commit_timestamp"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::STRING_NAME, "short_hash", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), //
		"", "get_short_hash"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::STRING_NAME, "tag", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), //
		"", "get_tag"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::STRING_NAME, "release_name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), //
		"", "get_release_name"
	);
}

GitInfo* GitInfo::get_singleton() {
	return _singleton;
}
