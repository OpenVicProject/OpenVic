#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

namespace OpenVic {
	namespace CoreBind {
		class OVDataloader;
	}

	class OVDataloader {
		friend class CoreBind::OVDataloader;

	public:
		static godot::TypedDictionary<godot::String, godot::PackedStringArray> get_author_info();
		static godot::TypedArray<godot::Dictionary> get_copyright_info();
		static godot::TypedDictionary<godot::String, godot::String> get_license_info();
		static godot::String get_license_text();
		static godot::Dictionary get_version_info();
	};
}
