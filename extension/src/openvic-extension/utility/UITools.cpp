#include "UITools.hpp"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/style_box_empty.hpp>
#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/classes/texture_progress_bar.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/classes/GFXButtonStateTexture.hpp"
#include "openvic-extension/classes/GFXSpriteTexture.hpp"
#include "openvic-extension/classes/GFXMaskedFlagTexture.hpp"
#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUIListBox.hpp"
#include "openvic-extension/classes/GUIOverlappingElementsBox.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/classes/GUITextLabel.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

GFX::Sprite const* UITools::get_gfx_sprite(String const& gfx_sprite) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GFX::Sprite const* sprite = game_singleton->get_definition_manager().get_ui_manager().get_sprite_by_identifier(
		Utilities::godot_to_std_string(gfx_sprite)
	);
	ERR_FAIL_NULL_V_MSG(sprite, nullptr, vformat("GFX sprite not found: %s", gfx_sprite));
	return sprite;
}

GUI::Element const* UITools::get_gui_element(String const& gui_scene, String const& gui_element) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GUI::Scene const* scene = game_singleton->get_definition_manager().get_ui_manager().get_scene_by_identifier(
		Utilities::godot_to_std_string(gui_scene)
	);
	ERR_FAIL_NULL_V_MSG(scene, nullptr, vformat("Failed to find GUI scene %s", gui_scene));
	GUI::Element const* element = scene->get_scene_element_by_identifier(Utilities::godot_to_std_string(gui_element));
	ERR_FAIL_NULL_V_MSG(element, nullptr, vformat("Failed to find GUI element %s in GUI scene %s", gui_element, gui_scene));
	return element;
}

GUI::Position const* UITools::get_gui_position(String const& gui_scene, String const& gui_position) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GUI::Scene const* scene = game_singleton->get_definition_manager().get_ui_manager().get_scene_by_identifier(
		Utilities::godot_to_std_string(gui_scene)
	);
	ERR_FAIL_NULL_V_MSG(scene, nullptr, vformat("Failed to find GUI scene %s", gui_scene));
	GUI::Position const* position = scene->get_scene_position_by_identifier(Utilities::godot_to_std_string(gui_position));
	ERR_FAIL_NULL_V_MSG(position, nullptr, vformat("Failed to find GUI position %s in GUI scene %s", gui_position, gui_scene));
	return position;
}

/* GUI::Element tree -> godot::Control tree conversion code below: */

namespace OpenVic {
	struct generate_gui_args_t {
		GUI::Element const& element;
		String const& name;
		AssetManager& asset_manager;
		Control*& result;

		constexpr generate_gui_args_t(
			GUI::Element const& new_element, String const& new_name, AssetManager& new_asset_manager, Control*& new_result
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
		node->set_name(Utilities::std_to_godot_string(element.get_name()));
	} else {
		node->set_name(name);
	}

	bool ret = true;
	const decltype(orientation_map)::const_iterator it = orientation_map.find(element.get_orientation());
	if (it != orientation_map.end()) {
		node->set_anchors_and_offsets_preset(it->second);
	} else {
		UtilityFunctions::push_error(
			"Invalid orientation for GUI element ", Utilities::std_to_godot_string(element.get_name())
		);
		ret = false;
	}

	node->set_position(Utilities::to_godot_fvec2(element.get_position()));
	node->set_h_size_flags(Control::SizeFlags::SIZE_SHRINK_BEGIN);
	node->set_v_size_flags(Control::SizeFlags::SIZE_SHRINK_BEGIN);
	node->set_focus_mode(Control::FOCUS_NONE);

	return ret;
}

static bool add_theme_stylebox(
	Control* control, StringName const& theme_name, Ref<Texture2D> const& texture, Vector2 border = {}
) {
	Ref<StyleBoxTexture> stylebox;
	stylebox.instantiate();
	ERR_FAIL_NULL_V(stylebox, false);
	stylebox->set_texture(texture);

	static const StringName changed_signal = "changed";
	static const StringName emit_changed_func = "emit_changed";
	texture->connect(changed_signal, Callable { *stylebox, emit_changed_func }, Object::CONNECT_PERSIST);

	if (border != Vector2 {}) {
		stylebox->set_texture_margin(SIDE_LEFT, border.x);
		stylebox->set_texture_margin(SIDE_RIGHT, border.x);
		stylebox->set_texture_margin(SIDE_TOP, border.y);
		stylebox->set_texture_margin(SIDE_BOTTOM, border.y);
	}

	control->add_theme_stylebox_override(theme_name, stylebox);
	return true;
};

static bool generate_icon(generate_gui_args_t&& args) {
	using namespace OpenVic::Utilities::literals;

	GUI::Icon const& icon = static_cast<GUI::Icon const&>(args.element);

	const String icon_name = Utilities::std_to_godot_string(icon.get_name());

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

			const float scale = icon.get_scale();
			godot_texture_rect->set_scale({ scale, scale });

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

			static constexpr double MIN_VALUE = 0.0, MAX_VALUE = 1.0;
			static constexpr uint32_t STEPS = 100;

			godot_progress_bar->set_nine_patch_stretch(true);
			godot_progress_bar->set_step((MAX_VALUE - MIN_VALUE) / STEPS);
			godot_progress_bar->set_min(MIN_VALUE);
			godot_progress_bar->set_max(MAX_VALUE);

			GFX::ProgressBar const* progress_bar = icon.get_sprite()->cast_to<GFX::ProgressBar>();

			using enum AssetManager::LoadFlags;

			Ref<ImageTexture> back_texture;
			if (!progress_bar->get_back_texture_file().empty()) {
				const StringName back_texture_file = Utilities::std_to_godot_string(progress_bar->get_back_texture_file());
				back_texture = args.asset_manager.get_texture(back_texture_file, LOAD_FLAG_CACHE_TEXTURE | LOAD_FLAG_FLIP_Y);
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
				const StringName progress_texture_file =
					Utilities::std_to_godot_string(progress_bar->get_progress_texture_file());
				progress_texture =
					args.asset_manager.get_texture(progress_texture_file, LOAD_FLAG_CACHE_TEXTURE | LOAD_FLAG_FLIP_Y);
				if (progress_texture.is_null()) {
					UtilityFunctions::push_error(
						"Failed to load progress bar sprite progress texture ", progress_texture_file, " for GUI icon ",
						icon_name
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
						"Failed to generate progress bar sprite ", progress_colour, " progress texture for GUI icon ",
						icon_name
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
			godot_progress_bar->set_custom_minimum_size(
				Utilities::to_godot_fvec2(static_cast<fvec2_t>(progress_bar->get_size()))
			);

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
				pos.x -= texture->get_width() / 2.0_real;
				godot_texture_rect->set_position(pos);
			} else {
				UtilityFunctions::push_error("Failed to make GFXPieChartTexture for GUI icon ", icon_name);
				ret = false;
			}

			args.result = godot_texture_rect;
		} else if (icon.get_sprite()->is_type<GFX::LineChart>()) {
			// TODO - generate line chart
		} else {
			UtilityFunctions::push_error(
				"Invalid sprite type ", Utilities::std_to_godot_string(icon.get_sprite()->get_type()),
				" for GUI icon ", icon_name
			);
			ret = false;
		}

		if (args.result != nullptr) {
			const real_t rotation = icon.get_rotation();
			if (rotation != 0.0_real) {
				args.result->set_position(
					args.result->get_position() - args.result->get_custom_minimum_size().height * Vector2 {
						Math::sin(rotation), Math::cos(rotation) - 1.0_real
					}
				);
				args.result->set_rotation(-rotation);
			}
		}
	} else {
		UtilityFunctions::push_error("Null sprite for GUI icon ", icon_name);
		ret = false;
	}
	return ret;
}

static bool generate_button(generate_gui_args_t&& args) {
	GUI::Button const& button = static_cast<GUI::Button const&>(args.element);

	// TODO - shortcut, clicksound, rotation (?)
	const String button_name = Utilities::std_to_godot_string(button.get_name());

	Button* godot_button = nullptr;
	bool ret = new_control(godot_button, button, args.name);
	ERR_FAIL_NULL_V_MSG(godot_button, false, vformat("Failed to create Button for GUI button %s", button_name));

	godot_button->set_mouse_filter(Control::MOUSE_FILTER_PASS);

	godot_button->set_text(Utilities::std_to_godot_string(button.get_text()));

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
			UtilityFunctions::push_error(
				"Invalid sprite type ", Utilities::std_to_godot_string(button.get_sprite()->get_type()),
				" for GUI button ", button_name
			);
			ret = false;
		}

		if (texture.is_valid()) {
			godot_button->set_custom_minimum_size(texture->get_size());

			static const StringName normal_theme = "normal";
			ret &= add_theme_stylebox(godot_button, normal_theme, texture);

			using enum GFXButtonStateTexture::ButtonState;
			for (GFXButtonStateTexture::ButtonState button_state : { HOVER, PRESSED, DISABLED }) {
				Ref<GFXButtonStateTexture> button_state_texture = texture->get_button_state_texture(button_state);
				if (button_state_texture.is_valid()) {
					ret &= add_theme_stylebox(
						godot_button, button_state_texture->get_button_state_name(), button_state_texture
					);
				} else {
					UtilityFunctions::push_error(
						"Failed to make ", GFXButtonStateTexture::button_state_to_name(button_state),
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
		const StringName font_file = Utilities::std_to_godot_string(button.get_font()->get_fontname());
		const Ref<Font> font = args.asset_manager.get_font(font_file);
		if (font.is_valid()) {
			static const StringName font_theme = "font";
			godot_button->add_theme_font_override(font_theme, font);
		} else {
			UtilityFunctions::push_error("Failed to load font \"", font_file, "\" for GUI button ", button_name);
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

	// TODO - shortcut
	const String checkbox_name = Utilities::std_to_godot_string(checkbox.get_name());

	Button* godot_button = nullptr;
	bool ret = new_control(godot_button, checkbox, args.name);
	ERR_FAIL_NULL_V_MSG(godot_button, false, vformat("Failed to create Button for GUI checkbutton %s", checkbox_name));

	godot_button->set_text(Utilities::std_to_godot_string(checkbox.get_text()));

	godot_button->set_toggle_mode(true);

	if (checkbox.get_sprite() != nullptr) {
		GFX::IconTextureSprite const* texture_sprite = checkbox.get_sprite()->cast_to<GFX::IconTextureSprite>();

		if (texture_sprite != nullptr) {
			Ref<GFXSpriteTexture> texture = GFXSpriteTexture::make_gfx_sprite_texture(texture_sprite);

			if (texture.is_valid()) {
				godot_button->set_custom_minimum_size(texture->get_size());

				if (texture->get_icon_count() > 1) {
					static const StringName toggled_signal = "toggled";
					static const StringName set_toggled_icon_func = "set_toggled_icon";
					godot_button->connect(
						toggled_signal, Callable { *texture, set_toggled_icon_func }, Object::CONNECT_PERSIST
					);
				}

				static const StringName normal_theme = "normal";
				ret &= add_theme_stylebox(godot_button, normal_theme, texture);

				using enum GFXButtonStateTexture::ButtonState;
				for (GFXButtonStateTexture::ButtonState button_state : { HOVER, PRESSED, DISABLED }) {
					Ref<GFXButtonStateTexture> button_state_texture = texture->get_button_state_texture(button_state);
					if (button_state_texture.is_valid()) {
						ret &= add_theme_stylebox(
							godot_button, button_state_texture->get_button_state_name(), button_state_texture
						);
					} else {
						UtilityFunctions::push_error(
							"Failed to make ", GFXButtonStateTexture::button_state_to_name(button_state),
							" GFXButtonStateTexture for GUI checkbox ", checkbox_name
						);
						ret = false;
					}
				}
			} else {
				UtilityFunctions::push_error("Failed to make GFXSpriteTexture for GUI checkbox ", checkbox_name);
				ret = false;
			}
		} else {
			UtilityFunctions::push_error(
				"Invalid sprite type ", Utilities::std_to_godot_string(checkbox.get_sprite()->get_type()),
				" for GUI checkbox ", checkbox_name
			);
			ret = false;
		}
	} else {
		UtilityFunctions::push_error("Null sprite for GUI checkbox ", checkbox_name);
		ret = false;
	}

	if (checkbox.get_font() != nullptr) {
		const StringName font_file = Utilities::std_to_godot_string(checkbox.get_font()->get_fontname());
		const Ref<Font> font = args.asset_manager.get_font(font_file);
		if (font.is_valid()) {
			static const StringName font_theme = "font";
			godot_button->add_theme_font_override(font_theme, font);
		} else {
			UtilityFunctions::push_error("Failed to load font \"", font_file, "\" for GUI checkbox ", checkbox_name);
			ret = false;
		}

		static const std::vector<StringName> checkbox_font_themes {
			"font_color", "font_hover_color", "font_hover_pressed_color", "font_pressed_color", "font_disabled_color"
		};
		const Color colour = Utilities::to_godot_color(checkbox.get_font()->get_colour());
		for (StringName const& theme_name : checkbox_font_themes) {
			godot_button->add_theme_color_override(theme_name, colour);
		}
	}

	args.result = godot_button;
	return ret;
}

static bool generate_text(generate_gui_args_t&& args) {
	GUI::Text const& text = static_cast<GUI::Text const&>(args.element);

	const String text_name = Utilities::std_to_godot_string(text.get_name());

	GUITextLabel* text_label = nullptr;
	bool ret = new_control(text_label, text, args.name);
	ERR_FAIL_NULL_V_MSG(text_label, false, vformat("Failed to create GUITextLabel for GUI text %s", text_name));

	text_label->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

	if (text_label->set_gui_text(&text) != OK) {
		UtilityFunctions::push_error("Error initialising GUITextLabel for GUI text ", text_name);
		ret = false;
	}

	args.result = text_label;
	return ret;
}

static bool generate_overlapping_elements(generate_gui_args_t&& args) {
	GUI::OverlappingElementsBox const& overlapping_elements = static_cast<GUI::OverlappingElementsBox const&>(args.element);

	const String overlapping_elements_name = Utilities::std_to_godot_string(overlapping_elements.get_name());

	GUIOverlappingElementsBox* box = nullptr;
	bool ret = new_control(box, overlapping_elements, args.name);
	ERR_FAIL_NULL_V_MSG(
		box, false,
		vformat("Failed to create GUIOverlappingElementsBox for GUI overlapping elements %s", overlapping_elements_name)
	);
	box->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

	if (box->set_gui_overlapping_elements_box(&overlapping_elements) != OK) {
		UtilityFunctions::push_error(
			"Error initialising GUIOverlappingElementsBox for GUI overlapping elements ", overlapping_elements_name
		);
		ret = false;
	}

	args.result = box;
	return ret;
}

static bool generate_listbox(generate_gui_args_t&& args) {
	GUI::ListBox const& listbox = static_cast<GUI::ListBox const&>(args.element);

	const String listbox_name = Utilities::std_to_godot_string(listbox.get_name());

	GUIListBox* gui_listbox = nullptr;
	bool ret = new_control(gui_listbox, listbox, args.name);
	ERR_FAIL_NULL_V_MSG(gui_listbox, false, vformat("Failed to create GUIListBox for GUI listbox %s", listbox_name));

	if (gui_listbox->set_gui_listbox(&listbox) != OK) {
		UtilityFunctions::push_error("Error initialising GUIListBox for GUI listbox ", listbox_name);
		ret = false;
	}

	args.result = gui_listbox;
	return ret;
}

static bool generate_texteditbox(generate_gui_args_t&& args) {
	using namespace OpenVic::Utilities::literals;

	GUI::TextEditBox const& text_edit_box = static_cast<GUI::TextEditBox const&>(args.element);

	const String text_edit_box_name = Utilities::std_to_godot_string(text_edit_box.get_name());

	LineEdit* godot_line_edit = nullptr;
	bool ret = new_control(godot_line_edit, text_edit_box, args.name);
	ERR_FAIL_NULL_V_MSG(
		godot_line_edit, false, vformat("Failed to create LineEdit for GUI text edit box %s", text_edit_box_name)
	);

	godot_line_edit->set_context_menu_enabled(false);
	godot_line_edit->set_caret_blink_enabled(true);
	godot_line_edit->set_focus_mode(Control::FOCUS_CLICK);

	godot_line_edit->set_text(Utilities::std_to_godot_string(text_edit_box.get_text()));

	static const Vector2 default_position_padding { -4.0_real, 1.0_real };
	static const Vector2 default_size_padding { 2.0_real, 2.0_real };
	const Vector2 border_size = Utilities::to_godot_fvec2(text_edit_box.get_border_size());
	const Vector2 max_size = Utilities::to_godot_fvec2(text_edit_box.get_size());
	godot_line_edit->set_position(godot_line_edit->get_position() + border_size + default_position_padding);
	godot_line_edit->set_custom_minimum_size(max_size - border_size - default_size_padding);

	static const StringName caret_color_theme = "caret_color";
	static const Color caret_colour { 1.0_real, 0.0_real, 0.0_real };
	godot_line_edit->add_theme_color_override(caret_color_theme, caret_colour);

	if (text_edit_box.get_font() != nullptr) {
		const StringName font_file = Utilities::std_to_godot_string(text_edit_box.get_font()->get_fontname());
		const Ref<Font> font = args.asset_manager.get_font(font_file);
		if (font.is_valid()) {
			static const StringName font_theme = "font";
			godot_line_edit->add_theme_font_override(font_theme, font);
		} else {
			UtilityFunctions::push_error("Failed to load font \"", font_file, "\" for GUI text edit box", text_edit_box_name);
			ret = false;
		}
		const Color colour = Utilities::to_godot_color(text_edit_box.get_font()->get_colour());
		static const StringName font_color_theme = "font_color";
		godot_line_edit->add_theme_color_override(font_color_theme, colour);
	}

	const StringName texture_file = Utilities::std_to_godot_string(text_edit_box.get_texture_file());
	if (!texture_file.is_empty()) {
		Ref<ImageTexture> texture = args.asset_manager.get_texture(texture_file);

		if (texture.is_valid()) {
			static const StringName normal_theme = "normal";
			ret &= add_theme_stylebox(godot_line_edit, normal_theme, texture, border_size);
		} else {
			UtilityFunctions::push_error(
				"Failed to load texture \"", texture_file, "\" for text edit box \"", text_edit_box_name, "\""
			);
			ret = false;
		}
	}

	Ref<StyleBoxEmpty> stylebox_empty;
	stylebox_empty.instantiate();
	if (stylebox_empty.is_valid()) {
		static const StringName focus_theme = "focus";
		godot_line_edit->add_theme_stylebox_override(focus_theme, stylebox_empty);
	} else {
		UtilityFunctions::push_error("Failed to create empty style box for focus of GUI text edit box ", text_edit_box_name);
		ret = false;
	}

	args.result = godot_line_edit;
	return ret;
}

static bool generate_scrollbar(generate_gui_args_t&& args) {
	GUI::Scrollbar const& scrollbar = static_cast<GUI::Scrollbar const&>(args.element);

	const String scrollbar_name = Utilities::std_to_godot_string(scrollbar.get_name());

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
	using namespace OpenVic::Utilities::literals;

	GUI::Window const& window = static_cast<GUI::Window const&>(args.element);

	// TODO - moveable, fullscreen, dontRender (disable visibility?)
	const String window_name = Utilities::std_to_godot_string(window.get_name());

	Panel* godot_panel = nullptr;
	bool ret = new_control(godot_panel, window, args.name);
	ERR_FAIL_NULL_V_MSG(godot_panel, false, vformat("Failed to create Panel for GUI window %s", window_name));

	godot_panel->set_custom_minimum_size(Utilities::to_godot_fvec2(window.get_size()));
	godot_panel->set_self_modulate({ 1.0_real, 1.0_real, 1.0_real, 0.0_real });

	for (std::unique_ptr<GUI::Element> const& element : window.get_window_elements()) {
		Control* node = nullptr;
		const bool element_ret = generate_element(element.get(), "", args.asset_manager, node);
		if (node != nullptr) {
			godot_panel->add_child(node);
		}
		if (!element_ret) {
			UtilityFunctions::push_error(
				"Errors generating GUI element ", Utilities::std_to_godot_string(element->get_name())
			);
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
		it == type_map.end(), false,
		vformat("Invalid GUI element type: %s", Utilities::std_to_godot_string(element->get_type()))
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
	String const& gui_scene, String const& gui_element, String const& name, Control*& result
) {
	return generate_gui_element(get_gui_element(gui_scene, gui_element), name, result);
}
