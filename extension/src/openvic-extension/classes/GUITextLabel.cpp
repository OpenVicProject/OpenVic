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

	OV_BIND_METHOD(GUITextLabel::get_alignment);
	OV_BIND_METHOD(GUITextLabel::get_gui_text_name);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "substitution_dict"), "set_substitution_dict", "get_substitution_dict");
}

void GUITextLabel::_notification(int what) {
	switch (what) {
	case NOTIFICATION_RESIZED:
	case NOTIFICATION_TRANSLATION_CHANGED: {
		_queue_line_update();
	} break;
	case NOTIFICATION_DRAW: {
		const RID ci = get_canvas_item();
		const Size2 size = get_size();

		if (background.is_valid()) {
			draw_style_box(background, Rect2 { {}, size });
		}

		if (font.is_null()) {
			return;
		}

		const Vector2 content_max_size = get_content_max_size();

		// Starting offset needed
		static const Vector2 base_offset { 1.0_real, -1.0_real };
		const Vector2 offset = base_offset + border_size;
		Vector2 position = offset;

		for (line_t const& line : lines) {
			position.x = offset.x;
			switch (alignment) {
			case HORIZONTAL_ALIGNMENT_CENTER: {
				position.x += (content_max_size.width - line.width + 1.0_real) / 2.0_real;
			} break;
			case HORIZONTAL_ALIGNMENT_RIGHT: {
				position.x += content_max_size.width - line.width;
			} break;
			case HORIZONTAL_ALIGNMENT_LEFT:
			default:
				break;
			}

			position.y += font->get_ascent(font->get_fixed_size());

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
						ci, position, string_segment->text, HORIZONTAL_ALIGNMENT_LEFT, -1, font->get_fixed_size(),
						string_segment->colour
					);
					position.x += string_segment->width;
				}
			}

			position.y += font->get_descent(font->get_fixed_size());
		}

	} break;
	}
}

GUITextLabel::GUITextLabel()
  : gui_text { nullptr }, alignment { HORIZONTAL_ALIGNMENT_LEFT }, colour_codes { nullptr }, line_update_queued { false } {}

void GUITextLabel::clear() {
	gui_text = nullptr;

	text = String {};
	substitution_dict.clear();
	alignment = HORIZONTAL_ALIGNMENT_LEFT;
	border_size = {};

	_update_font(nullptr);

	background.unref();
	lines.clear();

	line_update_queued = false;

	queue_redraw();
}

Error GUITextLabel::set_gui_text(GUI::Text const* new_gui_text, GFX::Font::colour_codes_t const* override_colour_codes) {
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

	set_custom_minimum_size(Utilities::to_godot_fvec2(gui_text->get_max_size()));
	border_size = Utilities::to_godot_fvec2(gui_text->get_border_size());

	_queue_line_update();

	Error err = _update_font(override_colour_codes);

	background.unref();

	if (!gui_text->get_texture_file().empty()) {
		AssetManager* asset_manager = AssetManager::get_singleton();
		ERR_FAIL_NULL_V(asset_manager, FAILED);

		const StringName texture_path = Utilities::std_to_godot_string(gui_text->get_texture_file());
		Ref<ImageTexture> texture = asset_manager->get_texture(texture_path);
		ERR_FAIL_NULL_V_MSG(
			texture, FAILED, vformat("Failed to load texture \"%s\" for GUITextLabel %s", texture_path, get_name())
		);

		background.instantiate();
		ERR_FAIL_NULL_V(background, FAILED);
		background->set_texture(texture);

		background->set_texture_margin(SIDE_LEFT, border_size.x);
		background->set_texture_margin(SIDE_RIGHT, border_size.x);
		background->set_texture_margin(SIDE_TOP, border_size.y);
		background->set_texture_margin(SIDE_BOTTOM, border_size.y);
	}

	return err;
}

String GUITextLabel::get_gui_text_name() const {
	return gui_text != nullptr ? Utilities::std_to_godot_string(gui_text->get_name()) : String {};
}

void GUITextLabel::set_text(String const& new_text) {
	if (text != new_text) {
		text = new_text;
		_queue_line_update();
	}
}

void GUITextLabel::add_substitution(String const& key, String const& value) {
	Variant& existing_value = substitution_dict[key];
	if (existing_value != value) {
		existing_value = value;
		_queue_line_update();
	}
}

void GUITextLabel::set_substitution_dict(Dictionary const& new_substitution_dict) {
	substitution_dict = new_substitution_dict;
	_queue_line_update();
}

void GUITextLabel::clear_substitutions() {
	if (!substitution_dict.is_empty()) {
		substitution_dict.clear();
		_queue_line_update();
	}
}

Vector2 GUITextLabel::get_content_max_size() const {
	return get_size() - 2 * border_size;
}

Error GUITextLabel::_update_font(GFX::Font::colour_codes_t const* override_colour_codes) {
	if (gui_text == nullptr || gui_text->get_font() == nullptr) {
		font.unref();
		default_colour = {};
		colour_codes = nullptr;
		currency_texture.unref();

		return OK;
	}

	default_colour = Utilities::to_godot_color(gui_text->get_font()->get_colour());
	colour_codes = override_colour_codes != nullptr ? override_colour_codes : &gui_text->get_font()->get_colour_codes();

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V_MSG(asset_manager, FAILED, "Failed to get AssetManager singleton for GUITextLabel");

	const StringName font_file = Utilities::std_to_godot_string(gui_text->get_font()->get_fontname());
	font = asset_manager->get_font(font_file);
	ERR_FAIL_NULL_V_MSG(font, FAILED, vformat("Failed to load font \"%s\" for GUITextLabel", font_file));

	currency_texture = asset_manager->get_currency_texture(font->get_fixed_size());
	ERR_FAIL_NULL_V(currency_texture, FAILED);

	return OK;
}

real_t GUITextLabel::get_string_width(String const& string) const {
	return font->get_string_size(string, HORIZONTAL_ALIGNMENT_LEFT, -1, font->get_fixed_size()).x;
}

real_t GUITextLabel::get_segment_width(segment_t const& segment) const {
	if (string_segment_t const* string_segment = std::get_if<string_segment_t>(&segment)) {
		return string_segment->width;
	} else if (currency_texture.is_valid()) {
		return currency_texture->get_width();
	} else {
		return 0.0_real;
	}
}

void GUITextLabel::_queue_line_update() {
	if (!line_update_queued) {
		line_update_queued = true;

		callable_mp(this, &GUITextLabel::_update_lines).call_deferred();
	}
}

void GUITextLabel::_update_lines() {
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

	// TODO - trim and add ellipsis if necessary

	queue_redraw();
}

String GUITextLabel::generate_substituted_text(String const& base_text) const {
	static const String SUBSTITUTION_MARKER = String::chr(0x24); // $

	String result;
	int64_t start_pos = 0;
	int64_t marker_start_pos;

	while ((marker_start_pos = base_text.find(SUBSTITUTION_MARKER, start_pos)) != -1) {
		result += base_text.substr(start_pos, marker_start_pos - start_pos);

		int64_t marker_end_pos = base_text.find(SUBSTITUTION_MARKER, marker_start_pos + SUBSTITUTION_MARKER.length());
		if (marker_end_pos == -1) {
			return result;
		}

		String key = base_text.substr(
			marker_start_pos + SUBSTITUTION_MARKER.length(), marker_end_pos - marker_start_pos - SUBSTITUTION_MARKER.length()
		);
		String value = substitution_dict.get(key, String {});

		result += value;

		start_pos = marker_end_pos + SUBSTITUTION_MARKER.length();
	}

	result += base_text.substr(start_pos);

	return result;
}

std::pair<String, GUITextLabel::colour_instructions_t> GUITextLabel::generate_display_text_and_colour_instructions(
	String const& substituted_text
) const {
	static const String COLOUR_MARKER = String::chr(0xA7); // §

	String result;
	colour_instructions_t colour_instructions;
	int64_t start_pos = 0;
	int64_t marker_pos;

	while ((marker_pos = substituted_text.find(COLOUR_MARKER, start_pos)) != -1) {
		result += substituted_text.substr(start_pos, marker_pos - start_pos);

		if (marker_pos + COLOUR_MARKER.length() < substituted_text.length()) {
			const char32_t colour_code = substituted_text[marker_pos + COLOUR_MARKER.length()];

			// Check that the colour code can be safely cast to a char
			if (colour_code >> sizeof(char) * CHAR_BIT == 0) {
				colour_instructions.emplace_back(result.length(), static_cast<char>(colour_code));
			}

			start_pos = marker_pos + COLOUR_MARKER.length() + 1;
		} else {
			return { std::move(result), std::move(colour_instructions) };
		}
	}

	result += substituted_text.substr(start_pos);

	return { std::move(result), std::move(colour_instructions) };
}

std::vector<GUITextLabel::line_t> GUITextLabel::generate_lines_and_segments(
	String const& display_text, colour_instructions_t const& colour_instructions
) const {
	static constexpr char RESET_COLOUR_CODE = '!';

	std::vector<line_t> unwrapped_lines;
	colour_instructions_t::const_iterator colour_it = colour_instructions.begin();
	Color current_colour = default_colour;
	int64_t section_start = 0;

	unwrapped_lines.emplace_back();

	for (int64_t idx = 0; idx < display_text.length(); ++idx) {
		if (colour_it != colour_instructions.end() && idx == colour_it->first) {
			Color new_colour = current_colour;
			if (colour_it->second == RESET_COLOUR_CODE) {
				new_colour = default_colour;
			} else {
				const GFX::Font::colour_codes_t::const_iterator it = colour_codes->find(colour_it->second);
				if (it != colour_codes->end()) {
					new_colour = Utilities::to_godot_color(it->second);
				}
			}
			++colour_it;

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
	}

	if (section_start < display_text.length()) {
		separate_lines(display_text.substr(section_start), current_colour, unwrapped_lines);
	}

	return unwrapped_lines;
}

void GUITextLabel::separate_lines(
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

void GUITextLabel::separate_currency_segments(
	String const& string, Color const& colour, line_t& line
) const {
	static const String CURRENCY_MARKER = String::chr(0xA4); // ¤

	const auto push_string_segment = [this, &string, &colour, &line](int64_t start, int64_t end) -> void {
		String substring = string.substr(start, end - start);
		const real_t width = get_string_width(substring);
		line.segments.emplace_back(string_segment_t { std::move(substring), colour, width });
		line.width += width;
	};

	int64_t start_pos = 0;
	int64_t marker_pos;

	const real_t currency_width = currency_texture.is_valid() ? currency_texture->get_width() : 0.0_real;

	while ((marker_pos = string.find(CURRENCY_MARKER, start_pos)) != -1) {
		if (start_pos < marker_pos) {
			push_string_segment(start_pos, marker_pos);
		}

		line.segments.push_back(currency_segment_t {});
		line.width += currency_width;

		start_pos = marker_pos + CURRENCY_MARKER.length();
	}

	if (start_pos < string.length()) {
		push_string_segment(start_pos, string.length());
	}
}

std::vector<GUITextLabel::line_t> GUITextLabel::wrap_lines(std::vector<line_t>& unwrapped_lines) const {
	std::vector<line_t> wrapped_lines;

	const Size2 size = get_content_max_size();

	for (line_t& line : unwrapped_lines) {
		if (line.width <= size.x) {
			wrapped_lines.push_back(std::move(line));
		} else {
			line_t* current_line = &wrapped_lines.emplace_back();

			for (segment_t& segment : line.segments) {
				const real_t segment_width = get_segment_width(segment);

				if (current_line->width + segment_width <= size.x) {
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

						if (current_line->width + whole_segment_width > size.x) {
							String new_segment_string;
							real_t new_segment_width = 0.0_real;

							int64_t last_marker_pos = 0;
							int64_t marker_pos;

							while ((marker_pos = whole_segment_string.find(SPACE_MARKER, last_marker_pos)) != -1) {
								String substring = whole_segment_string.substr(0, marker_pos);
								const real_t width = get_string_width(substring);
								if (current_line->width + width <= size.x) {
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

	if (wrapped_lines.size() > 1 && wrapped_lines.size() * font->get_height(font->get_fixed_size()) > size.y) {
		do {
			wrapped_lines.pop_back();
		} while (wrapped_lines.size() > 1 && wrapped_lines.size() * font->get_height(font->get_fixed_size()) > size.y);

		static const String ELLIPSIS = "...";
		const real_t ellipsis_width = get_string_width(ELLIPSIS);

		line_t& last_line = wrapped_lines.back();
		Color last_colour = default_colour;

		while (last_line.segments.size() > 0 && last_line.width + ellipsis_width > size.x) {
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
