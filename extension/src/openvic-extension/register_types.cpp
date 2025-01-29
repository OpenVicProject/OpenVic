#include "register_types.hpp"

#include <godot_cpp/classes/engine.hpp>

#include "openvic-extension/classes/GFXButtonStateTexture.hpp"
#include "openvic-extension/classes/GFXSpriteTexture.hpp"
#include "openvic-extension/classes/GFXMaskedFlagTexture.hpp"
#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUIButton.hpp"
#include "openvic-extension/classes/GUIIcon.hpp"
#include "openvic-extension/classes/GUIIconButton.hpp"
#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUILineChart.hpp"
#include "openvic-extension/classes/GUIListBox.hpp"
#include "openvic-extension/classes/GUIMaskedFlag.hpp"
#include "openvic-extension/classes/GUIMaskedFlagButton.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIOverlappingElementsBox.hpp"
#include "openvic-extension/classes/GUIPieChart.hpp"
#include "openvic-extension/classes/GUIProgressBar.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/classes/GUITextureRect.hpp"
#include "openvic-extension/classes/MapMesh.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/Checksum.hpp"
#include "openvic-extension/singletons/CursorSingleton.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/LoadLocalisation.hpp"
#include "openvic-extension/singletons/MapItemSingleton.hpp"
#include "openvic-extension/singletons/MenuSingleton.hpp"
#include "openvic-extension/singletons/ModelSingleton.hpp"
#include "openvic-extension/singletons/SoundSingleton.hpp"

using namespace godot;
using namespace OpenVic;

static Checksum* _checksum_singleton = nullptr;
static CursorSingleton* _cursor_singleton = nullptr;
static LoadLocalisation* _load_localisation = nullptr;
static GameSingleton* _game_singleton = nullptr;
static MapItemSingleton* _map_item_singleton = nullptr;
static MenuSingleton* _menu_singleton = nullptr;
static ModelSingleton* _model_singleton = nullptr;
static AssetManager* _asset_manager_singleton = nullptr;
static SoundSingleton* _sound_singleton = nullptr;

void initialize_openvic_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<Checksum>();
	_checksum_singleton = memnew(Checksum);
	Engine::get_singleton()->register_singleton("Checksum", Checksum::get_singleton());

	ClassDB::register_class<CursorSingleton>();
	_cursor_singleton = memnew(CursorSingleton);
	Engine::get_singleton()->register_singleton("CursorSingleton", CursorSingleton::get_singleton());

	ClassDB::register_class<LoadLocalisation>();
	_load_localisation = memnew(LoadLocalisation);
	Engine::get_singleton()->register_singleton("LoadLocalisation", LoadLocalisation::get_singleton());

	ClassDB::register_class<GameSingleton>();
	_game_singleton = memnew(GameSingleton);
	Engine::get_singleton()->register_singleton("GameSingleton", GameSingleton::get_singleton());

	ClassDB::register_class<MapItemSingleton>();
	_map_item_singleton = memnew(MapItemSingleton);
	Engine::get_singleton()->register_singleton("MapItemSingleton", MapItemSingleton::get_singleton());

	ClassDB::register_class<MenuSingleton>();
	_menu_singleton = memnew(MenuSingleton);
	Engine::get_singleton()->register_singleton("MenuSingleton", MenuSingleton::get_singleton());

	ClassDB::register_class<ModelSingleton>();
	_model_singleton = memnew(ModelSingleton);
	Engine::get_singleton()->register_singleton("ModelSingleton", ModelSingleton::get_singleton());

	ClassDB::register_class<AssetManager>();
	_asset_manager_singleton = memnew(AssetManager);
	Engine::get_singleton()->register_singleton("AssetManager", AssetManager::get_singleton());

	ClassDB::register_class<SoundSingleton>();
	_sound_singleton = memnew(SoundSingleton);
	Engine::get_singleton()->register_singleton("SoundSingleton", SoundSingleton::get_singleton());

	ClassDB::register_class<MapMesh>();
	ClassDB::register_abstract_class<GFXCorneredTileSupportingTexture>();

	/* Depend on GFXCorneredTileSupportingTexture */
	ClassDB::register_class<GFXButtonStateTexture>();
	ClassDB::register_abstract_class<GFXButtonStateHavingTexture>();

	/* Depend on GFXButtonStateHavingTexture */
	ClassDB::register_class<GFXSpriteTexture>();
	ClassDB::register_class<GFXMaskedFlagTexture>();

	ClassDB::register_class<GFXPieChartTexture>();
	ClassDB::register_class<GUIButton>();
	ClassDB::register_class<GUILabel>();
	ClassDB::register_class<GUILineChart>();
	ClassDB::register_class<GUIListBox>();
	ClassDB::register_class<GUINode>();
	ClassDB::register_class<GUIOverlappingElementsBox>();
	ClassDB::register_class<GUIPieChart>();
	ClassDB::register_class<GUIProgressBar>();
	ClassDB::register_class<GUIScrollbar>();
	ClassDB::register_class<GUITextureRect>();

	/* Depend on GUITextureRect */
	ClassDB::register_class<GUIIcon>();
	ClassDB::register_class<GUIMaskedFlag>();

	/* Depend on GUIButton */
	ClassDB::register_class<GUIIconButton>();
	ClassDB::register_class<GUIMaskedFlagButton>();
}

void uninitialize_openvic_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	Engine::get_singleton()->unregister_singleton("Checksum");
	memdelete(_checksum_singleton);

	Engine::get_singleton()->unregister_singleton("CursorSingleton");
	memdelete(_cursor_singleton);

	Engine::get_singleton()->unregister_singleton("LoadLocalisation");
	memdelete(_load_localisation);

	Engine::get_singleton()->unregister_singleton("GameSingleton");
	memdelete(_game_singleton);

	Engine::get_singleton()->unregister_singleton("MapItemSingleton");
	memdelete(_map_item_singleton);

	Engine::get_singleton()->unregister_singleton("MenuSingleton");
	memdelete(_menu_singleton);

	Engine::get_singleton()->unregister_singleton("ModelSingleton");
	memdelete(_model_singleton);

	Engine::get_singleton()->unregister_singleton("AssetManager");
	memdelete(_asset_manager_singleton);

	Engine::get_singleton()->unregister_singleton("SoundSingleton");
	memdelete(_sound_singleton);
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
