#include "GFXButtonStateTexture.hpp"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/utility/Typedefs.hpp>

#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/StaticString.hpp"

using namespace OpenVic;
using namespace godot;

void GFXCorneredTileSupportingTexture::_bind_methods() {
	OV_BIND_METHOD(GFXCorneredTileSupportingTexture::get_cornered_tile_border_size);
	OV_BIND_METHOD(GFXCorneredTileSupportingTexture::is_cornered_tile_texture);
	OV_BIND_METHOD(GFXCorneredTileSupportingTexture::draw_rect_cornered, { "to_canvas_item", "rect" });
}

GFXCorneredTileSupportingTexture::GFXCorneredTileSupportingTexture() {}

bool GFXCorneredTileSupportingTexture::is_cornered_tile_texture() const {
	return cornered_tile_border_size != Vector2i {};
}

void GFXCorneredTileSupportingTexture::draw_rect_cornered(RID const& to_canvas_item, Rect2 const& rect) const {
	if (is_cornered_tile_texture()) {
		RenderingServer* rendering_server = RenderingServer::get_singleton();
		if (rendering_server != nullptr) {
			const Size2 size = get_size();
			rendering_server->canvas_item_add_nine_patch(
				to_canvas_item, rect, { {}, size }, get_rid(),
				cornered_tile_border_size, size - cornered_tile_border_size
			);
		}
	} else {
		draw_rect(to_canvas_item, rect, false);
	}
}

void GFXButtonStateTexture::_bind_methods() {
	OV_BIND_METHOD(GFXButtonStateTexture::set_button_state, { "new_button_state" });
	OV_BIND_METHOD(GFXButtonStateTexture::get_button_state);

	OV_BIND_SMETHOD(button_state_to_name, { "button_state" });
	OV_BIND_METHOD(GFXButtonStateTexture::get_button_state_name);

	OV_BIND_METHOD(GFXButtonStateTexture::generate_state_image, { "source_image", "region", "new_cornered_tile_border_size" });

	BIND_ENUM_CONSTANT(HOVER);
	BIND_ENUM_CONSTANT(PRESSED);
	BIND_ENUM_CONSTANT(DISABLED);
	BIND_ENUM_CONSTANT(SELECTED);
}

GFXButtonStateTexture::GFXButtonStateTexture() {}

Ref<GFXButtonStateTexture> GFXButtonStateTexture::make_gfx_button_state_texture(
	ButtonState button_state, Ref<Image> const& source_image, Rect2i const& region, Vector2i const& cornered_tile_border_size
) {
	Ref<GFXButtonStateTexture> button_state_texture;
	button_state_texture.instantiate();
	ERR_FAIL_NULL_V(button_state_texture, nullptr);
	button_state_texture->set_button_state(button_state);
	if (source_image.is_valid()) {
		ERR_FAIL_COND_V(
			button_state_texture->generate_state_image(source_image, region, cornered_tile_border_size) != OK, nullptr
		);
	}
	return button_state_texture;
}

void GFXButtonStateTexture::set_button_state(ButtonState new_button_state) {
	ERR_FAIL_COND(
		new_button_state != HOVER && new_button_state != PRESSED &&
		new_button_state != DISABLED && new_button_state != SELECTED
	);
	button_state = new_button_state;
}

Error GFXButtonStateTexture::generate_state_image(
	Ref<Image> const& source_image, Rect2i const& region, Vector2i const& new_cornered_tile_border_size
) {
	ERR_FAIL_COND_V(source_image.is_null() || source_image->is_empty(), FAILED);
	const Rect2i source_image_rect { {}, source_image->get_size() };
	ERR_FAIL_COND_V(!region.has_area() || !source_image_rect.encloses(region), FAILED);
	/* Whether we've already set the ImageTexture to an image of the right dimensions and format,
	* and so can update it without creating and setting a new image, or not. */
	bool can_update = state_image.is_valid() && state_image->get_size() == region.get_size()
		&& state_image->get_format() == source_image->get_format();
	if (!can_update) {
		state_image = Image::create(region.size.width, region.size.height, false, source_image->get_format());
		ERR_FAIL_NULL_V(state_image, FAILED);
	}

	if (state_texture.is_null()) {
		can_update = false;
		state_texture.instantiate();
		ERR_FAIL_NULL_V(state_texture, FAILED);
		set_atlas(state_texture);
		set_region({ {}, state_image->get_size() });
	}

	cornered_tile_border_size = new_cornered_tile_border_size;

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
	static constexpr auto selected_colour = [](Color const& colour) -> Color {
		return { std::max(colour.r - 0.3f, 0.0f), std::max(colour.g - 0.3f, 0.0f), std::max(colour.b - 0.3f, 0.0f), colour.a };
	};

	const auto colour_func =
		button_state == HOVER ? hover_colour :
		button_state == PRESSED ? pressed_colour :
		button_state == DISABLED ? disabled_colour :
		selected_colour;

	for (Vector2i point { 0, 0 }; point.y < state_image->get_height(); ++point.y) {
		for (point.x = 0; point.x < state_image->get_width(); ++point.x) {
			state_image->set_pixelv(point, colour_func(source_image->get_pixelv(region.position + point)));
		}
	}

	if (can_update) {
		state_texture->update(state_image);
	} else {
		state_texture->set_image(state_image);
	}
	return OK;
}

StringName const& GFXButtonStateTexture::button_state_to_name(ButtonState button_state) {
	switch (button_state) {
		case HOVER:
			return OV_SNAME(hover);
		case PRESSED:
			return OV_SNAME(pressed);
		case DISABLED:
			return OV_SNAME(disabled);
		case SELECTED:
			return OV_SNAME(selected);
		default:
			unreachable();
	}
}

StringName const& GFXButtonStateTexture::get_button_state_name() const {
	return button_state_to_name(button_state);
}

void GFXButtonStateHavingTexture::_bind_methods() {
	OV_BIND_METHOD(GFXButtonStateHavingTexture::get_button_state_texture, { "button_state" });
}

void GFXButtonStateHavingTexture::_update_button_states() {
	for (Ref<GFXButtonStateTexture>& button_state_texture : button_state_textures) {
		if (button_state_texture.is_valid()) {
			button_state_texture->generate_state_image(button_image, get_region(), cornered_tile_border_size);
		}
	}
}

void GFXButtonStateHavingTexture::_clear_button_states() {
	set_atlas(nullptr);
	set_region({});
	button_image.unref();
	for (Ref<GFXButtonStateTexture>& button_state_texture : button_state_textures) {
		button_state_texture.unref();
	}
}

GFXButtonStateHavingTexture::GFXButtonStateHavingTexture() : button_state_textures {} {}

Ref<GFXButtonStateTexture> GFXButtonStateHavingTexture::get_button_state_texture(
	GFXButtonStateTexture::ButtonState button_state
) {
	const size_t button_state_index = button_state;
	ERR_FAIL_COND_V(button_state_index >= button_state_textures.size(), nullptr);
	Ref<GFXButtonStateTexture>& button_state_texture = button_state_textures[button_state_index];
	if (button_state_texture.is_null()) {
		button_state_texture = GFXButtonStateTexture::make_gfx_button_state_texture(
			button_state, button_image, get_region(), cornered_tile_border_size
		);
		ERR_FAIL_NULL_V(button_state_texture, nullptr);
	}
	return button_state_texture;
}
