#include "register_core_types.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "openvic-extension/core/ArgumentParser.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/core/io/GameSettings.hpp"

using namespace OpenVic;
using namespace godot;

static ArgumentParser* _argument_parser = nullptr;

void OpenVic::register_core_types() {
	StaticStrings::create();

	GDREGISTER_CLASS(ArgumentParser);
	GDREGISTER_CLASS(ArgumentOption);
	GDREGISTER_CLASS(GameSettings);

	_argument_parser = memnew(ArgumentParser);
	Engine::get_singleton()->register_singleton("ArgumentParser", ArgumentParser::get_singleton());
}

void OpenVic::unregister_core_types() {
	Engine::get_singleton()->unregister_singleton("ArgumentParser");
	memdelete(_argument_parser);

	StaticStrings::free();
}
