#include "UITools.hpp"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/classes/texture_progress_bar.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/classes/GFXButtonStateTexture.hpp"
#include "openvic-extension/classes/GFXSpriteTexture.hpp"
#include "openvic-extension/classes/GFXMaskedFlagTexture.hpp"
#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUIOverlappingElementsBox.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_view_to_godot_string;
using OpenVic::Utilities::std_view_to_godot_string_name;

GFX::Sprite const* UITools::get_gfx_sprite(godot::String const& gfx_sprite) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GFX::Sprite const* sprite = game_singleton->get_game_manager().get_ui_manager().get_sprite_by_identifier(
		godot_to_std_string(gfx_sprite)
	);
	ERR_FAIL_NULL_V_MSG(sprite, nullptr, vformat("GFX sprite not found: %s", gfx_sprite));
	return sprite;
}

GUI::Element const* UITools::get_gui_element(godot::String const& gui_scene, godot::String const& gui_element) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GUI::Scene const* scene =
		game_singleton->get_game_manager().get_ui_manager().get_scene_by_identifier(godot_to_std_string(gui_scene));
	ERR_FAIL_NULL_V_MSG(scene, nullptr, vformat("Failed to find GUI scene %s", gui_scene));
	GUI::Element const* element = scene->get_scene_element_by_identifier(godot_to_std_string(gui_element));
	ERR_FAIL_NULL_V_MSG(element, nullptr, vformat("Failed to find GUI element %s in GUI scene %s", gui_element, gui_scene));
	return element;
}

GUI::Position const* UITools::get_gui_position(String const& gui_scene, String const& gui_position) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GUI::Scene const* scene =
		game_singleton->get_game_manager().get_ui_manager().get_scene_by_identifier(godot_to_std_string(gui_scene));
	ERR_FAIL_NULL_V_MSG(scene, nullptr, vformat("Failed to find GUI scene %s", gui_scene));
	GUI::Position const* position = scene->get_scene_position_by_identifier(godot_to_std_string(gui_position));
	ERR_FAIL_NULL_V_MSG(position, nullptr, vformat("Failed to find GUI position %s in GUI scene %s", gui_position, gui_scene));
	return position;
}

/* GUI::Element tree -> godot::Control tree conversion code below: */

namespace OpenVic {
	struct generate_gui_args_t {
		GUI::Element const& element;
		godot::String const& name;
		AssetManager& asset_manager;
		godot::Control*& result;

		constexpr generate_gui_args_t(
			GUI::Element const& new_element, godot::String const& new_name, AssetManager& new_asset_manager,
			godot::Control*& new_result
		) : element { new_element }, name { new_name }, asset_manager { new_asset_manager }, result { new_result } {}
	};
}

template<std::derived_from<Control> T>
static bool new_control(T*& node, GUI::Element const& element, String const& name) {
	node = memnew(T);
	ERR_FAIL_NULL_V(node, false);

	using enum GUI::Element::orientation_t;
	using enum Control::LayoutPreset;
	static const ordered_map<GUI::Element::orientation_t, Control::LayoutPreset> orientation_map {
		{ UPPER_LEFT, PRESET_TOP_LEFT }, { LOWER_LEFT, PRESET_BOTTOM_LEFT },
		{ LOWER_RIGHT, PRESET_BOTTOM_RIGHT }, { UPPER_RIGHT, PRESET_TOP_RIGHT },
		{ CENTER, PRESET_CENTER }
	};

	if (name.is_empty()) {
		node->set_name(std_view_to_godot_string(element.get_name()));
	} else {
		node->set_name(name);
	}

	bool ret = true;
	const decltype(orientation_map)::const_iterator it = orientation_map.find(element.get_orientation());
	if (it != orientation_map.end()) {
		node->set_anchors_and_offsets_preset(it->second);
	} else {
		UtilityFunctions::push_error("Invalid orientation for GUI element ", std_view_to_godot_string(element.get_name()));
		ret = false;
	}

	node->set_position(Utilities::to_godot_fvec2(element.get_position()));
	node->set_h_size_flags(Control::SizeFlags::SIZE_SHRINK_BEGIN);
	node->set_v_size_flags(Control::SizeFlags::SIZE_SHRINK_BEGIN);
	node->set_focus_mode(Control::FOCUS_NONE);

	return ret;
}

static bool add_theme_stylebox(Control* control, StringName const& theme_name, Ref<Texture2D> const& texture) {
	Ref<StyleBoxTexture> stylebox;
	stylebox.instantiate();
	ERR_FAIL_NULL_V(stylebox, false);
	stylebox->set_texture(texture);
	control->add_theme_stylebox_override(theme_name, stylebox);
	return true;
};

static bool generate_icon(generate_gui_args_t&& args) {
	GUI::Icon const& icon = static_cast<GUI::Icon const&>(args.element);

	const String icon_name = std_view_to_godot_string(icon.get_name());

	/* Change to use sprite type to choose Godot node type! */
	bool ret = true;
	if (icon.get_sprite() != nullptr) {
		if (icon.get_sprite()->is_type<GFX::IconTextureSprite>()) {
			TextureRect* godot_texture_rect = nullptr;
			ret &= new_control(godot_texture_rect, icon, args.name);
			ERR_FAIL_NULL_V_MSG(godot_texture_rect, false, vformat("Failed to create TextureRect for GUI icon %s", icon_name));

			godot_texture_rect->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

			GFX::IconTextureSprite const* texture_sprite = icon.get_sprite()->cast_to<GFX::IconTextureSprite>();
			Ref<GFXSpriteTexture> texture = GFXSpriteTexture::make_gfx_sprite_texture(texture_sprite, icon.get_frame());
			if (texture.is_valid()) {
				godot_texture_rect->set_texture(texture);
			} else {
				UtilityFunctions::push_error("Failed to make GFXSpriteTexture for GUI icon ", icon_name);
				ret = false;
			}

			args.result = godot_texture_rect;
		} else if (icon.get_sprite()->is_type<GFX::MaskedFlag>()) {
			TextureRect* godot_texture_rect = nullptr;
			ret &= new_control(godot_texture_rect, icon, args.name);
			ERR_FAIL_NULL_V_MSG(godot_texture_rect, false, vformat("Failed to create TextureRect for GUI icon %s", icon_name));

			GFX::MaskedFlag const* masked_flag = icon.get_sprite()->cast_to<GFX::MaskedFlag>();
			Ref<GFXMaskedFlagTexture> texture = GFXMaskedFlagTexture::make_gfx_masked_flag_texture(masked_flag);
			if (texture.is_valid()) {
				godot_texture_rect->set_texture(texture);
			} else {
				UtilityFunctions::push_error("Failed to make GFXMaskedFlagTexture for GUI icon ", icon_name);
				ret = false;
			}

			args.result = godot_texture_rect;
		} else if (icon.get_sprite()->is_type<GFX::ProgressBar>()) {
			TextureProgressBar* godot_progress_bar = nullptr;
			ret &= new_control(godot_progress_bar, icon, args.name);
			ERR_FAIL_NULL_V_MSG(
				godot_progress_bar, false, vformat("Failed to create TextureProgressBar for GUI icon %s", icon_name)
			);

			GFX::ProgressBar const* progress_bar = icon.get_sprite()->cast_to<GFX::ProgressBar>();

			Ref<ImageTexture> back_texture;
			if (!progress_bar->get_back_texture_file().empty()) {
				const StringName back_texture_file = std_view_to_godot_string_name(progress_bar->get_back_texture_file());
				back_texture = args.asset_manager.get_texture(back_texture_file);
				if (back_texture.is_null()) {
					UtilityFunctions::push_error(
						"Failed to load progress bar sprite back texture ", back_texture_file, " for GUI icon ", icon_name
					);
					ret = false;
				}
			}
			if (back_texture.is_null()) {
				const Color back_colour = Utilities::to_godot_color(progress_bar->get_back_colour());
				back_texture = Utilities::make_solid_colour_texture(
					back_colour, progress_bar->get_size().x, progress_bar->get_size().y
				);
				if (back_texture.is_null()) {
					UtilityFunctions::push_error(
						"Failed to generate progress bar sprite ", back_colour, " back texture for GUI icon ", icon_name
					);
					ret = false;
				}
			}
			if (back_texture.is_valid()) {
				godot_progress_bar->set_under_texture(back_texture);
			} else {
				UtilityFunctions::push_error(
					"Failed to create and set progress bar sprite back texture for GUI icon ", icon_name
				);
				ret = false;
			}

			Ref<ImageTexture> progress_texture;
			if (!progress_bar->get_progress_texture_file().empty()) {
				const StringName progress_texture_file = std_view_to_godot_string_name(progress_bar->get_progress_texture_file());
				progress_texture = args.asset_manager.get_texture(progress_texture_file);
				if (progress_texture.is_null()) {
					UtilityFunctions::push_error(
						"Failed to load progress bar sprite progress texture ", progress_texture_file, " for GUI icon ", icon_name
					);
					ret = false;
				}
			}
			if (progress_texture.is_null()) {
				const Color progress_colour = Utilities::to_godot_color(progress_bar->get_progress_colour());
				progress_texture = Utilities::make_solid_colour_texture(
					progress_colour, progress_bar->get_size().x, progress_bar->get_size().y
				);
				if (progress_texture.is_null()) {
					UtilityFunctions::push_error(
						"Failed to generate progress bar sprite ", progress_colour, " progress texture for GUI icon ", icon_name
					);
					ret = false;
				}
			}
			if (progress_texture.is_valid()) {
				godot_progress_bar->set_progress_texture(progress_texture);
			} else {
				UtilityFunctions::push_error(
					"Failed to create and set progress bar sprite progress texture for GUI icon ", icon_name
				);
				ret = false;
			}

			// TODO - work out why progress bar is missing bottom border pixel (e.g. province building expansion bar)
			godot_progress_bar->set_custom_minimum_size(Utilities::to_godot_fvec2(static_cast<fvec2_t>(progress_bar->get_size())));

			args.result = godot_progress_bar;
		} else if (icon.get_sprite()->is_type<GFX::PieChart>()) {
			TextureRect* godot_texture_rect = nullptr;
			ret &= new_control(godot_texture_rect, icon, args.name);
			ERR_FAIL_NULL_V_MSG(godot_texture_rect, false, vformat("Failed to create TextureRect for GUI icon %s", icon_name));

			GFX::PieChart const* pie_chart = icon.get_sprite()->cast_to<GFX::PieChart>();
			Ref<GFXPieChartTexture> texture = GFXPieChartTexture::make_gfx_pie_chart_texture(pie_chart);
			if (texture.is_valid()) {
				godot_texture_rect->set_texture(texture);
				// TODO - work out why this is needed
				Vector2 pos = godot_texture_rect->get_position();
				pos.x -= texture->get_width() / 2.0f;
				godot_texture_rect->set_position(pos);
			} else {
				UtilityFunctions::push_error("Failed to make GFXPieChartTexture for GUI icon ", icon_name);
				ret = false;
			}

			args.result = godot_texture_rect;
		} else if (icon.get_sprite()->is_type<GFX::LineChart>()) {
			// TODO - generate line chart
		} else {
			UtilityFunctions::push_error("Invalid sprite type ", std_view_to_godot_string(icon.get_sprite()->get_type()),
				" for GUI icon ", icon_name);
			ret = false;
		}

		if (args.result != nullptr) {
			const float scale = icon.get_scale();
			args.result->set_scale({ scale, scale });
			// TODO - rotation (may have to translate as godot rotates around the top left corner)
		}
	} else {
		UtilityFunctions::push_error("Null sprite for GUI icon ", icon_name);
		ret = false;
	}
	return ret;
}

static bool generate_button(generate_gui_args_t&& args) {
	GUI::Button const& button = static_cast<GUI::Button const&>(args.element);

	// TODO - shortcut, sprite, text
	const String button_name = std_view_to_godot_string(button.get_name());

	Button* godot_button = nullptr;
	bool ret = new_control(godot_button, button, args.name);
	ERR_FAIL_NULL_V_MSG(godot_button, false, vformat("Failed to create Button for GUI button %s", button_name));

	godot_button->set_mouse_filter(Control::MOUSE_FILTER_PASS);

	if (!button.get_text().empty()) {
		godot_button->set_text(std_view_to_godot_string(button.get_text()));
	}

	if (button.get_sprite() != nullptr) {
		Ref<GFXButtonStateHavingTexture> texture;
		if (button.get_sprite()->is_type<GFX::IconTextureSprite>()) {
			GFX::IconTextureSprite const* texture_sprite = button.get_sprite()->cast_to<GFX::IconTextureSprite>();
			texture = GFXSpriteTexture::make_gfx_sprite_texture(texture_sprite);
			if (texture.is_null()) {
				UtilityFunctions::push_error("Failed to make GFXSpriteTexture for GUI button ", button_name);
				ret = false;
			}
		} else if (button.get_sprite()->is_type<GFX::MaskedFlag>()) {
			GFX::MaskedFlag const* masked_flag = button.get_sprite()->cast_to<GFX::MaskedFlag>();
			texture = GFXMaskedFlagTexture::make_gfx_masked_flag_texture(masked_flag);
			if (texture.is_null()) {
				UtilityFunctions::push_error("Failed to make GFXMaskedFlagTexture for GUI button ", button_name);
				ret = false;
			}
		} else {
			UtilityFunctions::push_error("Invalid sprite type ", std_view_to_godot_string(button.get_sprite()->get_type()),
				" for GUI button ", button_name);
			ret = false;
		}

		if (texture.is_valid()) {
			godot_button->set_custom_minimum_size(texture->get_size());

			static const StringName theme_name_normal = "normal";
			ret &= add_theme_stylebox(godot_button, theme_name_normal, texture);

			using enum GFXButtonStateTexture::ButtonState;
			for (GFXButtonStateTexture::ButtonState button_state : { HOVER, PRESSED, DISABLED }) {
				Ref<GFXButtonStateTexture> button_state_texture = texture->get_button_state_texture(button_state);
				if (button_state_texture.is_valid()) {
					ret &= add_theme_stylebox(
						godot_button, button_state_texture->get_button_state_theme(), button_state_texture
					);
				} else {
					UtilityFunctions::push_error(
						"Failed to make ", GFXButtonStateTexture::button_state_to_theme_name(button_state),
						" GFXButtonStateTexture for GUI button ", button_name
					);
					ret = false;
				}
			}
		}
	} else {
		UtilityFunctions::push_error("Null sprite for GUI button ", button_name);
		ret = false;
	}

	if (button.get_font() != nullptr) {
		const StringName font_file = std_view_to_godot_string_name(button.get_font()->get_fontname());
		const Ref<Font> font = args.asset_manager.get_font(font_file);
		if (font.is_valid()) {
			godot_button->add_theme_font_override("font", font);
		} else {
			UtilityFunctions::push_error("Failed to load font for GUI button ", button_name);
			ret = false;
		}

		static const std::vector<StringName> button_font_themes {
			"font_color", "font_hover_color", "font_hover_pressed_color", "font_pressed_color", "font_disabled_color"
		};
		const Color colour = Utilities::to_godot_color(button.get_font()->get_colour());
		for (StringName const& theme_name : button_font_themes) {
			godot_button->add_theme_color_override(theme_name, colour);
		}
	}

	args.result = godot_button;
	return ret;
}

static bool generate_checkbox(generate_gui_args_t&& args) {
	GUI::Checkbox const& checkbox = static_cast<GUI::Checkbox const&>(args.element);

	// TODO - shortcut, sprite, text
	const String checkbox_name = std_view_to_godot_string(checkbox.get_name());

	CheckBox* godot_checkbox = nullptr;
	bool ret = new_control(godot_checkbox, checkbox, args.name);
	ERR_FAIL_NULL_V_MSG(godot_checkbox, false, vformat("Failed to create CheckBox for GUI checkbox %s", checkbox_name));

	if (checkbox.get_sprite() != nullptr) {
		GFX::IconTextureSprite const* texture_sprite = checkbox.get_sprite()->cast_to<GFX::IconTextureSprite>();
		if (texture_sprite != nullptr) {
			Ref<GFXSpriteTexture> icon_texture = GFXSpriteTexture::make_gfx_sprite_texture(texture_sprite, 1);
			if (icon_texture.is_valid()) {
				godot_checkbox->set_custom_minimum_size(icon_texture->get_size());
				godot_checkbox->add_theme_icon_override("unchecked", icon_texture);
			} else {
				UtilityFunctions::push_error("Failed to make unchecked GFXSpriteTexture for GUI checkbox ", checkbox_name);
				ret = false;
			}
			icon_texture = GFXSpriteTexture::make_gfx_sprite_texture(texture_sprite, 2);
			if (icon_texture.is_valid()) {
				godot_checkbox->add_theme_icon_override("checked", icon_texture);
			} else {
				UtilityFunctions::push_error("Failed to make checked GFXSpriteTexture for GUI checkbox ", checkbox_name);
				ret = false;
			}
		} else {
			UtilityFunctions::push_error(
				"Invalid sprite type ", std_view_to_godot_string(checkbox.get_sprite()->get_type()), " for GUI checkbox ",
				checkbox_name
			);
			ret = false;
		}
	} else {
		UtilityFunctions::push_error("Null sprite for GUI checkbox ", checkbox_name);
		ret = false;
	}

	args.result = godot_checkbox;
	return ret;
}

static bool generate_text(generate_gui_args_t&& args) {
	GUI::Text const& text = static_cast<GUI::Text const&>(args.element);

	const String text_name = std_view_to_godot_string(text.get_name());

	Label* godot_label = nullptr;
	bool ret = new_control(godot_label, text, args.name);
	ERR_FAIL_NULL_V_MSG(godot_label, false, vformat("Failed to create Label for GUI text %s", text_name));

	godot_label->set_text(std_view_to_godot_string(text.get_text()));
	godot_label->set_custom_minimum_size(Utilities::to_godot_fvec2(text.get_max_size()));

	using enum GUI::AlignedElement::format_t;
	static const ordered_map<GUI::AlignedElement::format_t, HorizontalAlignment> format_map {
		{ left, HORIZONTAL_ALIGNMENT_LEFT },
		{ centre, HORIZONTAL_ALIGNMENT_CENTER },
		{ right, HORIZONTAL_ALIGNMENT_RIGHT }
	};

	const decltype(format_map)::const_iterator it = format_map.find(text.get_format());
	if (it != format_map.end()) {
		godot_label->set_horizontal_alignment(it->second);
	} else {
		UtilityFunctions::push_error("Invalid text format (horizontal alignment) for GUI text ", text_name);
		ret = false;
	}

	if (text.get_font() != nullptr) {
		const StringName font_file = std_view_to_godot_string_name(text.get_font()->get_fontname());
		const Ref<Font> font = args.asset_manager.get_font(font_file);
		if (font.is_valid()) {
			godot_label->add_theme_font_override("font", font);
		} else {
			UtilityFunctions::push_error("Failed to load font for GUI text ", text_name);
			ret = false;
		}
		const Color colour = Utilities::to_godot_color(text.get_font()->get_colour());
		godot_label->add_theme_color_override("font_color", colour);
	}

	args.result = godot_label;
	return ret;
}

static bool generate_overlapping_elements(generate_gui_args_t&& args) {
	GUI::OverlappingElementsBox const& overlapping_elements = static_cast<GUI::OverlappingElementsBox const&>(args.element);

	const String overlapping_elements_name = std_view_to_godot_string(overlapping_elements.get_name());

	GUIOverlappingElementsBox* box = nullptr;
	bool ret = new_control(box, overlapping_elements, args.name);
	ERR_FAIL_NULL_V_MSG(
		box, false,
		vformat("Failed to create GUIOverlappingElementsBox for GUI overlapping elements %s", overlapping_elements_name)
	);
	box->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
	ret &= box->set_gui_overlapping_elements_box(&overlapping_elements) == OK;
	args.result = box;
	return ret;
}

template<std::derived_from<GUI::Element> T>
requires requires(T const& element) {
	{ element.get_size() } -> std::same_as<fvec2_t>;
}
static bool generate_placeholder(generate_gui_args_t&& args, Color colour) {
	T const& cast_element = static_cast<T const&>(args.element);

	static const String type_name = std_view_to_godot_string(T::get_type_static());
	const String placeholder_name = std_view_to_godot_string(cast_element.get_name());
	const Vector2 godot_size = Utilities::to_godot_fvec2(cast_element.get_size());

	UtilityFunctions::push_warning(
		"Generating placeholder ColorRect for GUI ", type_name, " ", placeholder_name, " (size ", godot_size, ")"
	);

	ColorRect* godot_rect = nullptr;
	bool ret = new_control(godot_rect, cast_element, args.name);
	ERR_FAIL_NULL_V_MSG(
		godot_rect, false, vformat("Failed to create placeholder ColorRect for GUI %s %s", type_name, placeholder_name)
	);

	godot_rect->set_custom_minimum_size(godot_size);
	godot_rect->set_color(colour);

	args.result = godot_rect;
	return ret;
}

static bool generate_listbox(generate_gui_args_t&& args) {
	return generate_placeholder<GUI::ListBox>(std::move(args), { 0.0f, 0.0f, 1.0f, 0.3f });
}

static bool generate_texteditbox(generate_gui_args_t&& args) {
	return generate_placeholder<GUI::TextEditBox>(std::move(args), { 0.0f, 1.0f, 0.0f, 0.3f });
}

static bool generate_scrollbar(generate_gui_args_t&& args) {
	GUI::Scrollbar const& scrollbar = static_cast<GUI::Scrollbar const&>(args.element);

	const String scrollbar_name = std_view_to_godot_string(scrollbar.get_name());

	GUIScrollbar* gui_scrollbar = nullptr;
	bool ret = new_control(gui_scrollbar, scrollbar, args.name);
	ERR_FAIL_NULL_V_MSG(gui_scrollbar, false, vformat("Failed to create GUIScrollbar for GUI scrollbar %s", scrollbar_name));

	if (gui_scrollbar->set_gui_scrollbar(&scrollbar) != OK) {
		UtilityFunctions::push_error("Error initialising GUIScrollbar for GUI scrollbar ", scrollbar_name);
		ret = false;
	}

	args.result = gui_scrollbar;
	return ret;
}

/* Forward declaration for use in generate_window. */
static bool generate_element(GUI::Element const* element, String const& name, AssetManager& asset_manager, Control*& result);

static bool generate_window(generate_gui_args_t&& args) {
	GUI::Window const& window = static_cast<GUI::Window const&>(args.element);

	// TODO - moveable, fullscreen, dontRender (disable visibility?)
	const String window_name = std_view_to_godot_string(window.get_name());

	Panel* godot_panel = nullptr;
	bool ret = new_control(godot_panel, window, args.name);
	ERR_FAIL_NULL_V_MSG(godot_panel, false, vformat("Failed to create Panel for GUI window %s", window_name));

	godot_panel->set_custom_minimum_size(Utilities::to_godot_fvec2(window.get_size()));
	godot_panel->set_self_modulate({ 1.0f, 1.0f, 1.0f, 0.0f });

	for (std::unique_ptr<GUI::Element> const& element : window.get_window_elements()) {
		Control* node = nullptr;
		const bool element_ret = generate_element(element.get(), "", args.asset_manager, node);
		if (node != nullptr) {
			godot_panel->add_child(node);
		}
		if (!element_ret) {
			UtilityFunctions::push_error("Errors generating GUI element ", std_view_to_godot_string(element->get_name()));
			ret = false;
		}
	}

	args.result = godot_panel;
	return ret;
}

static bool generate_element(GUI::Element const* element, String const& name, AssetManager& asset_manager, Control*& result) {
	ERR_FAIL_NULL_V(element, false);
	static const ordered_map<std::string_view, bool (*)(generate_gui_args_t&&)> type_map {
		{ GUI::Icon::get_type_static(), &generate_icon },
		{ GUI::Button::get_type_static(), &generate_button },
		{ GUI::Checkbox::get_type_static(), &generate_checkbox },
		{ GUI::Text::get_type_static(), &generate_text },
		{ GUI::OverlappingElementsBox::get_type_static(), &generate_overlapping_elements },
		{ GUI::ListBox::get_type_static(), &generate_listbox },
		{ GUI::TextEditBox::get_type_static(), &generate_texteditbox },
		{ GUI::Scrollbar::get_type_static(), &generate_scrollbar },
		{ GUI::Window::get_type_static(), &generate_window }
	};
	const decltype(type_map)::const_iterator it = type_map.find(element->get_type());
	ERR_FAIL_COND_V_MSG(
		it == type_map.end(), false, vformat("Invalid GUI element type: %s", std_view_to_godot_string(element->get_type()))
	);
	return it->second({ *element, name, asset_manager, result });
}

bool UITools::generate_gui_element(
	GUI::Element const* element, String const& name, Control*& result
) {
	result = nullptr;
	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, false);
	return generate_element(element, name, *asset_manager, result);
}

bool UITools::generate_gui_element(
	godot::String const& gui_scene, godot::String const& gui_element, godot::String const& name, godot::Control*& result
) {
	return generate_gui_element(get_gui_element(gui_scene, gui_element), name, result);
}
