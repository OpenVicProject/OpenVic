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
	return _checksum;
}

Checksum::Checksum() {
	ERR_FAIL_COND(_checksum != nullptr);
	_checksum = this;
}

Checksum::~Checksum() {
	ERR_FAIL_COND(_checksum != this);
	_checksum = nullptr;
}

/* REQUIREMENTS:
 * DAT-8
 */
godot::String Checksum::get_checksum_text() {
	return godot::String("1234abcd");
}
