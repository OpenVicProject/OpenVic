#include "GFXButtonStateTexture.hpp"

#include "openvic-extension/utility/ClassBindings.hpp"

using namespace OpenVic;
using namespace godot;

void GFXButtonStateTexture::_bind_methods() {
	OV_BIND_METHOD(GFXButtonStateTexture::set_button_state, { "new_button_state" });
	OV_BIND_METHOD(GFXButtonStateTexture::get_button_state);

	OV_BIND_SMETHOD(get_generate_state_image_func_name);

	OV_BIND_SMETHOD(button_state_to_theme_name, { "button_state" });
	OV_BIND_METHOD(GFXButtonStateTexture::get_button_state_theme);

	OV_BIND_METHOD(GFXButtonStateTexture::generate_state_image, { "source_image" });

	BIND_ENUM_CONSTANT(HOVER);
	BIND_ENUM_CONSTANT(PRESSED);
	BIND_ENUM_CONSTANT(DISABLED);
}

GFXButtonStateTexture::GFXButtonStateTexture() : button_state { HOVER } {}

Ref<GFXButtonStateTexture> GFXButtonStateTexture::make_gfx_button_state_texture(
	ButtonState button_state, Ref<Image> const& source_image
) {
	Ref<GFXButtonStateTexture> button_state_texture;
	button_state_texture.instantiate();
	ERR_FAIL_NULL_V(button_state_texture, nullptr);
	button_state_texture->set_button_state(button_state);
	if (source_image.is_valid()) {
		ERR_FAIL_COND_V(button_state_texture->generate_state_image(source_image) != OK, nullptr);
	}
	return button_state_texture;
}

void GFXButtonStateTexture::set_button_state(ButtonState new_button_state) {
	ERR_FAIL_COND(new_button_state != HOVER && new_button_state != PRESSED && new_button_state != DISABLED);
	button_state = new_button_state;
}

Error GFXButtonStateTexture::generate_state_image(Ref<Image> const& source_image) {
	ERR_FAIL_COND_V(source_image.is_null() || source_image->is_empty(), FAILED);
	/* Whether we've already set the ImageTexture to an image of the right dimensions and format,
	* and so can update it without creating and setting a new image, or not. */
	const bool can_update = state_image.is_valid() && state_image->get_size() == source_image->get_size()
		&& state_image->get_format() == source_image->get_format();
	if (!can_update) {
		state_image = Image::create(source_image->get_width(), source_image->get_height(), false, source_image->get_format());
		ERR_FAIL_NULL_V(state_image, FAILED);
	}

	static constexpr auto hover_colour = [](Color const& colour) -> Color {
		return { std::min(colour.r + 0.1f, 1.0f), std::min(colour.g + 0.1f, 1.0f), std::min(colour.b + 0.1f, 1.0f), colour.a };
	};
	static constexpr auto pressed_colour = [](Color const& colour) -> Color {
		return { std::max(colour.r - 0.1f, 0.0f), std::max(colour.g - 0.1f, 0.0f), std::max(colour.b - 0.1f, 0.0f), colour.a };
	};
	static constexpr auto disabled_colour = [](Color const& colour) -> Color {
		const float luma = colour.get_luminance();
		return { luma, luma, luma, colour.a };
	};

	const auto colour_func = button_state == HOVER ? hover_colour : button_state == PRESSED ? pressed_colour : disabled_colour;

	for (Vector2i point { 0, 0 }; point.y < state_image->get_height(); ++point.y) {
		for (point.x = 0; point.x < state_image->get_width(); ++point.x) {
			state_image->set_pixelv(point, colour_func(source_image->get_pixelv(point)));
		}
	}

	if (can_update) {
		update(state_image);
	} else {
		set_image(state_image);
	}
	return OK;
}

StringName const& GFXButtonStateTexture::get_generate_state_image_func_name() {
	static const StringName generate_state_image_func_name = "generate_state_image";
	return generate_state_image_func_name;
}

StringName const& GFXButtonStateTexture::button_state_to_theme_name(ButtonState button_state) {
	static const StringName theme_name_hover = "hover";
	static const StringName theme_name_pressed = "pressed";
	static const StringName theme_name_disabled = "disabled";
	static const StringName theme_name_error = "";
	switch (button_state) {
		case HOVER:
			return theme_name_hover;
		case PRESSED:
			return theme_name_pressed;
		case DISABLED:
			return theme_name_disabled;
		default:
			return theme_name_error;
	}
}

StringName const& GFXButtonStateTexture::get_button_state_theme() const {
	return button_state_to_theme_name(button_state);
}
