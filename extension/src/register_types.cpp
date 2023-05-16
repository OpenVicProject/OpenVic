#include "register_types.h"

#include <godot_cpp/classes/engine.hpp>

#include "Checksum.hpp"
#include "GameSingleton.hpp"
#include "LoadLocalisation.hpp"
#include "MapMesh.hpp"

using namespace godot;
using namespace OpenVic;

static Checksum* _checksum;
static LoadLocalisation* _load_localisation;
static GameSingleton* _map_singleton;

void initialize_openvic_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<Checksum>();
	_checksum = memnew(Checksum);
	Engine::get_singleton()->register_singleton("Checksum", Checksum::get_singleton());

	ClassDB::register_class<LoadLocalisation>();
	_load_localisation = memnew(LoadLocalisation);
	Engine::get_singleton()->register_singleton("LoadLocalisation", LoadLocalisation::get_singleton());

	ClassDB::register_class<GameSingleton>();
	_map_singleton = memnew(GameSingleton);
	Engine::get_singleton()->register_singleton("GameSingleton", GameSingleton::get_singleton());

	ClassDB::register_class<MapMesh>();
}

void uninitialize_openvic_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	Engine::get_singleton()->unregister_singleton("Checksum");
	memdelete(_checksum);

	Engine::get_singleton()->unregister_singleton("LoadLocalisation");
	memdelete(_load_localisation);

	Engine::get_singleton()->unregister_singleton("GameSingleton");
	memdelete(_map_singleton);
}

extern "C" {
	// Initialization.
	GDExtensionBool GDE_EXPORT openvic_library_init(GDExtensionInterface const* p_interface, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
		GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

		init_obj.register_initializer(initialize_openvic_types);
		init_obj.register_terminator(uninitialize_openvic_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
