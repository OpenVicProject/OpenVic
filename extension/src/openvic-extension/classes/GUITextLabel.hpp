#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/style_box_texture.hpp>

#include <openvic-simulation/interface/GUI.hpp>

#include "openvic-extension/classes/GFXSpriteTexture.hpp"

namespace OpenVic {
	class GUITextLabel : public godot::Control {
		GDCLASS(GUITextLabel, godot::Control)

		using colour_instructions_t = std::vector<std::pair<int64_t, char>>;

		GUI::Text const* PROPERTY(gui_text);

		godot::String PROPERTY(text);
		godot::Dictionary PROPERTY(substitution_dict);
		godot::HorizontalAlignment PROPERTY(alignment);
		godot::Vector2 border_size;

		godot::Ref<godot::FontFile> font;
		godot::Color default_colour;
		GFX::Font::colour_codes_t const* colour_codes;
		godot::Ref<GFXSpriteTexture> currency_texture;

		godot::Ref<godot::StyleBoxTexture> PROPERTY(background);

		struct string_segment_t {
			godot::String text;
			godot::Color colour;
			real_t width;
		};
		using currency_segment_t = std::monostate;
		using segment_t = std::variant<string_segment_t, currency_segment_t>;
		struct line_t {
			std::vector<segment_t> segments;
			real_t width {};
		};

		std::vector<line_t> lines;

		bool line_update_queued;

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		GUITextLabel();

		/* Reset gui_text to nullptr and reset current text. */
		void clear();

		/* Set the GUI::Text. */
		godot::Error set_gui_text(
			GUI::Text const* new_gui_text, GFX::Font::colour_codes_t const* override_colour_codes = nullptr
		);

		/* Return the name of the GUI::Text, or an empty String if it's null. */
		godot::String get_gui_text_name() const;

		void set_text(godot::String const& new_text);
		void add_substitution(godot::String const& key, godot::String const& value);
		void set_substitution_dict(godot::Dictionary const& new_substitution_dict);
		void clear_substitutions();

	private:
		godot::Vector2 get_content_max_size() const;

		godot::Error _update_font(GFX::Font::colour_codes_t const* override_colour_codes);
		real_t get_string_width(godot::String const& string) const;
		real_t get_segment_width(segment_t const& segment) const;

		void _queue_line_update();
		void _update_lines();

		godot::String generate_substituted_text(godot::String const& base_text) const;
		std::pair<godot::String, colour_instructions_t> generate_display_text_and_colour_instructions(
			godot::String const& substituted_text
		) const;
		std::vector<line_t> generate_lines_and_segments(
			godot::String const& display_text, colour_instructions_t const& colour_instructions
		) const;
		void separate_lines(
			godot::String const& string, godot::Color const& colour, std::vector<line_t>& lines
		) const;
		void separate_currency_segments(
			godot::String const& string, godot::Color const& colour, line_t& line
		) const;
		std::vector<line_t> wrap_lines(std::vector<line_t>& unwrapped_lines) const;
	};
}
