#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

namespace OpenVic::CoreBind {
	class OVGame : public godot::Object {
		GDCLASS(OVGame, godot::Object);

		static inline OVGame* singleton = nullptr;

	protected:
		static void _bind_methods();

	public:
		static OVGame* get_singleton() {
			return singleton;
		}

		godot::TypedDictionary<godot::String, godot::PackedStringArray> get_author_info() const;
		godot::TypedArray<godot::Dictionary> get_copyright_info() const;
		godot::TypedDictionary<godot::String, godot::String> get_license_info() const;
		godot::String get_license_text() const;
		godot::Dictionary get_version_info() const;

		OVGame();
		~OVGame();
	};

	class OVSimulation : public godot::Object {
		GDCLASS(OVSimulation, godot::Object);

		static inline OVSimulation* singleton = nullptr;

	protected:
		static void _bind_methods();

	public:
		static OVSimulation* get_singleton() {
			return singleton;
		}

		godot::TypedDictionary<godot::String, godot::PackedStringArray> get_author_info() const;
		godot::TypedArray<godot::Dictionary> get_copyright_info() const;
		godot::TypedDictionary<godot::String, godot::String> get_license_info() const;
		godot::String get_license_text() const;
		godot::Dictionary get_version_info() const;

		OVSimulation();
		~OVSimulation();
	};
}
