#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace OpenVic2 {
	class Checksum : public godot::Object {
		GDCLASS(Checksum, godot::Object)

		//BEGIN BOILERPLATE
		static Checksum* _checksum;

	protected:
		static void _bind_methods() {
			godot::ClassDB::bind_method(godot::D_METHOD("get_checksum_text"), &Checksum::get_checksum_text);
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

		inline char* get_checksum_text() {
			return "1234abcd";
		}
	};

	Checksum* Checksum::_checksum = nullptr;
}