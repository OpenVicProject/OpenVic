#include "register_types.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>

#include "TestSingleton.hpp"
#include "Simulation.hpp"

using namespace godot;
using namespace OpenVic2;

static TestSingleton* _test_singleton;
static Simulation* _simulation;

void initialize_openvic2_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<TestSingleton>();
	_test_singleton = memnew(TestSingleton);
	Engine::get_singleton()->register_singleton("TestSingleton", TestSingleton::get_singleton());

	ClassDB::register_class<Simulation>();
	_simulation = memnew(Simulation);
	Engine::get_singleton()->register_singleton("Simulation", Simulation::get_singleton());

}

void uninitialize_openvic2_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	Engine::get_singleton()->unregister_singleton("TestSingleton");
	memdelete(_test_singleton);

	Engine::get_singleton()->unregister_singleton("Simulation");
	memdelete(_test_singleton);
}

extern "C"
{

	// Initialization.

	GDExtensionBool GDE_EXPORT openvic2_library_init(const GDExtensionInterface *p_interface, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

		init_obj.register_initializer(initialize_openvic2_types);
		init_obj.register_terminator(uninitialize_openvic2_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}