#pragma once

#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/classes/resource_format_saver.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace OpenVic {
	class GameSettings : public godot::Resource {
		GDCLASS(GameSettings, godot::Resource);

		godot::Ref<godot::ConfigFile> config;
		godot::Ref<godot::ConfigFile> defaults;

	protected:
		static void _bind_methods();

	public:
		void reset_settings();

		godot::Error save(godot::String path = godot::String());
		godot::Error load(godot::String path = godot::String());

		static godot::Ref<GameSettings> load_from_file(godot::String path);

		void set_value(godot::String section, godot::String key, godot::Variant value);
		godot::Variant get_value(godot::String section, godot::String key, godot::Variant default_value = nullptr);

		bool has_section(godot::String section) const;
		bool has_section_key(godot::String section, godot::String key) const;

		void reset_section(godot::String section);
		void reset_section_key(godot::String section, godot::String key);

		godot::PackedStringArray get_sections() const;
		godot::PackedStringArray get_section_keys(godot::String section) const;

		godot::Error load_deprecated_file(godot::String path, godot::Dictionary section_keys);

		GameSettings();
	};
}
