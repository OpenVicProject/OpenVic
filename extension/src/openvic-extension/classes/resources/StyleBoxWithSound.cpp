#include "StyleBoxWithSound.hpp"

#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/StaticString.hpp"

using namespace OpenVic;
using namespace godot;

void StyleBoxWithSound::_bind_methods() {
	OV_BIND_METHOD(StyleBoxWithSound::get_style_box);
	OV_BIND_METHOD(StyleBoxWithSound::set_style_box, { "style" });
	OV_BIND_METHOD(StyleBoxWithSound::get_audio_bus);
	OV_BIND_METHOD(StyleBoxWithSound::set_audio_bus, { "bus" });
	OV_BIND_METHOD(StyleBoxWithSound::get_audio_stream);
	OV_BIND_METHOD(StyleBoxWithSound::set_audio_stream, { "stream" });
	OV_BIND_METHOD(StyleBoxWithSound::get_audio_volume);
	OV_BIND_METHOD(StyleBoxWithSound::set_audio_volume, { "volume" });
	OV_BIND_METHOD(StyleBoxWithSound::is_audio_disabled);
	OV_BIND_METHOD(StyleBoxWithSound::set_audio_disabled, { "disabled" });

	ADD_PROPERTY(
		PropertyInfo(Variant::OBJECT, "style_box", PROPERTY_HINT_RESOURCE_TYPE, "StyleBox"), "set_style_box", "get_style_box"
	);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "audio_bus", PROPERTY_HINT_ENUM, ""), "set_audio_bus", "get_audio_bus");
	ADD_PROPERTY(
		PropertyInfo(Variant::OBJECT, "audio_stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), //
		"set_audio_stream", "get_audio_stream"
	);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "audio_volume"), "set_audio_volume", "get_audio_volume");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "audio_disabled"), "set_audio_disabled", "is_audio_disabled");
}

StyleBoxWithSound::StyleBoxWithSound() {
	audio_bus = OV_SNAME(Master);
}

Ref<StyleBox> StyleBoxWithSound::get_style_box() const {
	return style_box;
}

void StyleBoxWithSound::set_style_box(Ref<StyleBox> const& style) {
	style_box = style;
	emit_changed();
}

StringName StyleBoxWithSound::get_audio_bus() const {
	return audio_bus;
}

void StyleBoxWithSound::set_audio_bus(StringName const& bus) {
	audio_bus = bus;
	emit_changed();
}

Ref<AudioStream> StyleBoxWithSound::get_audio_stream() const {
	return audio_stream;
}

void StyleBoxWithSound::set_audio_stream(Ref<AudioStream> const& stream) {
	audio_stream = stream;
	emit_changed();
}

void StyleBoxWithSound::set_audio_volume(float volume) {
	audio_volume = volume;
	emit_changed();
}

void StyleBoxWithSound::set_audio_disabled(bool disable) {
	audio_disabled = disable;
	emit_changed();
}

godot::Rect2 StyleBoxWithSound::_get_draw_rect(Rect2 const& rect) const {
	if (style_box.is_null()) {
		return Rect2();
	}
	return style_box->_get_draw_rect(rect);
}

void StyleBoxWithSound::_on_audio_finished(AudioStreamPlayer* player) {
	player->queue_free();
}

void StyleBoxWithSound::_draw(RID const& canvas_item, Rect2 const& rect) const {
	if (!audio_disabled) {
		SceneTree* tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
		if (tree != nullptr) {
			if (audio_stream.is_valid()) {
				AudioStreamPlayer* asp = memnew(AudioStreamPlayer);
				asp->connect("finished", callable_mp_static(&StyleBoxWithSound::_on_audio_finished).bind(asp));
				asp->set_bus(audio_bus);
				asp->set_stream(audio_stream);
				asp->set_volume_db(audio_volume);
				asp->set_autoplay(true);
				tree->get_root()->add_child(asp, false, Node::INTERNAL_MODE_BACK);
			}
		}
	}
	if (style_box.is_valid()) {
		style_box->draw(canvas_item, rect);
	}
}

void StyleBoxWithSound::_validate_property(PropertyInfo& property) const {
	static StringName audio_bus_name = "audio_bus";
	if (property.name == audio_bus_name) {
		String options;
		for (int i = 0; i < AudioServer::get_singleton()->get_bus_count(); i++) {
			if (i > 0) {
				options += ",";
			}
			String name = AudioServer::get_singleton()->get_bus_name(i);
			options += name;
		}

		property.hint_string = options;
	}
}
