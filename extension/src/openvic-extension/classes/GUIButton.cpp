#include "GUIButton.hpp"

#include <array>

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

GUI_TOOLTIP_IMPLEMENTATIONS(GUIButton)

void GUIButton::_bind_methods() {
	GUI_TOOLTIP_BIND_METHODS(GUIButton)
}

void GUIButton::_notification(int what) {
	_tooltip_notification(what);
}

GUIButton::GUIButton() : tooltip_active { false } {}

Error GUIButton::set_gfx_button_state_having_texture(Ref<GFXButtonStateHavingTexture> const& texture) {
	ERR_FAIL_NULL_V(texture, FAILED);

	Error err = OK;

	set_custom_minimum_size(texture->get_size());

	{
		Ref<StyleBoxTexture> stylebox = AssetManager::make_stylebox_texture(texture);

		if (stylebox.is_valid()) {
			static const StringName normal_theme = "normal";

			add_theme_stylebox_override(normal_theme, stylebox);
		} else {
			UtilityFunctions::push_error("Failed to make StyleBoxTexture for GUIButton ", get_name());

			err = FAILED;
		}
	}

	using enum GFXButtonStateTexture::ButtonState;

	for (GFXButtonStateTexture::ButtonState button_state : { HOVER, PRESSED, DISABLED }) {
		Ref<GFXButtonStateTexture> button_state_texture = texture->get_button_state_texture(button_state);

		if (button_state_texture.is_valid()) {
			Ref<StyleBoxTexture> stylebox = AssetManager::make_stylebox_texture(button_state_texture);

			if (stylebox.is_valid()) {
				add_theme_stylebox_override(button_state_texture->get_button_state_name(), stylebox);
			} else {
				UtilityFunctions::push_error(
					"Failed to make ", GFXButtonStateTexture::button_state_to_name(button_state),
					" StyleBoxTexture for GUIButton ", get_name()
				);

				err = FAILED;
			}
		} else {
			UtilityFunctions::push_error(
				"Failed to make ", GFXButtonStateTexture::button_state_to_name(button_state),
				" GFXButtonStateTexture for GUIButton ", get_name()
			);

			err = FAILED;
		}
	}

	return err;
}

Error GUIButton::set_gfx_font(GFX::Font const* gfx_font) {
	ERR_FAIL_NULL_V(gfx_font, FAILED);

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);

	Error err = OK;

	const StringName font_file = Utilities::std_to_godot_string(gfx_font->get_fontname());
	const Ref<Font> font = asset_manager->get_font(font_file);

	if (font.is_valid()) {
		static const StringName font_theme = "font";

		add_theme_font_override(font_theme, font);
	} else {
		UtilityFunctions::push_error("Failed to load font \"", font_file, "\" for GUIButton ", get_name());

		err = FAILED;
	}

	static const std::array<StringName, 5> button_font_themes {
		"font_color", "font_hover_color", "font_hover_pressed_color", "font_pressed_color", "font_disabled_color"
	};

	const Color colour = Utilities::to_godot_color(gfx_font->get_colour());

	for (StringName const& theme_name : button_font_themes) {
		add_theme_color_override(theme_name, colour);
	}

	return err;
}
