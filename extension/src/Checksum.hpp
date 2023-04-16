#pragma once

#include <godot_cpp/core/class_db.hpp>

namespace OpenVic2 {
	class Checksum : public godot::Object {
		GDCLASS(Checksum, godot::Object)

		//BEGIN BOILERPLATE
		inline static Checksum* _checksum = nullptr;

	protected:
		static void _bind_methods() {
			godot::ClassDB::bind_method(godot::D_METHOD("get_checksum_text"), &Checksum::getChecksumText);
		}

	public:
		inline static Checksum* get_singleton() { return _checksum; }

		inline Checksum() {
			ERR_FAIL_COND(_checksum != nullptr);
			_checksum = this;
		}
		inline ~Checksum() {
			ERR_FAIL_COND(_checksum != this);
			_checksum = nullptr;
		}
		//END BOILERPLATE

		inline godot::String getChecksumText() {
			return godot::String("1234abcd");
		}
	};
}
