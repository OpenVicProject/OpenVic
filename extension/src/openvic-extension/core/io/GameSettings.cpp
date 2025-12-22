#include "GameSettings.hpp"

#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "openvic-extension/core/Bind.hpp"

using namespace OpenVic;
using namespace godot;

GameSettings::GameSettings() {
	config.instantiate();
	defaults.instantiate();
}

void GameSettings::reset_settings() {
	config = defaults;
	emit_changed();
}

Error GameSettings::save(String path) {
	if (path.is_empty()) {
		path = get_path();
	}
	for (String const& section : defaults->get_sections()) {
		for (String const& key : defaults->get_section_keys(section)) {
			if (!has_section_key(section, key)) {
				config->set_value(section, key, defaults->get_value(section, key));
			}
		}
	}
	Error err = config->save(path);
	return err;
}

Error GameSettings::load(String path) {
	if (path.is_empty()) {
		path = get_path();
	} else {
		set_path(path);
	}

	Error err = config->load(path);
	emit_changed();
	return err;
}

Ref<GameSettings> GameSettings::load_from_file(String path) {
	Ref<GameSettings> settings;
	if (ResourceLoader::get_singleton()->has_cached(path)) {
		settings = ResourceLoader::get_singleton()->get_cached_ref(path);
		if (settings.is_valid()) {
			return settings;
		}
	}

	settings.instantiate();
	if (!FileAccess::file_exists(path)) {
		if (String dir = path.get_base_dir(); !DirAccess::dir_exists_absolute(dir)) {
			DirAccess::make_dir_recursive_absolute(dir);
		}

		ERR_FAIL_COND_V(settings->save(path) != OK, Ref<GameSettings>());
	} else {
		ERR_FAIL_COND_V(settings->load(path) != OK, Ref<GameSettings>());
	}
	return settings;
}

void GameSettings::set_value(String section, String key, Variant value) {
	if (value == Variant()) {
		config->set_value(section, key, defaults->get_value(section, key));
		return;
	}

	config->set_value(section, key, value);
}

Variant GameSettings::get_value(String section, String key, Variant default_value) {
	if (default_value != Variant()) {
		if (defaults->has_section_key(section, key)) {
			ERR_FAIL_COND_V_MSG(
				defaults->get_value(section, key) != default_value, nullptr,
				vformat("Setting '%s.%s' default value does match previously set default value.", section, key)
			);
		} else {
			defaults->set_value(section, key, default_value);
		}
	} else if (defaults->has_section_key(section, key)) {
		default_value = defaults->get_value(section, key);
	}

	if (!has_section_key(section, key)) {
		set_value(section, key, default_value);
		return default_value;
	}

	return config->get_value(section, key);
}

bool GameSettings::has_section(String section) const {
	return config->has_section(section);
}

bool GameSettings::has_section_key(String section, String key) const {
	return config->has_section_key(section, key);
}

void GameSettings::reset_section(String section) {
	set_block_signals(true);
	for (String const& key : config->get_section_keys(section)) {
		reset_section_key(section, key);
	}
	set_block_signals(false);
	emit_changed();
}

void GameSettings::reset_section_key(String section, String key) {
	config->set_value(section, key, defaults->get_value(section, key));
	emit_changed();
}

PackedStringArray GameSettings::get_sections() const {
	return config->get_sections();
}

PackedStringArray GameSettings::get_section_keys(String section) const {
	return config->get_section_keys(section);
}

Error GameSettings::load_deprecated_file(String path, Dictionary section_keys) {
	Ref<ConfigFile> config;
	config.instantiate();
	if (Error err = config->load(path); err != OK) {
		return err;
	}

	Array sections = section_keys.keys();
	for (size_t i = 0; i < sections.size(); i++) {
		Variant section = sections[i];
		const Variant::Type section_type = section.get_type();
		ERR_FAIL_COND_V(section_type != Variant::STRING && section_type != Variant::STRING_NAME, ERR_INVALID_PARAMETER);

		Variant key = section_keys[section];
		switch (key.get_type()) {
			using enum Variant::Type;
		case NIL:
			for (String const& key : config->get_section_keys(section)) {
				set_value(section, key, config->get_value(section, key));
			}
			break;
		case STRING:
		case STRING_NAME:
			if (config->has_section_key(section, key) && !has_section_key(section, key)) {
				set_value(section, key, config->get_value(section, key));
			}
			break;

		case PACKED_STRING_ARRAY: {
			PackedStringArray array = key;
			for (String const& k : array) {
				if (config->has_section_key(section, k) && !has_section_key(section, k)) {
					set_value(section, k, config->get_value(section, k));
				}
			}
			break;
		}

		case ARRAY: {
			Array array = key;
			for (size_t arr_i = 0; arr_i < array.size(); arr_i++) {
				Variant const& inner_key = array[arr_i];
				Variant::Type inner_key_type = inner_key.get_type();
				ERR_FAIL_COND_V(
					inner_key_type != Variant::STRING && inner_key_type != Variant::STRING_NAME, ERR_INVALID_PARAMETER
				);

				String const& k = inner_key;
				if (config->has_section_key(section, k) && !has_section_key(section, k)) {
					set_value(section, k, config->get_value(section, k));
				}
			}
			break;
		}

		default: //
			ERR_FAIL_V(ERR_INVALID_PARAMETER);
		}
	}
	emit_changed();

	return OK;
}

void GameSettings::_bind_methods() {
	OV_BIND_METHOD(GameSettings::reset_settings);

	OV_BIND_METHOD(GameSettings::save, { "path" }, DEFVAL(String()));
	OV_BIND_METHOD(GameSettings::load, { "path" }, DEFVAL(String()));

	OV_BIND_SMETHOD(GameSettings::load_from_file, { "path" });

	OV_BIND_METHOD(GameSettings::set_value, { "section", "key", "value" });
	OV_BIND_METHOD(GameSettings::get_value, { "section", "key", "default" }, DEFVAL(nullptr));

	OV_BIND_METHOD(GameSettings::has_section, { "section" });
	OV_BIND_METHOD(GameSettings::has_section_key, { "section", "key" });

	OV_BIND_METHOD(GameSettings::reset_section, { "section" });
	OV_BIND_METHOD(GameSettings::reset_section_key, { "section", "key" });

	OV_BIND_METHOD(GameSettings::get_sections);
	OV_BIND_METHOD(GameSettings::get_section_keys, { "section" });

	OV_BIND_METHOD(GameSettings::load_deprecated_file, { "path", "section_keys" });
}
