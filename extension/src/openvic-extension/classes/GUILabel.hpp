#pragma once

#include <godot_cpp/classes/atlas_texture.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/style_box_texture.hpp>

#include <openvic-simulation/interface/GUI.hpp>
#include <openvic-simulation/types/Signal.hpp>

#include "openvic-extension/classes/GFXSpriteTexture.hpp"
#include "openvic-extension/classes/GUIHasTooltip.hpp"

namespace godot {
	struct NodePath;
	struct StringName;
}

namespace OpenVic {
	struct GovernmentType;

	class GUILabel : public godot::Control, public observer {
		GDCLASS(GUILabel, godot::Control)

		GUI_TOOLTIP_DEFINITIONS

		using colour_instructions_t = std::vector<std::pair<int64_t, char>>;

		GUI::Text const* PROPERTY(gui_text, nullptr);

		godot::String PROPERTY(text);
		godot::Dictionary PROPERTY(substitution_dict);
		godot::HorizontalAlignment PROPERTY(horizontal_alignment, godot::HORIZONTAL_ALIGNMENT_LEFT);
		godot::Size2 PROPERTY(max_size); // Actual max size is max_size - 2 * border_size
		godot::Size2 PROPERTY(border_size); // The padding between the Nodes bounding box and the text within it
		godot::Rect2 PROPERTY(adjusted_rect); // Offset + size after adjustment to fit content size
		bool PROPERTY_CUSTOM_PREFIX(auto_adjust_to_content_size, will, false);

		godot::Ref<godot::Font> font;
		int32_t PROPERTY(font_size);
		godot::Color PROPERTY(default_colour);
		GFX::Font::colour_codes_t const* colour_codes = nullptr;
		godot::Ref<GFXSpriteTexture> currency_texture;

		godot::Ref<godot::StyleBoxTexture> background;

		struct string_segment_t {
			godot::String text;
			godot::Color colour;
			real_t width;

			string_segment_t(godot::String&& new_text, godot::Color const& new_colour, real_t new_width);
			string_segment_t(godot::String const& new_text, godot::Color const& new_colour, real_t new_width);
			string_segment_t(string_segment_t&&) = default;
		};
		using currency_segment_t = std::monostate;
		using flag_segment_t = godot::Ref<godot::AtlasTexture>;
		using segment_t = std::variant<string_segment_t, currency_segment_t, flag_segment_t>;
		struct line_t {
			std::vector<segment_t> segments;
			real_t width {};
		};

		std::vector<line_t> lines;

		bool line_update_queued = false;

	protected:
		static void _bind_methods();

		void _notification(int what);

	public:
		static godot::String const& get_colour_marker();
		static godot::String const& get_currency_marker();
		static godot::String const& get_substitution_marker();
		static godot::String const& get_flag_marker();
		static void set_text_and_tooltip(
			GUINode const& parent,
			godot::NodePath const& path,
			godot::StringName const& text_localisation_key,
			godot::StringName const& tooltip_localisation_key
		);

		GUILabel();
		~GUILabel() override;

		/* Reset gui_text to nullptr and reset current text. */
		void clear();
		/* Return the name of the GUI::Text, or an empty String if it's null. */
		godot::String get_gui_text_name() const;
		/* Set the GUI::Text. */
		godot::Error set_gui_text(
			GUI::Text const* new_gui_text, GFX::Font::colour_codes_t const* override_colour_codes = nullptr
		);

		void force_update_lines();

		void set_text(godot::String const& new_text);

		void add_substitution(godot::String const& key, godot::String const& value);
		void set_substitution_dict(godot::Dictionary const& new_substitution_dict);
		void clear_substitutions();

		void set_horizontal_alignment(godot::HorizontalAlignment new_horizontal_alignment);
		godot::Size2 get_base_max_size() const;
		void set_max_size(godot::Size2 new_max_size);
		void set_border_size(godot::Size2 new_border_size);
		void set_auto_adjust_to_content_size(bool new_auto_adjust_to_content_size);

		godot::Ref<godot::Font> get_font() const;
		void set_font(godot::Ref<godot::Font> const& new_font);
		godot::Error set_font_file(godot::Ref<godot::FontFile> const& new_font_file);
		godot::Error set_font_size(int32_t new_font_size);
		void set_default_colour(godot::Color const& new_default_colour);

		godot::Ref<GFXSpriteTexture> get_currency_texture() const;

		godot::Ref<godot::StyleBoxTexture> get_background() const;
		void set_background_texture(godot::Ref<godot::Texture2D> const& new_texture);
		void set_background_stylebox(godot::Ref<godot::StyleBoxTexture> const& new_stylebox_texture);

	private:
		void update_stylebox_border_size();
		real_t get_string_width(godot::String const& string) const;
		real_t get_segment_width(segment_t const& segment) const;

		void _queue_line_update();
		void _update_lines();
		void on_flag_government_type_changed();

		godot::String generate_substituted_text(godot::String const& base_text) const;
		std::pair<godot::String, colour_instructions_t> generate_display_text_and_colour_instructions(
			godot::String const& substituted_text
		) const;
		std::vector<line_t> generate_lines_and_segments(
			godot::String const& display_text, colour_instructions_t const& colour_instructions
		);
		void separate_lines(
			godot::String const& string, godot::Color const& colour, std::vector<line_t>& lines
		);
		void separate_currency_segments(
			godot::String const& string, godot::Color const& colour, line_t& line
		);
		flag_segment_t make_flag_segment(godot::String const& identifier);
		void separate_flag_segments(
			godot::String const& string, godot::Color const& colour, line_t& line
		);
		std::vector<line_t> wrap_lines(std::vector<line_t>& unwrapped_lines) const;
		void adjust_to_content_size();
	};
}
