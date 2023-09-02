#include "GameSingleton.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "Utilities.hpp"

using namespace godot;
using namespace OpenVic;

Error GameSingleton::_generate_terrain_texture_array() {
	Error err = OK;
	if (terrain_variants.size() == 0) {
		UtilityFunctions::push_error("Failed to load terrain textures!");
		return FAILED;
	}
	// TerrainVariant count is limited by the data type representing it in the map image
	if (terrain_variants.size() > TerrainVariant::MAX_TERRIN_VARIANT_COUNT) {
		UtilityFunctions::push_error("Too many terrain textures - all after the first ",
			static_cast<uint64_t>(TerrainVariant::MAX_TERRIN_VARIANT_COUNT), " will be ignored");
		err = FAILED;
	}

	Array terrain_images;
	for (size_t i = 0; i < terrain_variants.size() && i < TerrainVariant::MAX_TERRIN_VARIANT_COUNT; ++i) {
		TerrainVariant const& var = *terrain_variants.get_item_by_index(i);
		terrain_variant_map[var.get_colour()] = i;
		terrain_images.append(var.get_image());
	}

	terrain_texture.instantiate();
	if (terrain_texture->create_from_images(terrain_images) != OK) {
		UtilityFunctions::push_error("Failed to create terrain texture array!");
		return FAILED;
	}
	return err;
}

Error GameSingleton::_load_map_images(String const& province_image_path, String const& terrain_image_path, bool flip_vertical) {
	if (province_shape_texture.is_valid()) {
		UtilityFunctions::push_error("Map images have already been loaded, cannot load: ", province_image_path, " and ", terrain_image_path);
		return FAILED;
	}

	// Load images
	Ref<Image> province_image = load_godot_image(province_image_path);
	if (province_image.is_null()) {
		UtilityFunctions::push_error("Failed to load province image: ", province_image_path);
		return FAILED;
	}
	Ref<Image> terrain_image = load_godot_image(terrain_image_path);
	if (terrain_image.is_null()) {
		UtilityFunctions::push_error("Failed to load terrain image: ", terrain_image_path);
		return FAILED;
	}

	if (flip_vertical) {
		province_image->flip_y();
		terrain_image->flip_y();
	}

	// Validate dimensions and format
	Error err = OK;
	const Vector2i province_dims = province_image->get_size(), terrain_dims = terrain_image->get_size();
	if (province_dims.x < 1 || province_dims.y < 1) {
		UtilityFunctions::push_error("Invalid dimensions (", province_dims.x, "x", province_dims.y, ") for province image: ", province_image_path);
		err = FAILED;
	}
	if (province_dims != terrain_dims) {
		UtilityFunctions::push_error("Invalid dimensions (", terrain_dims.x, "x", terrain_dims.y, ") for terrain image: ",
			terrain_image_path, " (must match province image: (", province_dims.x, "x", province_dims.x, "))");
		err = FAILED;
	}
	static constexpr Image::Format expected_format = Image::FORMAT_RGB8;
	if (province_image->get_format() == Image::FORMAT_RGBA8) province_image->convert(expected_format);
	if (terrain_image->get_format() == Image::FORMAT_RGBA8) terrain_image->convert(expected_format);
	if (province_image->get_format() != expected_format) {
		UtilityFunctions::push_error("Invalid format (", province_image->get_format(), ", should be ", expected_format, ") for province image: ", province_image_path);
		err = FAILED;
	}
	if (terrain_image->get_format() != expected_format) {
		UtilityFunctions::push_error("Invalid format (", terrain_image->get_format(), ", should be ", expected_format, ") for terrain image: ", terrain_image_path);
		err = FAILED;
	}
	if (err != OK) return err;

	// Generate interleaved province and terrain ID image
	if (game_manager.map.generate_province_shape_image(province_dims.x, province_dims.y, province_image->get_data().ptr(),
		terrain_image->get_data().ptr(), terrain_variant_map, false) != SUCCESS) err = FAILED;

	static constexpr int32_t GPU_DIM_LIMIT = 0x3FFF;
	// For each dimension of the image, this finds the small number of equal subdivisions required get the individual texture dims under GPU_DIM_LIMIT
	for (int i = 0; i < 2; ++i)
		for (image_subdivisions[i] = 1; province_dims[i] / image_subdivisions[i] > GPU_DIM_LIMIT ||
			province_dims[i] % image_subdivisions[i] != 0; ++image_subdivisions[i]);

	Map::shape_pixel_t const* province_shape_data = game_manager.map.get_province_shape_image().data();
	const Vector2i divided_dims = province_dims / image_subdivisions;
	Array province_shape_images;
	province_shape_images.resize(image_subdivisions.x * image_subdivisions.y);
	for (int32_t v = 0; v < image_subdivisions.y; ++v) {
		for (int32_t u = 0; u < image_subdivisions.x; ++u) {
			PackedByteArray index_data_array;
			index_data_array.resize(divided_dims.x * divided_dims.y * sizeof(Map::shape_pixel_t));

			for (int32_t y = 0; y < divided_dims.y; ++y)
				memcpy(index_data_array.ptrw() + y * divided_dims.x * sizeof(Map::shape_pixel_t),
					province_shape_data + (v * divided_dims.y + y) * province_dims.x + u * divided_dims.x,
					divided_dims.x * sizeof(Map::shape_pixel_t));

			const Ref<Image> province_shape_subimage = Image::create_from_data(divided_dims.x, divided_dims.y, false, Image::FORMAT_RGB8, index_data_array);
			if (province_shape_subimage.is_null()) {
				UtilityFunctions::push_error("Failed to create province shape image (", u, ", ", v, ")");
				err = FAILED;
			}
			province_shape_images[u + v * image_subdivisions.x] = province_shape_subimage;
		}
	}

	province_shape_texture.instantiate();
	if (province_shape_texture->create_from_images(province_shape_images) != OK) {
		UtilityFunctions::push_error("Failed to create terrain texture array!");
		err = FAILED;
	}

	if (_update_colour_image() != OK) err = FAILED;

	return err;
}
