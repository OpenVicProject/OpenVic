#pragma once
#include "xxHash/include/xxhash.h"
#include <godot_cpp/core/class_db.hpp>
using namespace godot;
namespace OpenVic {
	class Checksum : public Object {
		GDCLASS(Checksum, Object)
		
		// INIT PART
		inline static Checksum* _checksum = nullptr;

	private:
		XXH64_hash_t hash_checksum;

	protected:
		static void _bind_methods();

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
		// END INIT
		void checksum_dir(XXH3_state_t* const state, PackedByteArray buffer, String directory_path);
		void calculate_checksum(String directory_path);
		XXH64_hash_t get_checksum() const;
	};
}
