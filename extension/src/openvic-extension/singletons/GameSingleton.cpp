#include "GameSingleton.hpp"

#include <functional>

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/utility/Logger.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/LoadLocalisation.hpp"
#include "openvic-extension/singletons/MenuSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_to_godot_string;
using OpenVic::Utilities::std_to_godot_string_name;
using OpenVic::Utilities::std_view_to_godot_string;

/* Maximum width or height a GPU texture can have. */
static constexpr int32_t GPU_DIM_LIMIT = 0x3FFF;

/* StringNames cannot be constructed until Godot has called StringName::setup(),
 * so we must use these wrapper functions to delay their initialisation. */
StringName const& GameSingleton::_signal_gamestate_updated() {
	static const StringName signal_gamestate_updated = "gamestate_updated";
	return signal_gamestate_updated;
}
StringName const& GameSingleton::_signal_province_selected() {
	static const StringName signal_province_selected = "province_selected";
	return signal_province_selected;
}
StringName const& GameSingleton::_signal_clock_state_changed() {
	static const StringName signal_clock_state_changed = "clock_state_changed";
	return signal_clock_state_changed;
}

void GameSingleton::_bind_methods() {
	OV_BIND_SMETHOD(setup_logger);

	OV_BIND_METHOD(GameSingleton::load_defines_compatibility_mode, { "file_paths" });
	OV_BIND_SMETHOD(search_for_game_path, { "hint_path" }, DEFVAL(String {}));

	OV_BIND_METHOD(GameSingleton::setup_game, { "bookmark_index" });

	OV_BIND_METHOD(GameSingleton::get_province_index_from_uv_coords, { "coords" });

	OV_BIND_METHOD(GameSingleton::get_map_width);
	OV_BIND_METHOD(GameSingleton::get_map_height);
	OV_BIND_METHOD(GameSingleton::get_map_aspect_ratio);
	OV_BIND_METHOD(GameSingleton::get_terrain_texture);
	OV_BIND_METHOD(GameSingleton::get_province_shape_image_subdivisions);
	OV_BIND_METHOD(GameSingleton::get_province_shape_texture);
	OV_BIND_METHOD(GameSingleton::get_province_colour_texture);

	OV_BIND_METHOD(GameSingleton::get_mapmode_count);
	OV_BIND_METHOD(GameSingleton::get_mapmode_identifier);
	OV_BIND_METHOD(GameSingleton::set_mapmode, { "identifier" });
	OV_BIND_METHOD(GameSingleton::is_parchment_mapmode_allowed);
	OV_BIND_METHOD(GameSingleton::get_selected_province_index);
	OV_BIND_METHOD(GameSingleton::set_selected_province, { "index" });

	OV_BIND_METHOD(GameSingleton::try_tick);

	ADD_SIGNAL(MethodInfo(_signal_gamestate_updated()));
	ADD_SIGNAL(MethodInfo(_signal_province_selected(), PropertyInfo(Variant::INT, "index")));
	ADD_SIGNAL(MethodInfo(_signal_clock_state_changed()));
}

GameSingleton* GameSingleton::get_singleton() {
	return singleton;
}

void GameSingleton::_on_gamestate_updated() {
	_update_colour_image();
	emit_signal(_signal_gamestate_updated());
}

void GameSingleton::_on_clock_state_changed() {
	emit_signal(_signal_clock_state_changed());
}

/* REQUIREMENTS:
 * MAP-21, MAP-23, MAP-25, MAP-32, MAP-33, MAP-34
 */
GameSingleton::GameSingleton()
	: game_manager {
		std::bind(&GameSingleton::_on_gamestate_updated, this), std::bind(&GameSingleton::_on_clock_state_changed, this)
	} {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

GameSingleton::~GameSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

void GameSingleton::setup_logger() {
	Logger::set_info_func([](std::string&& str) {
		UtilityFunctions::print(std_to_godot_string(str));
	});
	Logger::set_warning_func([](std::string&& str) {
		UtilityFunctions::push_warning(std_to_godot_string(str));
	});
	Logger::set_error_func([](std::string&& str) {
		UtilityFunctions::push_error(std_to_godot_string(str));
	});
}

Error GameSingleton::setup_game(int32_t bookmark_index) {
	Bookmark const* bookmark = game_manager.get_history_manager().get_bookmark_manager().get_bookmark_by_index(bookmark_index);
	ERR_FAIL_NULL_V_MSG(bookmark, FAILED, vformat("Failed to get bookmark with index: %d", bookmark_index));
	bool ret = game_manager.load_bookmark(bookmark);

	for (Province& province : game_manager.get_map().get_provinces()) {
		province.set_crime(
			game_manager.get_crime_manager().get_crime_modifier_by_index(
				(province.get_index() - 1) % game_manager.get_crime_manager().get_crime_modifier_count()
			)
		);
	}

	MenuSingleton* menu_singleton = MenuSingleton::get_singleton();
	ERR_FAIL_NULL_V(menu_singleton, FAILED);
	menu_singleton->_population_menu_update_provinces();

	return ERR(ret);
}

int32_t GameSingleton::get_province_index_from_uv_coords(Vector2 const& coords) const {
	const size_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * get_map_width();
	const size_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * get_map_height();
	return game_manager.get_map().get_province_index_at(x_mod_w, y_mod_h);
}

int32_t GameSingleton::get_map_width() const {
	return game_manager.get_map().get_width();
}

int32_t GameSingleton::get_map_height() const {
	return game_manager.get_map().get_height();
}

float GameSingleton::get_map_aspect_ratio() const {
	return static_cast<float>(get_map_width()) / static_cast<float>(get_map_height());
}

Ref<Texture2DArray> GameSingleton::get_terrain_texture() const {
	return terrain_texture;
}

Ref<Image> GameSingleton::get_flag_image(Country const* country, StringName const& flag_type) const {
	ERR_FAIL_NULL_V(country, nullptr);
	const typename decltype(flag_image_map)::const_iterator it = flag_image_map.find(country);
	ERR_FAIL_COND_V_MSG(
		it == flag_image_map.end(), nullptr,
		vformat("Failed to find flags for country: %s", std_view_to_godot_string(country->get_identifier()))
	);
	const typename decltype(it->second)::const_iterator it2 = it->second.find(flag_type);
	ERR_FAIL_COND_V_MSG(
		it2 == it->second.end(), nullptr,
		vformat("Failed to find %s flag for country: %s", flag_type, std_view_to_godot_string(country->get_identifier()))
	);
	return it2->second;
}

Vector2i GameSingleton::get_province_shape_image_subdivisions() const {
	return image_subdivisions;
}

Ref<Texture2DArray> GameSingleton::get_province_shape_texture() const {
	return province_shape_texture;
}

Ref<ImageTexture> GameSingleton::get_province_colour_texture() const {
	return province_colour_texture;
}

Error GameSingleton::_update_colour_image() {
	Map const& map = game_manager.get_map();
	ERR_FAIL_COND_V_MSG(
		!map.provinces_are_locked(), FAILED, "Cannot generate province colour image before provinces are locked!"
	);

	/* We reshape the list of colours into a square, as each texture dimensions cannot exceed 16384. */
	static constexpr int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(Province::index_t) * CHAR_BIT / 2);
	static constexpr int32_t colour_image_width = PROVINCE_INDEX_SQRT * sizeof(Mapmode::base_stripe_t) / sizeof(colour_argb_t);
	/* Province count + null province, rounded up to next multiple of PROVINCE_INDEX_SQRT.
	 * Rearranged from: (map.get_province_count() + 1) + (PROVINCE_INDEX_SQRT - 1) */
	static const int32_t colour_image_height = (map.get_province_count() + PROVINCE_INDEX_SQRT) / PROVINCE_INDEX_SQRT;

	static PackedByteArray colour_data_array;
	static const int64_t colour_data_array_size = colour_image_width * colour_image_height * sizeof(colour_argb_t);
	colour_data_array.resize(colour_data_array_size);

	Error err = OK;
	if (!map.generate_mapmode_colours(mapmode_index, colour_data_array.ptrw())) {
		err = FAILED;
	}

	if (province_colour_image.is_null()) {
		province_colour_image.instantiate();
		ERR_FAIL_NULL_V_EDMSG(province_colour_image, FAILED, "Failed to create province colour image");
	}
	/* Width is doubled as each province has a (base, stripe) colour pair. */
	province_colour_image->set_data(
		colour_image_width, colour_image_height, false, Image::FORMAT_RGBA8, colour_data_array
	);
	if (province_colour_texture.is_null()) {
		province_colour_texture = ImageTexture::create_from_image(province_colour_image);
		ERR_FAIL_NULL_V_EDMSG(province_colour_texture, FAILED, "Failed to create province colour texture");
	} else {
		province_colour_texture->update(province_colour_image);
	}
	return err;
}

int32_t GameSingleton::get_mapmode_count() const {
	return game_manager.get_map().get_mapmode_count();
}

String GameSingleton::get_mapmode_identifier(int32_t index) const {
	Mapmode const* mapmode = game_manager.get_map().get_mapmode_by_index(index);
	if (mapmode != nullptr) {
		return std_view_to_godot_string(mapmode->get_identifier());
	}
	return String {};
}

Error GameSingleton::set_mapmode(String const& identifier) {
	Mapmode const* mapmode = game_manager.get_map().get_mapmode_by_identifier(godot_to_std_string(identifier));
	ERR_FAIL_NULL_V_MSG(mapmode, FAILED, vformat("Failed to find mapmode with identifier: %s", identifier));
	mapmode_index = mapmode->get_index();
	return _update_colour_image();
}

bool GameSingleton::is_parchment_mapmode_allowed() const {
	// TODO - parchment bool per mapmode?
	// TODO - move mapmode index to SIM/Map?
	/* Disallows parchment mapmode for the cosmetic terrain mapmode */
	static constexpr std::string_view cosmetic_terrain_mapmode = "mapmode_terrain";
	Mapmode const* mapmode = game_manager.get_map().get_mapmode_by_index(mapmode_index);
	return mapmode != nullptr && mapmode->get_identifier() != cosmetic_terrain_mapmode;
}

int32_t GameSingleton::get_selected_province_index() const {
	return game_manager.get_map().get_selected_province_index();
}

void GameSingleton::set_selected_province(int32_t index) {
	game_manager.get_map().set_selected_province(index);
	_update_colour_image();
	emit_signal(_signal_province_selected(), index);
}

void GameSingleton::try_tick() {
	game_manager.get_simulation_clock().conditionally_advance_game();
}

Error GameSingleton::_load_map_images() {
	ERR_FAIL_COND_V_MSG(province_shape_texture.is_valid(), FAILED, "Map images have already been loaded!");

	Error err = OK;

	const Vector2i province_dims {
		static_cast<int32_t>(game_manager.get_map().get_width()),
		static_cast<int32_t>(game_manager.get_map().get_height())
	};

	// For each dimension of the image, this finds the small number of equal subdivisions
	// required get the individual texture dims under GPU_DIM_LIMIT
	for (int i = 0; i < 2; ++i) {
		image_subdivisions[i] = 1;
		while (province_dims[i] / image_subdivisions[i] > GPU_DIM_LIMIT || province_dims[i] % image_subdivisions[i] != 0) {
			++image_subdivisions[i];
		}
	}

	Map::shape_pixel_t const* province_shape_data = game_manager.get_map().get_province_shape_image().data();
	const Vector2i divided_dims = province_dims / image_subdivisions;
	TypedArray<Image> province_shape_images;
	province_shape_images.resize(image_subdivisions.x * image_subdivisions.y);
	for (int32_t v = 0; v < image_subdivisions.y; ++v) {
		for (int32_t u = 0; u < image_subdivisions.x; ++u) {
			PackedByteArray index_data_array;
			index_data_array.resize(divided_dims.x * divided_dims.y * sizeof(Map::shape_pixel_t));

			for (int32_t y = 0; y < divided_dims.y; ++y) {
				memcpy(
					index_data_array.ptrw() + y * divided_dims.x * sizeof(Map::shape_pixel_t),
					province_shape_data + (v * divided_dims.y + y) * province_dims.x + u * divided_dims.x,
					divided_dims.x * sizeof(Map::shape_pixel_t)
				);
			}

			const Ref<Image> province_shape_subimage =
				Image::create_from_data(divided_dims.x, divided_dims.y, false, Image::FORMAT_RGB8, index_data_array);
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

	if (_update_colour_image() != OK) {
		err = FAILED;
	}

	return err;
}

Error GameSingleton::_load_terrain_variants() {
	ERR_FAIL_COND_V_MSG(terrain_texture.is_valid(), FAILED, "Terrain variants have already been loaded!");

	static const StringName terrain_texturesheet_path = "map/terrain/texturesheet.tga";

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);
	// Load the terrain texture sheet and prepare to slice it up
	Ref<Image> terrain_sheet = asset_manager->get_image(terrain_texturesheet_path);
	ERR_FAIL_NULL_V_MSG(terrain_sheet, FAILED, vformat("Failed to load terrain texture sheet: %s", terrain_texturesheet_path));

	static constexpr int32_t SHEET_DIMS = 8, SHEET_SIZE = SHEET_DIMS * SHEET_DIMS;

	const int32_t sheet_width = terrain_sheet->get_width(), sheet_height = terrain_sheet->get_height();
	ERR_FAIL_COND_V_MSG(
		sheet_width < 1 || sheet_width % SHEET_DIMS != 0 || sheet_width != sheet_height, FAILED, vformat(
			"Invalid terrain texture sheet dims: %dx%d (must be square with dims positive multiples of %d)",
			sheet_width, sheet_height, SHEET_DIMS
		)
	);
	const int32_t slice_size = sheet_width / SHEET_DIMS;

	TypedArray<Image> terrain_images;
	{
		/* This is a placeholder image so that we don't have to branch to avoid looking up terrain index 0 (water).
		 * It should never appear in game, and so is bright red to to make it obvious if it slips through. */
		const Ref<Image> water_image = Utilities::make_solid_colour_image(
			{ 1.0f, 0.0f, 0.0f }, slice_size, slice_size, terrain_sheet->get_format()
		);
		ERR_FAIL_NULL_V_EDMSG(water_image, FAILED, "Failed to create water terrain image");
		terrain_images.append(water_image);
	}
	Error err = OK;
	for (int32_t idx = 0; idx < SHEET_SIZE; ++idx) {
		const Rect2i slice { idx % SHEET_DIMS * slice_size, idx / SHEET_DIMS * slice_size, slice_size, slice_size };
		const Ref<Image> terrain_image = terrain_sheet->get_region(slice);
		if (terrain_image.is_null() || terrain_image->is_empty()) {
			UtilityFunctions::push_error(
				"Failed to extract terrain texture slice ", slice, " from ", terrain_texturesheet_path
			);
			err = FAILED;
		}
		terrain_images.append(terrain_image);
	}

	terrain_texture.instantiate();
	ERR_FAIL_COND_V_MSG(
		terrain_texture->create_from_images(terrain_images) != OK, FAILED, "Failed to create terrain texture array!"
	);
	return err;
}

Error GameSingleton::_load_flag_images() {
	ERR_FAIL_COND_V_MSG(!flag_image_map.empty(), FAILED, "Flag images have already been loaded!");

	GovernmentTypeManager const& government_type_manager = game_manager.get_politics_manager().get_government_type_manager();
	ERR_FAIL_COND_V_MSG(
		!government_type_manager.government_types_are_locked(), FAILED,
		"Cannot load flag images before government types are locked!"
	);
	CountryManager const& country_manager = game_manager.get_country_manager();
	ERR_FAIL_COND_V_MSG(
		!country_manager.countries_are_locked(), FAILED, "Cannot load flag images before countries are locked!"
	);

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);

	static const String flag_directory = "gfx/flags/";
	static const String flag_separator = "_";
	static const String flag_extension = ".tga";

	std::vector<StringName> flag_types;
	for (std::string const& type : government_type_manager.get_flag_types()) {
		flag_types.emplace_back(std_to_godot_string_name(type));
	}

	flag_image_map.reserve(country_manager.get_countries().size());

	Error ret = OK;
	for (Country const& country : country_manager.get_countries()) {
		ordered_map<StringName, Ref<Image>>& flag_images = flag_image_map[&country];
		flag_images.reserve(flag_types.size());
		const String country_name = std_view_to_godot_string(country.get_identifier());
		for (StringName const& flag_type : flag_types) {
			const StringName flag_path =
				flag_directory + country_name + (flag_type.is_empty() ? "" : flag_separator + flag_type) + flag_extension;
			const Ref<Image> flag_image = asset_manager->get_image(flag_path);
			if (flag_image.is_valid()) {
				flag_images.emplace(flag_type, flag_image);
			} else {
				UtilityFunctions::push_error("Failed to load flag image: ", flag_path);
				ret = FAILED;
			}
		}
	}
	return ret;
}

Error GameSingleton::load_defines_compatibility_mode(PackedStringArray const& file_paths) {
	Dataloader::path_vector_t roots;
	for (String const& path : file_paths) {
		roots.push_back(godot_to_std_string(path));
	}

	Error err = OK;

	if (!dataloader.set_roots(roots)) {
		Logger::error("Failed to set dataloader roots!");
		err = FAILED;
	}
	if (!dataloader.load_defines(game_manager)) {
		UtilityFunctions::push_error("Failed to load defines!");
		err = FAILED;
	}
	if (_load_terrain_variants() != OK) {
		UtilityFunctions::push_error("Failed to load terrain variants!");
		err = FAILED;
	}
	if (_load_flag_images() != OK) {
		UtilityFunctions::push_error("Failed to load flag textures!");
		err = FAILED;
	}
	if (_load_map_images() != OK) {
		UtilityFunctions::push_error("Failed to load map images!");
		err = FAILED;
	}
	if (!game_manager.load_hardcoded_defines()) {
		UtilityFunctions::push_error("Failed to hardcoded defines!");
		err = FAILED;
	}
	auto add_message = std::bind_front(&LoadLocalisation::add_message, LoadLocalisation::get_singleton());
	if (!dataloader.load_localisation_files(add_message)) {
		UtilityFunctions::push_error("Failed to load localisation!");
		err = FAILED;
	}

	return err;
}

String GameSingleton::search_for_game_path(String const& hint_path) {
	return std_to_godot_string(Dataloader::search_for_game_path(godot_to_std_string(hint_path)).string());
}
