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

#include "openvic-extension/classes/GFXIconTexture.hpp"
#include "openvic-extension/classes/GFXMaskedFlagTexture.hpp"
#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUIOverlappingElementsBox.hpp"
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

GUI::Element const* UITools::get_gui_element(godot::String const& gui_file, godot::String const& gui_element) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GUI::Scene const* scene =
		game_singleton->get_game_manager().get_ui_manager().get_scene_by_identifier(godot_to_std_string(gui_file));
	ERR_FAIL_NULL_V_MSG(scene, nullptr, vformat("Failed to find GUI file %s", gui_file));
	GUI::Element const* element = scene->get_scene_element_by_identifier(godot_to_std_string(gui_element));
	ERR_FAIL_NULL_V_MSG(element, nullptr, vformat("Failed to find GUI element %s in GUI file %s", gui_element, gui_file));
	return element;
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
static T* new_control(GUI::Element const& element, String const& name) {
	T* node = memnew(T);
	ERR_FAIL_NULL_V(node, nullptr);

	using enum GUI::Element::orientation_t;
	using enum Control::LayoutPreset;
	static const std::map<GUI::Element::orientation_t, Control::LayoutPreset> orientation_map {
		{ UPPER_LEFT, PRESET_TOP_LEFT }, { LOWER_LEFT, PRESET_BOTTOM_LEFT },
		{ LOWER_RIGHT, PRESET_BOTTOM_RIGHT }, { UPPER_RIGHT, PRESET_TOP_RIGHT },
		{ CENTER, PRESET_CENTER }
	};

	if (name.is_empty()) {
		node->set_name(std_view_to_godot_string(element.get_name()));
	} else {
		node->set_name(name);
	}

	const decltype(orientation_map)::const_iterator it = orientation_map.find(element.get_orientation());
	if (it != orientation_map.end()) {
		node->set_anchors_and_offsets_preset(it->second);
	} else {
		UtilityFunctions::push_error("Invalid orientation for GUI element ",
			std_view_to_godot_string(element.get_name()));
	}
	node->set_position(Utilities::to_godot_fvec2(element.get_position()));
	node->set_h_size_flags(Control::SizeFlags::SIZE_SHRINK_BEGIN);
	node->set_v_size_flags(Control::SizeFlags::SIZE_SHRINK_BEGIN);
	node->set_focus_mode(Control::FOCUS_NONE);

	return node;
}

static bool generate_icon(generate_gui_args_t&& args) {
	GUI::Icon const& icon = static_cast<GUI::Icon const&>(args.element);

	const String icon_name = std_view_to_godot_string(icon.get_name());

	/* Change to use sprite type to choose Godot node type! */
	bool ret = true;
	if (icon.get_sprite() != nullptr) {
		if (icon.get_sprite()->is_type<GFX::TextureSprite>()) {
			TextureRect* godot_texture_rect = new_control<TextureRect>(icon, args.name);
			ERR_FAIL_NULL_V_MSG(godot_texture_rect, false, vformat("Failed to create TextureRect for GUI icon %s", icon_name));

			GFX::TextureSprite const* texture_sprite = icon.get_sprite()->cast_to<GFX::TextureSprite>();
			Ref<GFXIconTexture> texture = GFXIconTexture::make_gfx_icon_texture(texture_sprite, icon.get_frame());
			if (texture.is_valid()) {
				godot_texture_rect->set_texture(texture);
			} else {
				UtilityFunctions::push_error("Failed to make GFXIconTexture for GUI icon ", icon_name);
				ret = false;
			}

			args.result = godot_texture_rect;
		} else if (icon.get_sprite()->is_type<GFX::MaskedFlag>()) {
			TextureRect* godot_texture_rect = new_control<TextureRect>(icon, args.name);
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
			TextureProgressBar* godot_progress_bar = new_control<TextureProgressBar>(icon, args.name);
			ERR_FAIL_NULL_V_MSG(
				godot_progress_bar, false, vformat("Failed to create TextureProgressBar for GUI icon %s", icon_name)
			);

			const StringName back_texture_file =
				std_view_to_godot_string_name(icon.get_sprite()->cast_to<GFX::ProgressBar>()->get_back_texture_file());
			const Ref<ImageTexture> back_texture = args.asset_manager.get_texture(back_texture_file);
			if (back_texture.is_valid()) {
				godot_progress_bar->set_under_texture(back_texture);
			} else {
				UtilityFunctions::push_error("Failed to load progress bar base sprite ", back_texture_file, " for GUI icon ", icon_name);
				ret = false;
			}

			const StringName progress_texture_file =
				std_view_to_godot_string_name(icon.get_sprite()->cast_to<GFX::ProgressBar>()->get_progress_texture_file());
			const Ref<ImageTexture> progress_texture = args.asset_manager.get_texture(progress_texture_file);
			if (progress_texture.is_valid()) {
				godot_progress_bar->set_progress_texture(progress_texture);
			} else {
				UtilityFunctions::push_error(
					"Failed to load progress bar base sprite ", progress_texture_file, " for GUI icon ", icon_name
				);
				ret = false;
			}

			args.result = godot_progress_bar;
		} else if (icon.get_sprite()->is_type<GFX::PieChart>()) {
			TextureRect* godot_texture_rect = new_control<TextureRect>(icon, args.name);
			ERR_FAIL_NULL_V_MSG(godot_texture_rect, false, vformat("Failed to create TextureRect for GUI icon %s", icon_name));

			GFX::PieChart const* pie_chart = icon.get_sprite()->cast_to<GFX::PieChart>();
			Ref<GFXPieChartTexture> texture = GFXPieChartTexture::make_gfx_pie_chart_texture(pie_chart);
			if (texture.is_valid()) {
				godot_texture_rect->set_texture(texture);
				// TODO - work out why this is needed
				Vector2 pos = godot_texture_rect->get_position();
				pos.x -= texture->get_width() / 2;
				godot_texture_rect->set_position(pos);
			} else {
				UtilityFunctions::push_error("Failed to make GFXPieChartTexture for GUI icon ", icon_name);
				ret = false;
			}

			args.result = godot_texture_rect;
		} else if (icon.get_sprite()->is_type<GFX::LineChart>()) {

		} else {
			UtilityFunctions::push_error("Invalid sprite type ", std_view_to_godot_string(icon.get_sprite()->get_type()),
				" for GUI icon ", icon_name);
			ret = false;
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

	Button* godot_button = new_control<Button>(button, args.name);
	ERR_FAIL_NULL_V_MSG(godot_button, false, vformat("Failed to create Button for GUI button %s", button_name));

	if (!button.get_text().empty()) {
		godot_button->set_text(std_view_to_godot_string(button.get_text()));
	}

	bool ret = true;
	if (button.get_sprite() != nullptr) {
		Ref<Texture2D> texture;
		if (button.get_sprite()->is_type<GFX::TextureSprite>()) {
			GFX::TextureSprite const* texture_sprite = button.get_sprite()->cast_to<GFX::TextureSprite>();
			texture = GFXIconTexture::make_gfx_icon_texture(texture_sprite);
			if (texture.is_null()) {
				UtilityFunctions::push_error("Failed to make GFXIconTexture for GUI button ", button_name);
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
			Ref<StyleBoxTexture> stylebox;
			stylebox.instantiate();
			if (stylebox.is_valid()) {
				static const StringName theme_name_normal = "normal";
				stylebox->set_texture(texture);
				godot_button->add_theme_stylebox_override(theme_name_normal, stylebox);
			} else {
				UtilityFunctions::push_error("Failed to load instantiate texture stylebox for GUI button ", button_name);
				ret = false;
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
		const Color colour = Utilities::to_godot_color(button.get_font()->get_colour());
		godot_button->add_theme_color_override("font_color", colour);
	}

	args.result = godot_button;
	return ret;
}

static bool generate_checkbox(generate_gui_args_t&& args) {
	GUI::Checkbox const& checkbox = static_cast<GUI::Checkbox const&>(args.element);

	// TODO - shortcut, sprite, text
	const String checkbox_name = std_view_to_godot_string(checkbox.get_name());

	CheckBox* godot_checkbox = new_control<CheckBox>(checkbox, args.name);
	ERR_FAIL_NULL_V_MSG(godot_checkbox, false, vformat("Failed to create CheckBox for GUI checkbox %s", checkbox_name));

	bool ret = true;
	if (checkbox.get_sprite() != nullptr) {
		GFX::TextureSprite const* texture_sprite = checkbox.get_sprite()->cast_to<GFX::TextureSprite>();
		if (texture_sprite != nullptr) {
			Ref<GFXIconTexture> icon_texture = GFXIconTexture::make_gfx_icon_texture(texture_sprite, 1);
			if (icon_texture.is_valid()) {
				godot_checkbox->set_custom_minimum_size(icon_texture->get_size());
				godot_checkbox->add_theme_icon_override("unchecked", icon_texture);
			} else {
				UtilityFunctions::push_error("Failed to make unchecked GFXIconTexture for GUI checkbox ", checkbox_name);
				ret = false;
			}
			icon_texture = GFXIconTexture::make_gfx_icon_texture(texture_sprite, 2);
			if (icon_texture.is_valid()) {
				godot_checkbox->add_theme_icon_override("checked", icon_texture);
			} else {
				UtilityFunctions::push_error("Failed to make checked GFXIconTexture for GUI checkbox ", checkbox_name);
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

	Label* godot_label = new_control<Label>(text, args.name);
	ERR_FAIL_NULL_V_MSG(godot_label, false, vformat("Failed to create Label for GUI text %s", text_name));

	godot_label->set_text(std_view_to_godot_string(text.get_text()));
	godot_label->set_custom_minimum_size(Utilities::to_godot_fvec2(text.get_max_size()));

	using enum GUI::AlignedElement::format_t;
	static const std::map<GUI::AlignedElement::format_t, HorizontalAlignment> format_map {
		{ left, HORIZONTAL_ALIGNMENT_LEFT },
		{ centre, HORIZONTAL_ALIGNMENT_CENTER },
		{ right, HORIZONTAL_ALIGNMENT_RIGHT }
	};

	const decltype(format_map)::const_iterator it = format_map.find(text.get_format());
	if (it != format_map.end()) {
		godot_label->set_horizontal_alignment(it->second);
	} else {
		UtilityFunctions::push_error("Invalid text format (horizontal alignment) for GUI text ", text_name);
	}

	bool ret = true;
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

	GUIOverlappingElementsBox* box = new_control<GUIOverlappingElementsBox>(overlapping_elements, args.name);
	ERR_FAIL_NULL_V_MSG(
		box, false,
		vformat("Failed to create GUIOverlappingElementsBox for GUI overlapping elements %s", overlapping_elements_name)
	);
	const bool ret = box->set_gui_overlapping_elements_box(&overlapping_elements) == OK;
	args.result = box;
	return ret;
}

static bool generate_listbox(generate_gui_args_t&& args) {
	GUI::ListBox const& listbox = static_cast<GUI::ListBox const&>(args.element);

	const String listbox_name = std_view_to_godot_string(listbox.get_name());

	ColorRect* godot_rect = new_control<ColorRect>(listbox, args.name);
	ERR_FAIL_NULL_V_MSG(godot_rect, false, vformat("Failed to create ColorRect for GUI listbox %s", listbox_name));

	godot_rect->set_custom_minimum_size(Utilities::to_godot_fvec2(listbox.get_size()));
	godot_rect->set_color({ 1.0f, 0.5f, 0.0f, 0.2f });

	args.result = godot_rect;
	return true;
}

/* Forward declaration for use in generate_window. */
static bool generate_element(GUI::Element const* element, String const& name, AssetManager& asset_manager, Control*& result);

static bool generate_window(generate_gui_args_t&& args) {
	GUI::Window const& window = static_cast<GUI::Window const&>(args.element);

	// TODO - moveable, fullscreen, dontRender (disable visibility?)
	const String window_name = std_view_to_godot_string(window.get_name());

	Panel* godot_panel = new_control<Panel>(window, args.name);
	ERR_FAIL_NULL_V_MSG(godot_panel, false, vformat("Failed to create Panel for GUI window %s", window_name));

	godot_panel->set_custom_minimum_size(Utilities::to_godot_fvec2(window.get_size()));
	godot_panel->set_self_modulate({ 1.0f, 1.0f, 1.0f, 0.0f });

	bool ret = true;
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
	static const std::map<std::string_view, bool (*)(generate_gui_args_t&&)> type_map {
		{ GUI::Icon::get_type_static(), &generate_icon },
		{ GUI::Button::get_type_static(), &generate_button },
		{ GUI::Checkbox::get_type_static(), &generate_checkbox },
		{ GUI::Text::get_type_static(), &generate_text },
		{ GUI::OverlappingElementsBox::get_type_static(), &generate_overlapping_elements },
		{ GUI::ListBox::get_type_static(), &generate_listbox },
		{ GUI::Window::get_type_static(), &generate_window }
	};
	const decltype(type_map)::const_iterator it = type_map.find(element->get_type());
	if (it != type_map.end()) {
		return it->second({ *element, name, asset_manager, result });
	} else {
		UtilityFunctions::push_error("Invalid GUI element type: ", std_view_to_godot_string(element->get_type()));
		return false;
	}
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
	godot::String const& gui_file, godot::String const& gui_element, godot::String const& name, godot::Control*& result
) {
	return generate_gui_element(get_gui_element(gui_file, gui_element), name, result);
}
