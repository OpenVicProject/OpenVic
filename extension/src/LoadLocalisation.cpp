#include "LoadLocalisation.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/translation_server.hpp>

using namespace godot;
using namespace OpenVic2;

LoadLocalisation* LoadLocalisation::singleton = nullptr;

void LoadLocalisation::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_file", "file_path", "locale"), &LoadLocalisation::loadFile);
	ClassDB::bind_method(D_METHOD("load_locale_dir", "dir_path", "locale"), &LoadLocalisation::loadLocaleDir);
	ClassDB::bind_method(D_METHOD("load_localisation_dir", "dir_path"), &LoadLocalisation::loadLocalisationDir);
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

Error LoadLocalisation::_loadFileIntoTranslation(String const& filePath, Ref<Translation> translation) {
	Ref<FileAccess> file = FileAccess::open(filePath, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load localisation file: ", filePath);
		return err == OK ? FAILED : err;
	}
	int lineNumber = 0;
	while (!file->eof_reached()) {
		PackedStringArray line = file->get_csv_line();
		lineNumber++;
		if (line.size() < 2 || line[0].is_empty() || line[1].is_empty()) {
			if (!line[0].is_empty())
				UtilityFunctions::push_warning("Key \"", line[0], "\" missing value on line ", lineNumber, " in file: ", filePath);
			else if (line.size() >= 2 && !line[1].is_empty())
				UtilityFunctions::push_warning("Value \"", line[1], "\" missing key on line ", lineNumber, " in file: ", filePath);
			continue;
		}
		translation->add_message(line[0], line[1].c_unescape());
	}
	return OK;
}

Ref<Translation> LoadLocalisation::_getTranslation(String const& locale) {
	TranslationServer* server = TranslationServer::get_singleton();
	Ref<Translation> translation = server->get_translation_object(locale);
	if (translation.is_null() || translation->get_locale() != locale) {
		translation.instantiate();
		translation->set_locale(locale);
		server->add_translation(translation);
	}
	return translation;
}

Error LoadLocalisation::loadFile(String const& filePath, String const& locale) {
	return _loadFileIntoTranslation(filePath, _getTranslation(locale));
}

/* REQUIREMENTS
 * FS-18, FS-24, FS-25
 */
Error LoadLocalisation::loadLocaleDir(String const& dirPath, String const& locale) {
	Ref<Translation> translation = _getTranslation(locale);
	if (DirAccess::dir_exists_absolute(dirPath)) {
		Error err = OK;
		for (String const& fileName : DirAccess::get_files_at(dirPath)) {
			if (fileName.get_extension().to_lower() == "csv") {
				String filePath = dirPath.path_join(fileName);
				if (_loadFileIntoTranslation(filePath, translation) != OK)
					err = FAILED;
			}
		}
		return err;
	}
	UtilityFunctions::push_error("Locale directory does not exist: ", dirPath);
	return FAILED;
}

/* REQUIREMENTS
 * FS-23
 */
Error LoadLocalisation::loadLocalisationDir(String const& dirPath) {
	if (DirAccess::dir_exists_absolute(dirPath)) {
		TranslationServer* server = TranslationServer::get_singleton();
		Error err = OK;
		for (String const& localeName : DirAccess::get_directories_at(dirPath)) {
			if (localeName == server->standardize_locale(localeName)) {
				if (loadLocaleDir(dirPath.path_join(localeName), localeName) != OK)
					err = FAILED;
			} else {
				err = FAILED;
				UtilityFunctions::push_error("Invalid locale directory name: ", localeName);
			}
		}
		return err;
	}
	UtilityFunctions::push_error("Localisation directory does not exist: ", dirPath);
	return FAILED;
}
