#pragma once

#include <godot_cpp/classes/image_texture.hpp>

#include <openvic-simulation/country/Country.hpp>
#include <openvic-simulation/interface/GFX.hpp>

namespace OpenVic {
	class GFXMaskedFlagTexture : public godot::ImageTexture {
		GDCLASS(GFXMaskedFlagTexture, godot::ImageTexture)

		GFX::MaskedFlag const* PROPERTY(gfx_masked_flag);
		Country const* PROPERTY(flag_country);
		godot::StringName PROPERTY(flag_type);

		godot::Ref<godot::Image> overlay_image, mask_image, flag_image, combined_image;

		godot::Error _generate_combined_image();

	protected:
		static void _bind_methods();

	public:
		GFXMaskedFlagTexture();

		/* Create a GFXMaskedFlagTexture using the specific GFX::MaskedFlag.
		 * Returns nullptr if setting gfx_masked_flag fails. */
		static godot::Ref<GFXMaskedFlagTexture> make_gfx_masked_flag_texture(GFX::MaskedFlag const* gfx_masked_flag);

		/* Reset gfx_masked_flag, flag_country and flag_type to nullptr/an empty string, and unreference all images.
		 * This does not affect the godot::ImageTexture, which cannot be reset to a null or empty image. */
		void clear();

		/* Set the GFX::MaskedFlag, load its overlay and mask textures, and regenerate the combined image. */
		godot::Error set_gfx_masked_flag(GFX::MaskedFlag const* new_gfx_masked_flag);

		/* Search for a GFX::MaskedFlag with the specfied name and, if successful, set it using set_gfx_masked_flag. */
		godot::Error set_gfx_masked_flag_name(godot::String const& gfx_masked_flag_name);

		/* Return the name of the GFX::MaskedFlag, or an empty String if it's null */
		godot::String get_gfx_masked_flag_name() const;

		/* Set flag_country and flag_type and update the combined image to use that flag, or no flag if it doesn't exist. */
		godot::Error set_flag_country_and_type(Country const* new_flag_country, godot::StringName const& new_flag_type);

		/* Look up the country with the specified identifier, then call set_flag_country_and_type with the country and
		 * specified flag_type as arguments. */
		godot::Error set_flag_country_name_and_type(
			godot::String const& new_flag_country_name, godot::StringName const& new_flag_type
		);

		/* Return the name of the selected flag's country, or an empty String if it's null */
		godot::String get_flag_country_name() const;
	};
}
