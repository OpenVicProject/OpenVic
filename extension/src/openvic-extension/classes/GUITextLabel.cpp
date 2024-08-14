#include "GUITextLabel.hpp"

#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;
using namespace OpenVic::Utilities::literals;

void GUITextLabel::_bind_methods() {
	OV_BIND_METHOD(GUITextLabel::clear);

	OV_BIND_METHOD(GUITextLabel::get_text);
	OV_BIND_METHOD(GUITextLabel::set_text, { "new_text" });

	OV_BIND_METHOD(GUITextLabel::get_substitution_dict);
	OV_BIND_METHOD(GUITextLabel::add_substitution, { "key", "value" });
	OV_BIND_METHOD(GUITextLabel::set_substitution_dict, { "new_substitution_dict" });
	OV_BIND_METHOD(GUITextLabel::clear_substitutions);

	OV_BIND_METHOD(GUITextLabel::get_max_lines);
	OV_BIND_METHOD(GUITextLabel::set_max_lines, { "new_max_lines" });

	OV_BIND_METHOD(GUITextLabel::get_alignment);
	OV_BIND_METHOD(GUITextLabel::get_gui_text_name);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "substitution_dict"), "set_substitution_dict", "get_substitution_dict");
}

void GUITextLabel::_notification(int what) {
	switch (what) {
	case NOTIFICATION_TRANSLATION_CHANGED: {
		_queue_update();
	} break;
	}
}

GUITextLabel::GUITextLabel()
  : gui_text { nullptr }, alignment { HORIZONTAL_ALIGNMENT_LEFT }, font_height { 0.0_real }, colour_codes { nullptr },
	max_lines { 1 }, update_queued { false } {
	set_scroll_active(false);
	set_clip_contents(false);
	set_autowrap_mode(TextServer::AUTOWRAP_ARBITRARY);
}

void GUITextLabel::clear() {
	gui_text = nullptr;

	text = String {};
	substitution_dict.clear();
	alignment = HORIZONTAL_ALIGNMENT_LEFT;
	max_lines = 1;

	static const StringName normal_theme = "normal";
	remove_theme_stylebox_override(normal_theme);

	_update_font();

	update_queued = false;
}

Error GUITextLabel::set_gui_text(GUI::Text const* new_gui_text) {
	if (gui_text == new_gui_text) {
		return OK;
	}

	if (new_gui_text == nullptr) {
		clear();
		return OK;
	}

	gui_text = new_gui_text;

	text = Utilities::std_to_godot_string(gui_text->get_text());

	using enum GUI::AlignedElement::format_t;
	static const ordered_map<GUI::AlignedElement::format_t, HorizontalAlignment> format_map {
		{ left, HORIZONTAL_ALIGNMENT_LEFT },
		{ centre, HORIZONTAL_ALIGNMENT_CENTER },
		{ right, HORIZONTAL_ALIGNMENT_RIGHT }
	};
	const decltype(format_map)::const_iterator it = format_map.find(gui_text->get_format());
	alignment = it != format_map.end() ? it->second : HORIZONTAL_ALIGNMENT_LEFT;

	// TODO - detect max_lines based on gui_text? E.g. from total height vs line height?
	max_lines = 1;

	static const Vector2 default_padding { 1.0_real, -1.0_real };
	const Vector2 border_size = Utilities::to_godot_fvec2(gui_text->get_border_size()) + default_padding;
	const Vector2 max_size = Utilities::to_godot_fvec2(gui_text->get_max_size());
	set_position(get_position() + border_size);
	set_custom_minimum_size(max_size - 2 * border_size);

	_queue_update();

	Error err = _update_font();

	if (!gui_text->get_texture_file().empty()) {
		AssetManager* asset_manager = AssetManager::get_singleton();
		ERR_FAIL_NULL_V(asset_manager, FAILED);

		const StringName texture_path = Utilities::std_to_godot_string(gui_text->get_texture_file());
		Ref<ImageTexture> texture = asset_manager->get_texture(texture_path);
		ERR_FAIL_NULL_V_MSG(
			texture, FAILED, vformat("Failed to load texture \"%s\" for GUITextLabel %s", texture_path, get_name())
		);

		Ref<StyleBoxTexture> stylebox;
		stylebox.instantiate();
		ERR_FAIL_NULL_V(stylebox, FAILED);
		stylebox->set_texture(texture);

		stylebox->set_texture_margin(SIDE_LEFT, border_size.x);
		stylebox->set_texture_margin(SIDE_RIGHT, border_size.x);
		stylebox->set_texture_margin(SIDE_TOP, border_size.y);
		stylebox->set_texture_margin(SIDE_BOTTOM, border_size.y);

		static const StringName normal_theme = "normal";
		add_theme_stylebox_override(normal_theme, stylebox);
	}

	return err;
}

String GUITextLabel::get_gui_text_name() const {
	return gui_text != nullptr ? Utilities::std_to_godot_string(gui_text->get_name()) : String {};
}

void GUITextLabel::set_text(godot::String const& new_text) {
	if (text != new_text) {
		text = new_text;
		_queue_update();
	}
}

void GUITextLabel::add_substitution(String const& key, godot::String const& value) {
	Variant& existing_value = substitution_dict[key];
	if (existing_value != value) {
		existing_value = value;
		_queue_update();
	}
}

void GUITextLabel::set_substitution_dict(godot::Dictionary const& new_substitution_dict) {
	substitution_dict = new_substitution_dict;
	_queue_update();
}

void GUITextLabel::clear_substitutions() {
	substitution_dict.clear();
	_queue_update();
}

void GUITextLabel::set_max_lines(int32_t new_max_lines) {
	if (new_max_lines != max_lines) {
		max_lines = new_max_lines;
		_queue_update();
	}
}

Error GUITextLabel::_update_font() {
	static const StringName font_theme = "normal_font";
	static const StringName font_color_theme = "default_color";

	if (gui_text == nullptr || gui_text->get_font() == nullptr) {
		remove_theme_font_override(font_theme);
		remove_theme_color_override(font_color_theme);
		font_height = 0.0_real;
		colour_codes = nullptr;

		return OK;
	}

	add_theme_color_override(font_color_theme, Utilities::to_godot_color(gui_text->get_font()->get_colour()));
	colour_codes = &gui_text->get_font()->get_colour_codes();

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V_MSG(asset_manager, FAILED, "Failed to get AssetManager singleton for GUITextLabel");

	const StringName font_file = Utilities::std_to_godot_string(gui_text->get_font()->get_fontname());
	const Ref<Font> font = asset_manager->get_font(font_file);

	ERR_FAIL_NULL_V_MSG(font, FAILED, vformat("Failed to load font \"%s\" for GUITextLabel", font_file));

	add_theme_font_override(font_theme, font);
	font_height = font->get_height();

	return OK;
}

void GUITextLabel::_queue_update() {
	if (!update_queued) {
		update_queued = true;

		callable_mp(this, &GUITextLabel::_update_text).call_deferred();
	}
}

void GUITextLabel::_update_text() {
	static constexpr char SUBSTITUTION_CHAR = '$';
	static constexpr char COLOUR_CHAR       = '\xA7'; // §

	String const& base_text = is_auto_translating() ? tr(text) : text;

	// Remove $keys$ and insert substitutions
	String substituted_text;
	{
		bool substitution_section = false;
		int64_t section_start = 0;
		for (int64_t idx = 0; idx < base_text.length(); ++idx) {
			if (static_cast<char>(base_text[idx]) == SUBSTITUTION_CHAR) {
				if (section_start < idx) {
					String section = base_text.substr(section_start, idx - section_start);
					if (substitution_section) {
						section = substitution_dict.get(section, String {});
					}
					substituted_text += section;
				}
				substitution_section = !substitution_section;
				section_start = idx + 1;
			}
		}
		if (!substitution_section && section_start < base_text.length()) {
			substituted_text += base_text.substr(section_start);
		}
	}

	// Separate out colour codes from displayed test
	String display_text;
	colour_instructions_t colour_instructions;
	{
		int64_t section_start = 0;
		for (int64_t idx = 0; idx < substituted_text.length(); ++idx) {
			if (static_cast<char>(substituted_text[idx]) == COLOUR_CHAR) {
				if (idx > section_start) {
					display_text += substituted_text.substr(section_start, idx - section_start);
				}
				if (++idx < substituted_text.length() && colour_codes != nullptr) {
					colour_instructions.emplace_back(display_text.length(), static_cast<char>(substituted_text[idx]));
				}
				section_start = idx + 1;
			}
		}
		if (section_start < substituted_text.length()) {
			display_text += substituted_text.substr(section_start);
		}
	}

	_generate_text(display_text, colour_instructions);

	// Trim and add ellipsis if text is too long
	if (max_lines > 0 && max_lines < get_line_count()) {
		int32_t visible_character_count = 0;
		while (
			visible_character_count < get_total_character_count() &&
			get_character_line(visible_character_count) < max_lines
		) {
			++visible_character_count;
		}
		static const String ellipsis = "...";
		if (visible_character_count > ellipsis.length()) {
			_generate_text(
				display_text.substr(0, visible_character_count - ellipsis.length()) + ellipsis, colour_instructions
			);
		}
	}

	update_queued = false;
}

void GUITextLabel::_generate_text(String const& display_text, colour_instructions_t const& colour_instructions) {
	static constexpr char RESET_COLOUR_CHAR = '!';
	static constexpr char CURRENCY_CHAR     = '\xA4'; // ¤

	AssetManager const* asset_manager = AssetManager::get_singleton();
	Ref<GFXSpriteTexture> const& currency_texture =
		asset_manager != nullptr ? asset_manager->get_currency_texture(font_height) : Ref<GFXSpriteTexture> {};

	RichTextLabel::clear();

	push_paragraph(alignment);

	// Add text, applying colour codes and inserting currency symbols
	{
		colour_instructions_t::const_iterator colour_it = colour_instructions.begin();
		bool has_colour = false;
		int64_t section_start = 0;
		for (int64_t idx = 0; idx < display_text.length(); ++idx) {
			if (colour_it != colour_instructions.end() && idx == colour_it->first) {
				if (section_start < idx) {
					add_text(display_text.substr(section_start, idx - section_start));
					section_start = idx;
				}
				if (colour_it->second == RESET_COLOUR_CHAR) {
					if (has_colour) {
						pop();
						has_colour = false;
					}
				} else {
					const GFX::Font::colour_codes_t::const_iterator it = colour_codes->find(colour_it->second);
					if (it != colour_codes->end()) {
						if (has_colour) {
							pop();
						} else {
							has_colour = true;
						}
						push_color(Utilities::to_godot_color(it->second));
					}
				}
				++colour_it;
			}
			if (static_cast<char>(display_text[idx]) == CURRENCY_CHAR) {
				if (section_start < idx) {
					add_text(display_text.substr(section_start, idx - section_start));
				}
				if (currency_texture.is_valid()) {
					add_image(currency_texture);
				} else {
					static const String currency_fallback = "£";
					add_text(currency_fallback);
				}
				section_start = idx + 1;
			}
		}
		if (section_start < display_text.length()) {
			add_text(display_text.substr(section_start));
		}
	}
}
