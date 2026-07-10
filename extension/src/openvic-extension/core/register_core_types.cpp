#include "register_core_types.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "openvic-extension/core/ArgumentParser.hpp"
#include "openvic-extension/core/config/config_bind.hpp"

using namespace OpenVic;
using namespace godot;

static ArgumentParser* _argument_parser = nullptr;
static CoreBind::OVGame* _ov_game = nullptr;
static CoreBind::OVSimulation* _ov_simulation = nullptr;

void OpenVic::register_core_types() {
	GDREGISTER_CLASS(ArgumentParser);
	GDREGISTER_CLASS(ArgumentOption);

	GDREGISTER_CLASS(CoreBind::OVGame);
	GDREGISTER_CLASS(CoreBind::OVSimulation);

	_argument_parser = memnew(ArgumentParser);
	Engine::get_singleton()->register_singleton("ArgumentParser", ArgumentParser::get_singleton());

	_ov_game = memnew(CoreBind::OVGame);
	Engine::get_singleton()->register_singleton("OVGame", CoreBind::OVGame::get_singleton());

	_ov_simulation = memnew(CoreBind::OVSimulation);
	Engine::get_singleton()->register_singleton("OVSimulation", CoreBind::OVSimulation::get_singleton());
}

void OpenVic::unregister_core_types() {
	Engine::get_singleton()->unregister_singleton("ArgumentParser");
	memdelete(_argument_parser);

	Engine::get_singleton()->unregister_singleton("OVGame");
	memdelete(_ov_game);

	Engine::get_singleton()->unregister_singleton("OVSimulation");
	memdelete(_ov_simulation);
}
