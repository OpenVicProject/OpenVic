#include "Checksum.hpp"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/string.hpp>

#include "openvic-extension/utility/ClassBindings.hpp"

using namespace OpenVic;
using namespace godot;

void Checksum::_bind_methods() {
	OV_BIND_METHOD(Checksum::get_checksum_text);
}

Checksum* Checksum::get_singleton() {
	return _singleton;
}

Checksum::Checksum() {
	ERR_FAIL_COND(_singleton != nullptr);
	_singleton = this;
}

Checksum::~Checksum() {
	ERR_FAIL_COND(_singleton != this);
	_singleton = nullptr;
}

/* REQUIREMENTS:
 * DAT-8
 */
godot::String Checksum::get_checksum_text() {
	return godot::String("1234abcd");
}
