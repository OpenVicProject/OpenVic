#pragma once

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/string.hpp>

#include <openvic-simulation/utility/Getters.hpp>

namespace OpenVic {
	class StyleBoxWithSound : public godot::StyleBox {
		GDCLASS(StyleBoxWithSound, godot::StyleBox)

		godot::Ref<godot::StyleBox> style_box;
		godot::StringName audio_bus;
		godot::Ref<godot::AudioStream> audio_stream;
		float PROPERTY(audio_volume, 0);
		bool PROPERTY_CUSTOM_PREFIX(audio_disabled, is, false);

		static void _on_audio_finished(godot::AudioStreamPlayer* player);

	protected:
		static void _bind_methods();

		void _validate_property(godot::PropertyInfo& property) const;

	public:
		StyleBoxWithSound();

		godot::Ref<godot::StyleBox> get_style_box() const;
		void set_style_box(godot::Ref<godot::StyleBox> const& style);
		godot::StringName get_audio_bus() const;
		void set_audio_bus(godot::StringName const& bus);
		godot::Ref<godot::AudioStream> get_audio_stream() const;
		void set_audio_stream(godot::Ref<godot::AudioStream> const& stream);
		void set_audio_volume(float volume);
		void set_audio_disabled(bool disable);

		virtual godot::Rect2 _get_draw_rect(godot::Rect2 const& rect) const override;
		virtual void _draw(godot::RID const& canvas_item, godot::Rect2 const& rect) const override;
	};
}
