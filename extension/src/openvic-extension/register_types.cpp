#include "register_types.hpp"

#include <godot_cpp/classes/engine.hpp>

#include "openvic-extension/classes/GFXButtonStateTexture.hpp"
#include "openvic-extension/classes/GFXSpriteTexture.hpp"
#include "openvic-extension/classes/GFXMaskedFlagTexture.hpp"
#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIOverlappingElementsBox.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/classes/MapMesh.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/Checksum.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/LoadLocalisation.hpp"

using namespace godot;
using namespace OpenVic;

static Checksum* _checksum_singleton;
static LoadLocalisation* _load_localisation;
static GameSingleton* _game_singleton;
static AssetManager* _asset_manager_singleton;

void initialize_openvic_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<Checksum>();
	_checksum_singleton = memnew(Checksum);
	Engine::get_singleton()->register_singleton("Checksum", Checksum::get_singleton());

	ClassDB::register_class<LoadLocalisation>();
	_load_localisation = memnew(LoadLocalisation);
	Engine::get_singleton()->register_singleton("LoadLocalisation", LoadLocalisation::get_singleton());

	ClassDB::register_class<GameSingleton>();
	_game_singleton = memnew(GameSingleton);
	Engine::get_singleton()->register_singleton("GameSingleton", GameSingleton::get_singleton());

	ClassDB::register_class<AssetManager>();
	_asset_manager_singleton = memnew(AssetManager);
	Engine::get_singleton()->register_singleton("AssetManager", AssetManager::get_singleton());

	ClassDB::register_class<MapMesh>();
	ClassDB::register_abstract_class<GFXCorneredTileSupportingTexture>();

	/* Depend on GFXCorneredTileSupportingTexture */
	ClassDB::register_class<GFXButtonStateTexture>();
	ClassDB::register_abstract_class<GFXButtonStateHavingTexture>();

	/* Depend on GFXButtonStateHavingTexture */
	ClassDB::register_class<GFXSpriteTexture>();
	ClassDB::register_class<GFXMaskedFlagTexture>();

	ClassDB::register_class<GFXPieChartTexture>();
	ClassDB::register_class<GUINode>();
	ClassDB::register_class<GUIOverlappingElementsBox>();
	ClassDB::register_class<GUIScrollbar>();
}

void uninitialize_openvic_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	Engine::get_singleton()->unregister_singleton("Checksum");
	memdelete(_checksum_singleton);

	Engine::get_singleton()->unregister_singleton("LoadLocalisation");
	memdelete(_load_localisation);

	Engine::get_singleton()->unregister_singleton("GameSingleton");
	memdelete(_game_singleton);

	Engine::get_singleton()->unregister_singleton("AssetManager");
	memdelete(_asset_manager_singleton);
}

extern "C" {
	// Initialization.
	GDExtensionBool GDE_EXPORT openvic_library_init(
		GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization* r_initialization
	) {
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(initialize_openvic_types);
		init_obj.register_terminator(uninitialize_openvic_types);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
