#include "GUILabel.hpp"

#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;
using namespace OpenVic::Utilities::literals;

static constexpr int32_t DEFAULT_FONT_SIZE = 16;

GUI_TOOLTIP_IMPLEMENTATIONS(GUILabel)

String const& GUILabel::get_colour_marker() {
	static const String COLOUR_MARKER = String::chr(0xA7); // §
	return COLOUR_MARKER;
}

String const& GUILabel::get_currency_marker() {
	static const String CURRENCY_MARKER = String::chr(0xA4); // ¤
	return CURRENCY_MARKER;
}

String const& GUILabel::get_substitution_marker() {
	static const String SUBSTITUTION_MARKER = String::chr(0x24); // $
	return SUBSTITUTION_MARKER;
}

String const& GUILabel::get_flag_marker() {
	static const String FLAG_MARKER = String::chr(0x40); // @
	return FLAG_MARKER;
}

void GUILabel::_bind_methods() {
	GUI_TOOLTIP_BIND_METHODS(GUILabel)

	OV_BIND_SMETHOD(get_colour_marker);
	OV_BIND_SMETHOD(get_currency_marker);
	OV_BIND_SMETHOD(get_substitution_marker);
	OV_BIND_SMETHOD(get_flag_marker);

	OV_BIND_METHOD(GUILabel::clear);
	OV_BIND_METHOD(GUILabel::get_gui_text_name);

	OV_BIND_METHOD(GUILabel::force_update_lines);

	OV_BIND_METHOD(GUILabel::get_text);
	OV_BIND_METHOD(GUILabel::set_text, { "new_text" });

	OV_BIND_METHOD(GUILabel::get_substitution_dict);
	OV_BIND_METHOD(GUILabel::add_substitution, { "key", "value" });
	OV_BIND_METHOD(GUILabel::set_substitution_dict, { "new_substitution_dict" });
	OV_BIND_METHOD(GUILabel::clear_substitutions);

	OV_BIND_METHOD(GUILabel::get_horizontal_alignment);
	OV_BIND_METHOD(GUILabel::set_horizontal_alignment, { "new_horizontal_alignment" });
	OV_BIND_METHOD(GUILabel::get_base_max_size);
	OV_BIND_METHOD(GUILabel::get_max_size);
	OV_BIND_METHOD(GUILabel::set_max_size, { "new_max_size" });
	OV_BIND_METHOD(GUILabel::get_border_size);
	OV_BIND_METHOD(GUILabel::set_border_size, { "new_border_size" });
	OV_BIND_METHOD(GUILabel::get_adjusted_rect);
	OV_BIND_METHOD(GUILabel::will_auto_adjust_to_content_size);
	OV_BIND_METHOD(GUILabel::set_auto_adjust_to_content_size, { "new_auto_adjust_to_content_size" });

	OV_BIND_METHOD(GUILabel::get_font);
	OV_BIND_METHOD(GUILabel::set_font, { "new_font" });
	OV_BIND_METHOD(GUILabel::set_font_file, { "new_font_file" });
	OV_BIND_METHOD(GUILabel::get_font_size);
	OV_BIND_METHOD(GUILabel::set_font_size, { "new_font_size" });
	OV_BIND_METHOD(GUILabel::get_default_colour);
	OV_BIND_METHOD(GUILabel::set_default_colour, { "new_default_colour" });
	OV_BIND_METHOD(GUILabel::get_currency_texture);

	OV_BIND_METHOD(GUILabel::get_background);
	OV_BIND_METHOD(GUILabel::set_background_texture, { "new_texture" });
	OV_BIND_METHOD(GUILabel::set_background_stylebox, { "new_stylebox_texture" });

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "substitution_dict"), "set_substitution_dict", "get_substitution_dict");
	ADD_PROPERTY(
		PropertyInfo(Variant::INT, "horizontal_alignment", PROPERTY_HINT_ENUM, "Left,Centre,Right,Fill"),
		"set_horizontal_alignment", "get_horizontal_alignment"
	);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "max_size", PROPERTY_HINT_NONE, "suffix:px"), "set_max_size", "get_max_size");
	ADD_PROPERTY(
		PropertyInfo(Variant::VECTOR2, "border_size", PROPERTY_HINT_NONE, "suffix:px"), "set_border_size", "get_border_size"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::RECT2, "adjusted_rect", PROPERTY_HINT_NONE, "suffix:px"), "", "get_adjusted_rect"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::BOOL, "auto_adjust_to_content_size"), "set_auto_adjust_to_content_size",
		"will_auto_adjust_to_content_size"
	);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "font", PROPERTY_HINT_RESOURCE_TYPE, "Font"), "set_font", "get_font");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size", PROPERTY_HINT_NONE, "suffix:px"), "set_font_size", "get_font_size");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_colour"), "set_default_colour", "get_default_colour");
	ADD_PROPERTY(
		PropertyInfo(Variant::OBJECT, "currency_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "", "get_currency_texture"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::OBJECT, "background", PROPERTY_HINT_RESOURCE_TYPE, "StyleBoxTexture"), "set_background_stylebox",
		"get_background"
	);
}

void GUILabel::_notification(int what) {
	_tooltip_notification(what);

	switch (what) {
	case NOTIFICATION_RESIZED:
	case NOTIFICATION_TRANSLATION_CHANGED: {
		_queue_line_update();
	} break;
	case NOTIFICATION_DRAW: {
		const RID ci = get_canvas_item();

		if (background.is_valid()) {
			draw_style_box(background, adjusted_rect);
		}

		if (font.is_null()) {
			return;
		}

		// Starting offset needed
		static const Vector2 base_offset { 1.0_real, -1.0_real };
		const Vector2 offset = base_offset + adjusted_rect.position + border_size;
		Vector2 position = offset;

		for (line_t const& line : lines) {
			position.x = offset.x;
			switch (horizontal_alignment) {
			case HORIZONTAL_ALIGNMENT_CENTER: {
				position.x += (adjusted_rect.size.width - 2 * border_size.width - line.width + 1.0_real) / 2.0_real;
			} break;
			case HORIZONTAL_ALIGNMENT_RIGHT: {
				position.x += adjusted_rect.size.width - 2 * border_size.width - line.width;
			} break;
			case HORIZONTAL_ALIGNMENT_LEFT:
			default:
				break;
			}

			position.y += font->get_ascent(font_size);

			for (segment_t const& segment : line.segments) {
				string_segment_t const* string_segment = std::get_if<string_segment_t>(&segment);

				if (string_segment == nullptr) {
					if (currency_texture.is_valid()) {
						currency_texture->draw(
							ci, position - Vector2 {
								1.0_real, static_cast<real_t>(currency_texture->get_height()) * 0.75_real
							}
						);
						position.x += currency_texture->get_width();
					}
				} else {
					font->draw_string(
						ci, position, string_segment->text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size,
						string_segment->colour
					);
					position.x += string_segment->width;
				}
			}

			position.y += font->get_descent(font_size);
		}

	} break;
	}
}

GUILabel::GUILabel()
  : tooltip_active { false },
	gui_text { nullptr },
	text {},
	substitution_dict {},
	horizontal_alignment { HORIZONTAL_ALIGNMENT_LEFT },
	max_size {},
	border_size {},
	adjusted_rect {},
	auto_adjust_to_content_size { false },
	font {},
	font_size { DEFAULT_FONT_SIZE },
	default_colour {},
	colour_codes { nullptr },
	currency_texture {},
	background {},
	lines {},
	line_update_queued { false } {}

void GUILabel::clear() {
	gui_text = nullptr;

	text = String {};
	substitution_dict.clear();
	horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
	max_size = {};
	border_size = {};
	adjusted_rect = {};
	auto_adjust_to_content_size = false;

	font.unref();
	font_size = DEFAULT_FONT_SIZE;
	default_colour = {};
	colour_codes = nullptr;
	currency_texture.unref();

	background.unref();
	lines.clear();

	line_update_queued = false;

	queue_redraw();
}

String GUILabel::get_gui_text_name() const {
	return gui_text != nullptr ? Utilities::std_to_godot_string(gui_text->get_name()) : String {};
}

Error GUILabel::set_gui_text(GUI::Text const* new_gui_text, GFX::Font::colour_codes_t const* override_colour_codes) {
	if (gui_text == new_gui_text) {
		return OK;
	}

	if (new_gui_text == nullptr) {
		clear();
		return OK;
	}

	gui_text = new_gui_text;

	set_text(Utilities::std_to_godot_string(gui_text->get_text()));

	using enum GUI::AlignedElement::format_t;
	static const ordered_map<GUI::AlignedElement::format_t, HorizontalAlignment> format_map {
		{ left, HORIZONTAL_ALIGNMENT_LEFT },
		{ centre, HORIZONTAL_ALIGNMENT_CENTER },
		{ right, HORIZONTAL_ALIGNMENT_RIGHT }
	};
	const decltype(format_map)::const_iterator it = format_map.find(gui_text->get_format());
	set_horizontal_alignment(it != format_map.end() ? it->second : HORIZONTAL_ALIGNMENT_LEFT);

	set_max_size(Utilities::to_godot_fvec2(gui_text->get_max_size()));
	set_border_size(Utilities::to_godot_fvec2(gui_text->get_border_size()));

	colour_codes = override_colour_codes != nullptr ? override_colour_codes : &gui_text->get_font()->get_colour_codes();
	set_default_colour(Utilities::to_godot_color(gui_text->get_font()->get_colour()));

	font.unref();
	font_size = DEFAULT_FONT_SIZE;
	currency_texture.unref();
	background.unref();

	Error err = OK;

	AssetManager* asset_manager = AssetManager::get_singleton();
	if (asset_manager != nullptr) {
		const StringName font_filepath = Utilities::std_to_godot_string(gui_text->get_font()->get_fontname());
		Ref<FontFile> font_file = asset_manager->get_font(font_filepath);
		if (font_file.is_valid()) {
			if (set_font_file(font_file) != OK) {
				err = FAILED;
			}
		} else {
			UtilityFunctions::push_error("Failed to load font \"", font_filepath, "\" for GUILabel");
			err = FAILED;
		}

		if (!gui_text->get_texture_file().empty()) {
			const StringName texture_path = Utilities::std_to_godot_string(gui_text->get_texture_file());
			Ref<ImageTexture> texture = asset_manager->get_texture(texture_path);
			if (texture.is_valid()) {
				set_background_texture(texture);
			} else {
				UtilityFunctions::push_error("Failed to load texture \"", texture_path, "\" for GUILabel ", get_name());
				err = FAILED;
			}
		}
	} else {
		UtilityFunctions::push_error("Failed to get AssetManager singleton for GUILabel");
		err = FAILED;
	}

	_queue_line_update();

	return err;
}

void GUILabel::force_update_lines() {
	line_update_queued = true;
	_update_lines();
}

void GUILabel::set_text(String const& new_text) {
	if (text != new_text) {
		text = new_text;

		_queue_line_update();
	}
}

void GUILabel::add_substitution(String const& key, String const& value) {
	Variant& existing_value = substitution_dict[key];
	if (existing_value != value) {
		existing_value = value;

		_queue_line_update();
	}
}

void GUILabel::set_substitution_dict(Dictionary const& new_substitution_dict) {
	substitution_dict = new_substitution_dict;
	_queue_line_update();
}

void GUILabel::clear_substitutions() {
	if (!substitution_dict.is_empty()) {
		substitution_dict.clear();

		_queue_line_update();
	}
}

void GUILabel::set_horizontal_alignment(HorizontalAlignment new_horizontal_alignment) {
	if (horizontal_alignment != new_horizontal_alignment) {
		horizontal_alignment = new_horizontal_alignment;

		_queue_line_update();
	}
}

Size2 GUILabel::get_base_max_size() const {
	return gui_text != nullptr ? Utilities::to_godot_fvec2(gui_text->get_max_size()) : Size2 {};
}

void GUILabel::set_max_size(Size2 new_max_size) {
	if (max_size != new_max_size) {
		max_size = new_max_size;

		set_custom_minimum_size(max_size);
		set_size(max_size);

		_queue_line_update();
	}
}

void GUILabel::set_border_size(Size2 new_border_size) {
	if (border_size != new_border_size) {
		border_size = new_border_size;

		update_stylebox_border_size();

		_queue_line_update();
	}
}

void GUILabel::set_auto_adjust_to_content_size(bool new_auto_adjust_to_content_size) {
	if (auto_adjust_to_content_size != new_auto_adjust_to_content_size) {
		auto_adjust_to_content_size = new_auto_adjust_to_content_size;

		adjust_to_content_size();

		queue_redraw();
	}
}

Ref<Font> GUILabel::get_font() const {
	return font;
}

void GUILabel::set_font(Ref<Font> const& new_font) {
	font = new_font;

	_queue_line_update();
}

Error GUILabel::set_font_file(Ref<FontFile> const& new_font_file) {
	ERR_FAIL_NULL_V(new_font_file, FAILED);

	set_font(new_font_file);

	return set_font_size(new_font_file->get_fixed_size());
}

Error GUILabel::set_font_size(int32_t new_font_size) {
	font_size = new_font_size;

	_queue_line_update();

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V_MSG(asset_manager, FAILED, "Failed to get AssetManager singleton for GUILabel");

	currency_texture = asset_manager->get_currency_texture(font_size);
	ERR_FAIL_NULL_V(currency_texture, FAILED);

	return OK;
}

void GUILabel::set_default_colour(Color const& new_default_colour) {
	if (default_colour != new_default_colour) {
		default_colour = new_default_colour;
		_queue_line_update();
	}
}

Ref<GFXSpriteTexture> GUILabel::get_currency_texture() const {
	return currency_texture;
}

Ref<StyleBoxTexture> GUILabel::get_background() const {
	return background;
}

void GUILabel::set_background_texture(Ref<Texture2D> const& new_texture) {
	Ref<StyleBoxTexture> new_background;

	if (new_texture.is_valid()) {
		new_background.instantiate();
		ERR_FAIL_NULL(new_background);

		new_background->set_texture(new_texture);
	}

	set_background_stylebox(new_background);
}

void GUILabel::set_background_stylebox(Ref<StyleBoxTexture> const& new_stylebox_texture) {
	if (background != new_stylebox_texture) {
		background = new_stylebox_texture;
		update_stylebox_border_size();
		queue_redraw();
	}
}

void GUILabel::update_stylebox_border_size() {
	if (background.is_valid()) {
		background->set_texture_margin(SIDE_LEFT, border_size.width);
		background->set_texture_margin(SIDE_RIGHT, border_size.width);
		background->set_texture_margin(SIDE_TOP, border_size.height);
		background->set_texture_margin(SIDE_BOTTOM, border_size.height);
	}
}

real_t GUILabel::get_string_width(String const& string) const {
	return font->get_string_size(string, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size).width;
}

real_t GUILabel::get_segment_width(segment_t const& segment) const {
	if (string_segment_t const* string_segment = std::get_if<string_segment_t>(&segment)) {
		return string_segment->width;
	} else if (currency_texture.is_valid()) {
		return currency_texture->get_width();
	} else {
		return 0.0_real;
	}
}

void GUILabel::_queue_line_update() {
	if (!line_update_queued) {
		line_update_queued = true;

		callable_mp(this, &GUILabel::_update_lines).call_deferred();
	}
}

void GUILabel::_update_lines() {
	if (!line_update_queued) {
		return;
	}

	line_update_queued = false;
	lines.clear();

	if (text.is_empty() || font.is_null()) {
		queue_redraw();
		return;
	}

	String const& base_text = is_auto_translating() ? tr(text) : text;

	String const& substituted_text = generate_substituted_text(base_text);

	auto const& [display_text, colour_instructions] = generate_display_text_and_colour_instructions(substituted_text);

	std::vector<line_t> unwrapped_lines = generate_lines_and_segments(display_text, colour_instructions);

	lines = wrap_lines(unwrapped_lines);

	adjust_to_content_size();

	queue_redraw();
}

String GUILabel::generate_substituted_text(String const& base_text) const {
	String result;
	int64_t start_pos = 0;
	int64_t marker_start_pos;

	while ((marker_start_pos = base_text.find(get_substitution_marker(), start_pos)) != -1) {
		result += base_text.substr(start_pos, marker_start_pos - start_pos);

		int64_t marker_end_pos = base_text.find(
			get_substitution_marker(), marker_start_pos + get_substitution_marker().length()
		);
		if (marker_end_pos == -1) {
			marker_end_pos = base_text.length();
		}

		String key = base_text.substr(
			marker_start_pos + get_substitution_marker().length(),
			marker_end_pos - marker_start_pos - get_substitution_marker().length()
		);
		String value = substitution_dict.get(key, String {});

		// Use the un-substituted key if no value is found or the value is empty
		if (value.is_empty()) {
			value = key;
		}

		// Translate the value if auto-translating (even if it's the key after a failed substitution)
		if (is_auto_translating()) {
			value = tr(value);
		}

		result += value;

		start_pos = marker_end_pos + get_substitution_marker().length();
	}

	if (start_pos < base_text.length()) {
		result += base_text.substr(start_pos);
	}

	return result;
}

std::pair<String, GUILabel::colour_instructions_t> GUILabel::generate_display_text_and_colour_instructions(
	String const& substituted_text
) const {
	String result;
	colour_instructions_t colour_instructions;
	int64_t start_pos = 0;
	int64_t marker_pos;

	while ((marker_pos = substituted_text.find(get_colour_marker(), start_pos)) != -1) {
		result += substituted_text.substr(start_pos, marker_pos - start_pos);

		if (marker_pos + get_colour_marker().length() < substituted_text.length()) {
			const char32_t colour_code = substituted_text[marker_pos + get_colour_marker().length()];

			// Check that the colour code can be safely cast to a char
			if (colour_code >> sizeof(char) * CHAR_BIT == 0) {
				colour_instructions.emplace_back(result.length(), static_cast<char>(colour_code));
			}

			start_pos = marker_pos + get_colour_marker().length() + 1;
		} else {
			return { std::move(result), std::move(colour_instructions) };
		}
	}

	result += substituted_text.substr(start_pos);

	return { std::move(result), std::move(colour_instructions) };
}

std::vector<GUILabel::line_t> GUILabel::generate_lines_and_segments(
	String const& display_text, colour_instructions_t const& colour_instructions
) const {
	static constexpr char RESET_COLOUR_CODE = '!';

	std::vector<line_t> unwrapped_lines;
	colour_instructions_t::const_iterator colour_it = colour_instructions.begin();
	Color current_colour = default_colour;
	int64_t section_start = 0;

	unwrapped_lines.emplace_back();

	for (int64_t idx = 0; idx < display_text.length(); ++idx) {
		Color new_colour = current_colour;
		while (colour_it != colour_instructions.end() && idx == colour_it->first) {
			if (colour_it->second == RESET_COLOUR_CODE) {
				new_colour = default_colour;
			} else {
				const GFX::Font::colour_codes_t::const_iterator it = colour_codes->find(colour_it->second);
				if (it != colour_codes->end()) {
					new_colour = Utilities::to_godot_color(it->second);
				}
			}
			++colour_it;
		}

		if (current_colour != new_colour) {
			if (section_start < idx) {
				separate_lines(
					display_text.substr(section_start, idx - section_start), current_colour, unwrapped_lines
				);
				section_start = idx;
			}
			current_colour = new_colour;
		}
	}

	if (section_start < display_text.length()) {
		separate_lines(display_text.substr(section_start), current_colour, unwrapped_lines);
	}

	return unwrapped_lines;
}

void GUILabel::separate_lines(
	String const& string, Color const& colour, std::vector<line_t>& unwrapped_lines
) const {
	static const String NEWLINE_MARKER = "\n";

	int64_t start_pos = 0;
	int64_t newline_pos;

	while ((newline_pos = string.find(NEWLINE_MARKER, start_pos)) != -1) {
		if (start_pos < newline_pos) {
			separate_currency_segments(string.substr(start_pos, newline_pos - start_pos), colour, unwrapped_lines.back());
		}

		unwrapped_lines.emplace_back();

		start_pos = newline_pos + NEWLINE_MARKER.length();
	}

	if (start_pos < string.length()) {
		separate_currency_segments(string.substr(start_pos), colour, unwrapped_lines.back());
	}
}

void GUILabel::separate_currency_segments(
	String const& string, Color const& colour, line_t& line
) const {
	const auto push_string_segment = [this, &string, &colour, &line](int64_t start, int64_t end) -> void {
		String substring = string.substr(start, end - start);
		const real_t width = get_string_width(substring);
		line.segments.emplace_back(string_segment_t { std::move(substring), colour, width });
		line.width += width;
	};

	int64_t start_pos = 0;
	int64_t marker_pos;

	const real_t currency_width = currency_texture.is_valid() ? currency_texture->get_width() : 0.0_real;

	while ((marker_pos = string.find(get_currency_marker(), start_pos)) != -1) {
		if (start_pos < marker_pos) {
			push_string_segment(start_pos, marker_pos);
		}

		line.segments.push_back(currency_segment_t {});
		line.width += currency_width;

		start_pos = marker_pos + get_currency_marker().length();
	}

	if (start_pos < string.length()) {
		push_string_segment(start_pos, string.length());
	}
}

std::vector<GUILabel::line_t> GUILabel::wrap_lines(std::vector<line_t>& unwrapped_lines) const {
	std::vector<line_t> wrapped_lines;

	const Size2 max_content_size = max_size - 2 * border_size;

	for (line_t& line : unwrapped_lines) {
		if (line.width <= max_content_size.width) {
			wrapped_lines.push_back(std::move(line));
		} else {
			line_t* current_line = &wrapped_lines.emplace_back();

			for (segment_t& segment : line.segments) {
				const real_t segment_width = get_segment_width(segment);

				if (current_line->width + segment_width <= max_content_size.width) {
					// Segement on current line
					current_line->segments.emplace_back(std::move(segment));
					current_line->width += segment_width;
				} else if (string_segment_t const* string_segment = std::get_if<string_segment_t>(&segment)) {
					// String segement wrapped onto new line
					static const String SPACE_MARKER = " ";

					String const& string = string_segment->text;

					int64_t start_pos = 0;

					while (start_pos < string.length()) {
						String whole_segment_string = string.substr(start_pos);
						real_t whole_segment_width = get_string_width(whole_segment_string);

						if (current_line->width + whole_segment_width > max_content_size.width) {
							String new_segment_string;
							real_t new_segment_width = 0.0_real;

							int64_t last_marker_pos = 0;
							int64_t marker_pos;

							while ((marker_pos = whole_segment_string.find(SPACE_MARKER, last_marker_pos)) != -1) {
								String substring = whole_segment_string.substr(0, marker_pos);
								const real_t width = get_string_width(substring);
								if (current_line->width + width <= max_content_size.width) {
									new_segment_string = std::move(substring);
									new_segment_width = width;
									last_marker_pos = marker_pos + SPACE_MARKER.length();
								} else {
									break;
								}
							}

							if (last_marker_pos != 0 || !current_line->segments.empty()) {
								if (!new_segment_string.is_empty()) {
									current_line->segments.emplace_back(string_segment_t {
										std::move(new_segment_string), string_segment->colour, new_segment_width
									});
									current_line->width += new_segment_width;
								}

								current_line = &wrapped_lines.emplace_back();

								start_pos += last_marker_pos;

								continue;
							}
						}
						current_line->segments.emplace_back(string_segment_t {
							std::move(whole_segment_string), string_segment->colour, whole_segment_width
						});
						current_line->width += whole_segment_width;
						break;
					}

				} else {
					// Currency segement on new line
					line_t* current_line = &wrapped_lines.emplace_back();
					current_line->segments.push_back(std::move(segment));
					current_line->width = segment_width;
				}
			}
		}
	}

	const auto is_over_max_height = [this, &wrapped_lines, &max_content_size]() -> bool {
		return wrapped_lines.size() > 1
			&& wrapped_lines.size() * font->get_height(font_size) > max_content_size.height;
	};

	if (is_over_max_height()) {
		do {
			wrapped_lines.pop_back();
		} while (is_over_max_height());

		static const String ELLIPSIS = "...";
		const real_t ellipsis_width = get_string_width(ELLIPSIS);

		line_t& last_line = wrapped_lines.back();
		Color last_colour = default_colour;

		while (last_line.segments.size() > 0 && last_line.width + ellipsis_width > max_content_size.width) {
			if (string_segment_t* string_segment = std::get_if<string_segment_t>(&last_line.segments.back())) {
				last_colour = string_segment->colour;

				String& last_string = string_segment->text;
				if (last_string.length() > 1) {
					last_string = last_string.substr(0, last_string.length() - 1);

					last_line.width -= string_segment->width;
					string_segment->width = get_string_width(last_string);
					last_line.width += string_segment->width;
				} else {
					last_line.width -= string_segment->width;
					last_line.segments.pop_back();
				}
			} else {
				last_line.width -= currency_texture->get_width();
				last_line.segments.pop_back();
			}
		}

		last_line.segments.push_back(string_segment_t { ELLIPSIS, last_colour, ellipsis_width });
		last_line.width += ellipsis_width;
	}

	return wrapped_lines;
}

void GUILabel::adjust_to_content_size() {
	if (auto_adjust_to_content_size) {
		adjusted_rect = {};

		for (line_t const& line : lines) {
			if (adjusted_rect.size.width < line.width) {
				adjusted_rect.size.width = line.width;
			}
		}

		adjusted_rect.size.height = lines.size() * font->get_height(font_size);

		adjusted_rect.size += 2 * border_size;

		switch (horizontal_alignment) {
		case HORIZONTAL_ALIGNMENT_CENTER: {
			adjusted_rect.position.x = (max_size.width - adjusted_rect.size.width + 1.0_real) / 2.0_real;
		} break;
		case HORIZONTAL_ALIGNMENT_RIGHT: {
			adjusted_rect.position.x = max_size.width - adjusted_rect.size.width;
		} break;
		case HORIZONTAL_ALIGNMENT_LEFT:
		default:
			break;
		}
	} else {
		adjusted_rect = { {}, max_size };
	}
}
