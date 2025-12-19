#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <openvic-simulation/core/Typedefs.hpp>

namespace OpenVic {
	class ArgumentOption : public godot::RefCounted {
		GDCLASS(ArgumentOption, godot::RefCounted);

		godot::StringName name;
		godot::String option_arguments;
		godot::Variant default_value;
		godot::String description;
		godot::PackedStringArray aliases;
		godot::HashSet<godot::String> aliases_set;

		static godot::Variant _get_empty_value_for(godot::Variant::Type p_type);

		ArgumentOption(
			godot::StringName const& name, godot::Variant const& default_value, godot::String const& description,
			godot::PackedStringArray const& aliases, godot::String option_args
		);

	protected:
		static void _bind_methods();

	public:
		godot::StringName get_name() const;

		godot::String get_option_arguments() const;

		godot::PackedStringArray get_aliases() const;
		bool has_alias(godot::String const& p_alias) const;

		godot::Variant::Type get_type() const;

		godot::Variant get_default_value() const;

		godot::String get_description() const;

		godot::String get_help_string(bool p_is_rich = false) const;

		static godot::Ref<ArgumentOption> create(
			godot::StringName const& p_name, godot::Variant::Type p_type, godot::String const& p_description,
			godot::PackedStringArray const& p_aliases = {}, godot::String p_option_args = godot::String()
		);

		static godot::Ref<ArgumentOption> create_with_default(
			godot::StringName const& p_name, godot::Variant const& p_default, godot::String const& p_description,
			godot::PackedStringArray const& p_aliases = {}, godot::String p_option_args = godot::String()
		);

		ArgumentOption() {}
	};

	class ArgumentParser : public godot::Object {
		GDCLASS(ArgumentParser, godot::Object);

		inline static ArgumentParser* singleton = nullptr;

		mutable godot::String _help_string;
		mutable godot::String _rich_help_string;

		godot::LocalVector<godot::Ref<ArgumentOption>> options;
		godot::HashMap<godot::StringName, godot::Variant> arguments;

		decltype(options)::Iterator _find_option(godot::String const& p_arg_name);

		godot::Variant _parse_value(
			godot::StringName const& p_arg_name, godot::String const& p_value_string, ArgumentOption const* p_option = nullptr
		);

	protected:
		static void _bind_methods();

	public:
		OV_ALWAYS_INLINE static ArgumentParser* get_singleton() {
			return singleton;
		}

		bool has_option(godot::StringName const& p_arg_name, bool p_include_aliases = false) const;
		bool is_option_set(godot::StringName const& p_arg_name, bool p_include_aliases = false) const;
		godot::Variant get_option_value(godot::StringName const& p_arg_name) const;
		void set_option_value(godot::StringName const& p_arg_name, godot::Variant p_value);

		godot::String get_help(bool p_is_rich = false) const;

		godot::Error parse_arguments(godot::PackedStringArray const& p_args, bool p_error_unknown = true);

		ArgumentParser();
		~ArgumentParser();
	};
}
