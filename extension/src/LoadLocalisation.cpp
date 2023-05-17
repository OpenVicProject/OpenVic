#include "LoadLocalisation.hpp"

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace OpenVic;

LoadLocalisation* LoadLocalisation::singleton = nullptr;

void LoadLocalisation::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_file", "file_path", "locale"), &LoadLocalisation::load_file);
	ClassDB::bind_method(D_METHOD("load_locale_dir", "dir_path", "locale"), &LoadLocalisation::load_locale_dir);
	ClassDB::bind_method(D_METHOD("load_localisation_dir", "dir_path"), &LoadLocalisation::load_localisation_dir);
}

LoadLocalisation* LoadLocalisation::get_singleton() {
	return singleton;
}

LoadLocalisation::LoadLocalisation() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

LoadLocalisation::~LoadLocalisation() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

Error LoadLocalisation::_load_file_into_translation(String const& file_path, Ref<Translation> translation) {
	Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load localisation file: ", file_path);
		return err == OK ? FAILED : err;
	}
	int line_number = 0;
	while (!file->eof_reached()) {
		PackedStringArray line = file->get_csv_line();
		line_number++;
		if (line.size() < 2 || line[0].is_empty() || line[1].is_empty()) {
			if (!line[0].is_empty())
				UtilityFunctions::push_warning("Key \"", line[0], "\" missing value on line ", line_number, " in file: ", file_path);
			else if (line.size() >= 2 && !line[1].is_empty())
				UtilityFunctions::push_warning("Value \"", line[1], "\" missing key on line ", line_number, " in file: ", file_path);
			continue;
		}
		translation->add_message(line[0], line[1].c_unescape());
	}
	return OK;
}

Ref<Translation> LoadLocalisation::_get_translation(String const& locale) {
	TranslationServer* server = TranslationServer::get_singleton();
	Ref<Translation> translation = server->get_translation_object(locale);
	if (translation.is_null() || translation->get_locale() != locale) {
		translation.instantiate();
		translation->set_locale(locale);
		server->add_translation(translation);
	}
	return translation;
}

Error LoadLocalisation::load_file(String const& file_path, String const& locale) {
	return _load_file_into_translation(file_path, _get_translation(locale));
}

/* REQUIREMENTS
 * FS-18, FS-24, FS-25
 */
Error LoadLocalisation::load_locale_dir(String const& dir_path, String const& locale) {
	Ref<Translation> translation = _get_translation(locale);
	if (DirAccess::dir_exists_absolute(dir_path)) {
		Error err = OK;
		for (String const& file_name : DirAccess::get_files_at(dir_path)) {
			if (file_name.get_extension().to_lower() == "csv") {
				String file_path = dir_path.path_join(file_name);
				if (_load_file_into_translation(file_path, translation) != OK)
					err = FAILED;
			}
		}
		return err;
	}
	UtilityFunctions::push_error("Locale directory does not exist: ", dir_path);
	return FAILED;
}

/* REQUIREMENTS
 * FS-23
 */
Error LoadLocalisation::load_localisation_dir(String const& dir_path) {
	if (DirAccess::dir_exists_absolute(dir_path)) {
		TranslationServer* server = TranslationServer::get_singleton();
		Error err = OK;
		for (String const& locale_name : DirAccess::get_directories_at(dir_path)) {
			if (locale_name == server->standardize_locale(locale_name)) {
				if (load_locale_dir(dir_path.path_join(locale_name), locale_name) != OK)
					err = FAILED;
			} else {
				err = FAILED;
				UtilityFunctions::push_error("Invalid locale directory name: ", locale_name);
			}
		}
		return err;
	}
	UtilityFunctions::push_error("Localisation directory does not exist: ", dir_path);
	return FAILED;
}
