#include "LoadLocalisation.hpp"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

void LoadLocalisation::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_file", "file_path", "locale"), &LoadLocalisation::load_file);
	ClassDB::bind_method(D_METHOD("load_locale_dir", "dir_path", "locale"), &LoadLocalisation::load_locale_dir);
	ClassDB::bind_method(D_METHOD("load_localisation_dir", "dir_path"), &LoadLocalisation::load_localisation_dir);
}

LoadLocalisation* LoadLocalisation::get_singleton() {
	return _singleton;
}

LoadLocalisation::LoadLocalisation() {
	ERR_FAIL_COND(_singleton != nullptr);
	_singleton = this;
}

LoadLocalisation::~LoadLocalisation() {
	ERR_FAIL_COND(_singleton != this);
	_singleton = nullptr;
}

Error LoadLocalisation::_load_file(String const& file_path, Ref<Translation> translation) const {
	const Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load localisation file: ", file_path);
		return err == OK ? FAILED : err;
	}
	int line_number = 0;
	while (!file->eof_reached()) {
		static const String delimeter = ";";
		const PackedStringArray line = file->get_csv_line(delimeter);
		line_number++;
		if (line.size() < 2 || line[0].is_empty() || line[1].is_empty()) {
			if (!line[0].is_empty()) {
				UtilityFunctions::push_warning(
					"Key \"", line[0], "\" missing value on line ", line_number, " in file: ", file_path
				);
				err = FAILED;
			} else if (line.size() >= 2 && !line[1].is_empty()) {
				UtilityFunctions::push_warning(
					"Value \"", line[1], "\" missing key on line ", line_number, " in file: ", file_path
				);
				err = FAILED;
			}
			continue;
		}
		translation->add_message(line[0], line[1].c_unescape());
	}
	return err;
}

Ref<Translation> LoadLocalisation::_get_translation(String const& locale) const {
	TranslationServer* server = TranslationServer::get_singleton();
	ERR_FAIL_NULL_V(server, nullptr);
	Ref<Translation> translation = server->get_translation_object(locale);
	if (translation.is_null() || translation->get_locale() != locale) {
		translation.instantiate();
		translation->set_locale(locale);
		server->add_translation(translation);
	}
	return translation;
}

Error LoadLocalisation::load_file(String const& file_path, String const& locale) const {
	return _load_file(file_path, _get_translation(locale));
}

/* REQUIREMENTS
 * FS-18, FS-24, FS-25
 */
Error LoadLocalisation::load_locale_dir(String const& dir_path, String const& locale) const {
	if (!DirAccess::dir_exists_absolute(dir_path)) {
		UtilityFunctions::push_error("Locale directory does not exist: ", dir_path);
		return FAILED;
	}
	/* This will add the locale to the list of loaded locales even if it has no
	 * localisation files - this is useful for testing other aspects of localisation
	 * such as number formatting and text direction. To disable this behaviour and
	 * only show non-empty localisations, move the `_get_translation` call to after
	 * the `files.size()` check.
	 */
	const Ref<Translation> translation = _get_translation(locale);
	const PackedStringArray files = DirAccess::get_files_at(dir_path);
	if (files.size() < 1) {
		UtilityFunctions::push_error("Locale directory does not contain any files: ", dir_path);
		return FAILED;
	}
	Error err = OK;
	for (String const& file_name : files) {
		if (file_name.get_extension().to_lower() == "csv") {
			if (_load_file(dir_path.path_join(file_name), translation) != OK) {
				err = FAILED;
			}
		}
	}
	return err;
}

/* REQUIREMENTS
 * FS-23
 */
Error LoadLocalisation::load_localisation_dir(String const& dir_path) const {
	if (!DirAccess::dir_exists_absolute(dir_path)) {
		UtilityFunctions::push_error("Localisation directory does not exist: ", dir_path);
		return FAILED;
	}
	PackedStringArray const dirs = DirAccess::get_directories_at(dir_path);
	if (dirs.size() < 1) {
		UtilityFunctions::push_error("Localisation directory does not contain any sub-directories: ", dir_path);
		return FAILED;
	}
	TranslationServer* server = TranslationServer::get_singleton();
	ERR_FAIL_NULL_V(server, FAILED);
	Error err = OK;
	for (String const& locale_name : dirs) {
		if (locale_name != server->standardize_locale(locale_name)) {
			UtilityFunctions::push_error("Invalid locale directory name: ", locale_name);
		} else if (load_locale_dir(dir_path.path_join(locale_name), locale_name) == OK) {
			continue;
		}
		err = FAILED;
	}
	return err;
}
bool LoadLocalisation::add_message(std::string_view key, Dataloader::locale_t locale, std::string_view localisation) {
	Ref<Translation>& translation = translations[locale];
	if (translation.is_null()) {
		translation = _singleton->_get_translation(Dataloader::locale_names[locale]);
		if (translation.is_null()) {
			UtilityFunctions::push_error("Failed to get translation object: ", Dataloader::locale_names[locale]);
			return false;
		}
	}
	const StringName godot_key = Utilities::std_view_to_godot_string_name(key);
	const StringName godot_localisation = Utilities::std_view_to_godot_string_name(localisation);
	if (0) {
		const StringName old_localisation = translation->get_message(godot_key);
		if (!old_localisation.is_empty()) {
			UtilityFunctions::push_warning(
				"Changing translation ", godot_key, " (", Dataloader::locale_names[locale], ") from \"",
				old_localisation, "\" to \"", godot_localisation, "\""
			);
		}
	}
	translation->add_message(godot_key, godot_localisation);
	return true;
}
