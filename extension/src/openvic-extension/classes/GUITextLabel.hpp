#pragma once

#include <godot_cpp/classes/rich_text_label.hpp>

#include <openvic-simulation/interface/GUI.hpp>

namespace OpenVic {
	class GUITextLabel : public godot::RichTextLabel {
		GDCLASS(GUITextLabel, godot::RichTextLabel)

		using colour_instructions_t = std::vector<std::pair<int64_t, char>>;

		GUI::Text const* PROPERTY(gui_text);

		godot::String PROPERTY(text);
		godot::Dictionary PROPERTY(substitution_dict);
		godot::HorizontalAlignment PROPERTY(alignment);
		real_t font_height;
		GFX::Font::colour_codes_t const* colour_codes;
		int32_t PROPERTY(max_lines);

		bool update_queued;

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		GUITextLabel();

		/* Reset gui_text to nullptr and reset current text. */
		void clear();

		/* Set the GUI::Text. */
		godot::Error set_gui_text(GUI::Text const* new_gui_text);

		/* Return the name of the GUI::Text, or an empty String if it's null. */
		godot::String get_gui_text_name() const;

		void set_text(godot::String const& new_text);
		void add_substitution(godot::String const& key, godot::String const& value);
		void set_substitution_dict(godot::Dictionary const& new_substitution_dict);
		void clear_substitutions();

		/* Any text going over this number of lines will be trimmed and replaced with an ellipsis.
		 * Values less than 1 indicate no limit. Default value: 1. */
		void set_max_lines(int32_t new_max_lines);

	private:
		godot::Error _update_font();

		void _queue_update();
		void _update_text();
		void _generate_text(godot::String const& display_text, colour_instructions_t const& colour_instructions);
	};
}
