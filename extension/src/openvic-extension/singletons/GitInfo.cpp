
#include "GitInfo.hpp"

#include <godot_cpp/core/error_macros.hpp>

#include "openvic-extension/utility/ClassBindings.hpp"

#include "gen/git_info.gen.hpp"

using namespace godot;
using namespace OpenVic;

GitInfo::GitInfo() {
	ERR_FAIL_COND(_singleton != nullptr);
	_singleton = this;

	_game_commit_hash = { GAME_COMMIT_HASH.data(), true };
	_game_commit_short_hash = _game_commit_hash.substr(0, 7);
	_game_tag = { GAME_TAG.data(), true };
	_game_release = { GAME_RELEASE.data(), true };
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

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "commit_hash"), "", "get_commit_hash");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "commit_timestamp"), "", "get_commit_timestamp");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "short_hash"), "", "get_short_hash");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "tag"), "", "get_tag");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "release_name"), "", "get_release_name");
}

StringName GitInfo::get_commit_hash() const {
	return _game_commit_hash;
}

uint64_t GitInfo::get_commit_timestamp() const {
	return GAME_COMMIT_TIMESTAMP;
}

StringName GitInfo::get_short_hash() const {
	return _game_commit_short_hash;
}

StringName GitInfo::get_tag() const {
	return _game_tag;
}

StringName GitInfo::get_release_name() const {
	return _game_release;
}

GitInfo* GitInfo::get_singleton() {
	return _singleton;
}
