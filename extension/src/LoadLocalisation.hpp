#pragma once

#include <godot_cpp/classes/translation.hpp>

namespace OpenVic2 {
	class LoadLocalisation : public godot::Object {

		GDCLASS(LoadLocalisation, godot::Object)

		static LoadLocalisation* singleton;

		godot::Error _loadFileIntoTranslation(godot::String const& filePath, godot::Ref<godot::Translation> translation);
		godot::Ref<godot::Translation> _getTranslation(godot::String const& locale);

	protected:
		static void _bind_methods();

	public:
		static LoadLocalisation* get_singleton();

		LoadLocalisation();
		~LoadLocalisation();

		godot::Error loadFile(godot::String const& filePath, godot::String const& locale);
		godot::Error loadLocaleDir(godot::String const& dirPath, godot::String const& locale);
		godot::Error loadLocalisationDir(godot::String const& dirPath);
	};
}
