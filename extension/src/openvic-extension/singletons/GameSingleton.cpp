#include "GameSingleton.hpp"

#include <functional>

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/utility/Logger.hpp>

#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/LoadLocalisation.hpp"
#include "openvic-extension/singletons/MenuSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

/* Maximum width or height a GPU texture can have. */
static constexpr int32_t GPU_DIM_LIMIT = 0x3FFF;

/* StringNames cannot be constructed until Godot has called StringName::setup(),
 * so we must use these wrapper functions to delay their initialisation. */
StringName const& GameSingleton::_signal_gamestate_updated() {
	static const StringName signal_gamestate_updated = "gamestate_updated";
	return signal_gamestate_updated;
}
StringName const& GameSingleton::_signal_mapmode_changed() {
	static const StringName signal_mapmode_changed = "mapmode_changed";
	return signal_mapmode_changed;
}

void GameSingleton::_bind_methods() {
	OV_BIND_SMETHOD(setup_logger);

	OV_BIND_METHOD(GameSingleton::load_defines_compatibility_mode);
	OV_BIND_METHOD(GameSingleton::set_compatibility_mode_roots, { "file_paths", "replace_paths" }, DEFVAL(PackedStringArray{}));

	OV_BIND_SMETHOD(search_for_game_path, { "hint_path" }, DEFVAL(String {}));
	OV_BIND_METHOD(GameSingleton::lookup_file_path, { "path" });

	OV_BIND_METHOD(GameSingleton::get_bookmark_info);
	OV_BIND_METHOD(GameSingleton::setup_game, { "bookmark_index" });
	OV_BIND_METHOD(GameSingleton::start_game_session);

	OV_BIND_METHOD(GameSingleton::get_province_index_from_uv_coords, { "coords" });

	OV_BIND_METHOD(GameSingleton::get_map_width);
	OV_BIND_METHOD(GameSingleton::get_map_height);
	OV_BIND_METHOD(GameSingleton::get_map_dims);
	OV_BIND_METHOD(GameSingleton::get_map_aspect_ratio);
	OV_BIND_METHOD(GameSingleton::get_bookmark_start_position);
	OV_BIND_METHOD(GameSingleton::get_terrain_texture);
	OV_BIND_METHOD(GameSingleton::get_flag_dims);
	OV_BIND_METHOD(GameSingleton::get_flag_sheet_texture);
	OV_BIND_METHOD(GameSingleton::get_province_shape_image_subdivisions);
	OV_BIND_METHOD(GameSingleton::get_province_shape_texture);
	OV_BIND_METHOD(GameSingleton::get_province_colour_texture);

	OV_BIND_METHOD(GameSingleton::get_province_names);

	OV_BIND_METHOD(GameSingleton::get_mapmode_count);
	OV_BIND_METHOD(GameSingleton::get_mapmode_identifier, { "index" });
	OV_BIND_METHOD(GameSingleton::get_mapmode_localisation_key, { "index" });
	OV_BIND_METHOD(GameSingleton::get_current_mapmode_index);
	OV_BIND_METHOD(GameSingleton::set_mapmode, { "index" });
	OV_BIND_METHOD(GameSingleton::is_parchment_mapmode_allowed);

	OV_BIND_METHOD(GameSingleton::update_clock);

	ADD_SIGNAL(MethodInfo(_signal_gamestate_updated()));
	ADD_SIGNAL(MethodInfo(_signal_mapmode_changed(), PropertyInfo(Variant::INT, "index")));
}

GameSingleton* GameSingleton::get_singleton() {
	return singleton;
}

void GameSingleton::_on_gamestate_updated() {
	_update_colour_image();
	emit_signal(_signal_gamestate_updated());
	gamestate_updated();
}

/* REQUIREMENTS:
 * MAP-21, MAP-23, MAP-25, MAP-32, MAP-33, MAP-34
 */
GameSingleton::GameSingleton()
	: game_manager { std::bind(&GameSingleton::_on_gamestate_updated, this) }, mapmode { &Mapmode::ERROR_MAPMODE } {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

GameSingleton::~GameSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

void GameSingleton::setup_logger() {
	Logger::set_info_func([](std::string&& str) {
		UtilityFunctions::print(Utilities::std_to_godot_string(str));
	});
	Logger::set_warning_func([](std::string&& str) {
		UtilityFunctions::push_warning(Utilities::std_to_godot_string(str));
	});
	Logger::set_error_func([](std::string&& str) {
		UtilityFunctions::push_error(Utilities::std_to_godot_string(str));
	});
}

TypedArray<Dictionary> GameSingleton::get_bookmark_info() const {
	static const StringName bookmark_info_name_key = "bookmark_name";
	static const StringName bookmark_info_date_key = "bookmark_date";

	TypedArray<Dictionary> results;

	BookmarkManager const& bookmark_manager =
		game_manager.get_definition_manager().get_history_manager().get_bookmark_manager();

	for (Bookmark const& bookmark : bookmark_manager.get_bookmarks()) {
		Dictionary bookmark_info;

		bookmark_info[bookmark_info_name_key] = Utilities::std_to_godot_string(bookmark.get_name());
		bookmark_info[bookmark_info_date_key] = Utilities::date_to_formatted_string(bookmark.get_date(), false);

		results.push_back(std::move(bookmark_info));
	}

	return results;
}

Error GameSingleton::setup_game(int32_t bookmark_index) {
	Bookmark const* bookmark =
		game_manager.get_definition_manager().get_history_manager().get_bookmark_manager().get_bookmark_by_index(bookmark_index
		);
	ERR_FAIL_NULL_V_MSG(bookmark, FAILED, vformat("Failed to get bookmark with index: %d", bookmark_index));
	bool ret = game_manager.setup_instance(bookmark);

	// TODO - remove this temporary crime assignment
	InstanceManager* instance_manager = get_instance_manager();
	ERR_FAIL_NULL_V_MSG(instance_manager, FAILED, "Failed to setup instance manager!");
	for (ProvinceInstance& province : instance_manager->get_map_instance().get_province_instances()) {
		province.set_crime(get_definition_manager().get_crime_manager().get_crime_modifier_by_index(
			(province.get_index() - 1) % get_definition_manager().get_crime_manager().get_crime_modifier_count()
		));
	}

	ret &= MenuSingleton::get_singleton()->_population_menu_update_provinces() == OK;

	// TODO - replace with actual starting country
	CountryInstance* starting_country =
		instance_manager->get_country_instance_manager().get_country_instance_by_identifier("ENG");

	PlayerSingleton& player_singleton = *PlayerSingleton::get_singleton();
	player_singleton.set_player_country(starting_country);
	ERR_FAIL_NULL_V(player_singleton.get_player_country(), FAILED);

	// TODO - remove this test starting research
	for (Technology const& technology :
		 get_definition_manager().get_research_manager().get_technology_manager().get_technologies()) {
		if (starting_country->can_research_tech(technology, instance_manager->get_today())) {
			starting_country->start_research(technology, *instance_manager);
			break;
		}
	}

	return ERR(ret);
}

Error GameSingleton::start_game_session() {
	return ERR(game_manager.start_game_session());
}

int32_t GameSingleton::get_province_index_from_uv_coords(Vector2 const& coords) const {
	const Vector2 pos = coords.posmod(1.0f) * get_map_dims();
	return get_definition_manager().get_map_definition().get_province_index_at(Utilities::from_godot_ivec2(pos));
}

int32_t GameSingleton::get_map_width() const {
	return get_definition_manager().get_map_definition().get_width();
}

int32_t GameSingleton::get_map_height() const {
	return get_definition_manager().get_map_definition().get_height();
}

Vector2i GameSingleton::get_map_dims() const {
	return Utilities::to_godot_ivec2(get_definition_manager().get_map_definition().get_dims());
}

float GameSingleton::get_map_aspect_ratio() const {
	return static_cast<float>(get_map_width()) / static_cast<float>(get_map_height());
}

Vector2 GameSingleton::normalise_map_position(fvec2_t const& position) const {
	return Utilities::to_godot_fvec2(position) / get_map_dims();
}

Vector2 GameSingleton::get_billboard_pos(ProvinceDefinition const& province) const {
	return normalise_map_position(province.get_city_position());
}

Vector2 GameSingleton::get_bookmark_start_position() const {
	InstanceManager const* instance_manager = get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	// TODO - What if game started from save rather than bookmark? Does a save game store which bookmark it originated from?

	Bookmark const* bookmark = instance_manager->get_bookmark();
	ERR_FAIL_NULL_V(bookmark, {});

	return normalise_map_position(bookmark->get_initial_camera_position());
}

Ref<Texture2DArray> GameSingleton::get_terrain_texture() const {
	return terrain_texture;
}

Ref<Image> GameSingleton::get_flag_sheet_image() const {
	return flag_sheet_image;
}

Ref<ImageTexture> GameSingleton::get_flag_sheet_texture() const {
	return flag_sheet_texture;
}

int32_t GameSingleton::get_flag_sheet_index(int32_t country_index, StringName const& flag_type) const {
	ERR_FAIL_COND_V_MSG(
		country_index < 0 ||
			country_index >= get_definition_manager().get_country_definition_manager().get_country_definition_count(),
		-1, vformat("Invalid country index: %d", country_index)
	);

	const typename decltype(flag_type_index_map)::const_iterator it = flag_type_index_map.find(flag_type);
	ERR_FAIL_COND_V_MSG(it == flag_type_index_map.end(), -1, vformat("Invalid flag type %s", flag_type));

	return flag_type_index_map.size() * country_index + it->second;
}

Rect2i GameSingleton::get_flag_sheet_rect(int32_t flag_index) const {
	ERR_FAIL_COND_V_MSG(
		flag_index < 0 || flag_index >= flag_sheet_count, {}, vformat("Invalid flag sheet index: %d", flag_index)
	);

	return { Vector2i { flag_index % flag_sheet_dims.x, flag_index / flag_sheet_dims.x } * flag_dims, flag_dims };
}

Rect2i GameSingleton::get_flag_sheet_rect(int32_t country_index, StringName const& flag_type) const {
	return get_flag_sheet_rect(get_flag_sheet_index(country_index, flag_type));
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
	MapDefinition const& map_definition = get_definition_manager().get_map_definition();
	ERR_FAIL_COND_V_MSG(
		!map_definition.province_definitions_are_locked(), FAILED,
		"Cannot generate province colour image before provinces are locked!"
	);

	/* We reshape the list of colours into a square, as each texture dimensions cannot exceed 16384. */
	static constexpr int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(ProvinceDefinition::index_t) * CHAR_BIT / 2);
	static constexpr int32_t colour_image_width = PROVINCE_INDEX_SQRT * sizeof(Mapmode::base_stripe_t) / sizeof(colour_argb_t);
	/* Province count + null province, rounded up to next multiple of PROVINCE_INDEX_SQRT.
	 * Rearranged from: (map_definition.get_province_definition_count() + 1) + (PROVINCE_INDEX_SQRT - 1) */
	const int32_t colour_image_height =
		(map_definition.get_province_definition_count() + PROVINCE_INDEX_SQRT) / PROVINCE_INDEX_SQRT;

	static PackedByteArray colour_data_array;
	const int64_t colour_data_array_size = colour_image_width * colour_image_height * sizeof(colour_argb_t);
	ERR_FAIL_COND_V(colour_data_array.resize(colour_data_array_size) != OK, FAILED);

	Error err = OK;

	InstanceManager const* instance_manager = get_instance_manager();
	PlayerSingleton const& player_singleton = *PlayerSingleton::get_singleton();
	if (instance_manager != nullptr &&
		!get_definition_manager().get_mapmode_manager().generate_mapmode_colours(
			instance_manager->get_map_instance(), mapmode, player_singleton.get_player_country(),
			player_singleton.get_selected_province(), colour_data_array.ptrw()
		)) {
		err = FAILED;
	}

	if (province_colour_image.is_null()) {
		province_colour_image.instantiate();
		ERR_FAIL_NULL_V_EDMSG(province_colour_image, FAILED, "Failed to create province colour image");
	}
	/* Width is doubled as each province has a (base, stripe) colour pair. */
	province_colour_image->set_data(colour_image_width, colour_image_height, false, Image::FORMAT_RGBA8, colour_data_array);
	if (province_colour_texture.is_null()) {
		province_colour_texture = ImageTexture::create_from_image(province_colour_image);
		ERR_FAIL_NULL_V_EDMSG(province_colour_texture, FAILED, "Failed to create province colour texture");
	} else {
		province_colour_texture->update(province_colour_image);
	}
	return err;
}

TypedArray<Dictionary> GameSingleton::get_province_names() const {
	static const StringName identifier_key = "identifier";
	static const StringName position_key = "position";
	static const StringName rotation_key = "rotation";
	static const StringName scale_key = "scale";

	MapDefinition const& map_definition = get_definition_manager().get_map_definition();

	TypedArray<Dictionary> ret;
	ERR_FAIL_COND_V(ret.resize(map_definition.get_province_definition_count()) != OK, {});

	for (int32_t index = 0; index < map_definition.get_province_definition_count(); ++index) {
		ProvinceDefinition const& province = map_definition.get_province_definitions()[index];

		Dictionary province_dict;

		province_dict[identifier_key] = Utilities::std_to_godot_string(province.get_identifier());
		province_dict[position_key] = normalise_map_position(province.get_text_position());

		const float rotation = province.get_text_rotation().to_float();
		if (rotation != 0.0f) {
			province_dict[rotation_key] = rotation;
		}

		const float scale = province.get_text_scale().to_float();
		if (scale != 1.0f) {
			province_dict[scale_key] = scale;
		}

		ret[index] = std::move(province_dict);
	}

	return ret;
}

int32_t GameSingleton::get_mapmode_count() const {
	return get_definition_manager().get_mapmode_manager().get_mapmode_count();
}

String GameSingleton::get_mapmode_identifier(int32_t index) const {
	Mapmode const* identifier_mapmode = get_definition_manager().get_mapmode_manager().get_mapmode_by_index(index);
	if (identifier_mapmode != nullptr) {
		return Utilities::std_to_godot_string(identifier_mapmode->get_identifier());
	}
	return String {};
}

String GameSingleton::get_mapmode_localisation_key(int32_t index) const {
	Mapmode const* localisation_key_mapmode = get_definition_manager().get_mapmode_manager().get_mapmode_by_index(index);
	if (localisation_key_mapmode != nullptr) {
		return Utilities::std_to_godot_string(localisation_key_mapmode->get_localisation_key());
	}
	return String {};
}

int32_t GameSingleton::get_current_mapmode_index() const {
	return mapmode->get_index();
}

Error GameSingleton::set_mapmode(int32_t index) {
	Mapmode const* new_mapmode = get_definition_manager().get_mapmode_manager().get_mapmode_by_index(index);
	ERR_FAIL_NULL_V_MSG(new_mapmode, FAILED, vformat("Failed to find mapmode with index: %d", index));
	mapmode = new_mapmode;
	const Error err = _update_colour_image();
	emit_signal(_signal_mapmode_changed(), mapmode->get_index());
	return err;
}

bool GameSingleton::is_parchment_mapmode_allowed() const {
	/* Disallows parchment mapmode, e.g. for the cosmetic terrain mapmode */
	return mapmode->is_parchment_mapmode_allowed();
}

Error GameSingleton::update_clock() {
	return ERR(game_manager.update_clock());
}

Error GameSingleton::_load_map_images() {
	ERR_FAIL_COND_V_MSG(province_shape_texture.is_valid(), FAILED, "Map images have already been loaded!");

	Error err = OK;

	const Vector2i map_dims = get_map_dims();

	// For each dimension of the image, this finds the small number of equal subdivisions
	// required get the individual texture dims under GPU_DIM_LIMIT
	for (int i = 0; i < 2; ++i) {
		image_subdivisions[i] = 1;
		while (map_dims[i] / image_subdivisions[i] > GPU_DIM_LIMIT || map_dims[i] % image_subdivisions[i] != 0) {
			++image_subdivisions[i];
		}
	}

	MapDefinition::shape_pixel_t const* province_shape_data =
		get_definition_manager().get_map_definition().get_province_shape_image().data();

	const Vector2i divided_dims = map_dims / image_subdivisions;
	const int64_t subdivision_width = divided_dims.x * sizeof(MapDefinition::shape_pixel_t);
	const int64_t subdivision_size = subdivision_width * divided_dims.y;

	TypedArray<Image> province_shape_images;
	ERR_FAIL_COND_V(province_shape_images.resize(image_subdivisions.x * image_subdivisions.y) != OK, FAILED);

	PackedByteArray index_data_array;
	ERR_FAIL_COND_V(index_data_array.resize(subdivision_size) != OK, FAILED);

	for (int32_t v = 0; v < image_subdivisions.y; ++v) {
		for (int32_t u = 0; u < image_subdivisions.x; ++u) {

			for (int32_t y = 0; y < divided_dims.y; ++y) {
				memcpy(
					index_data_array.ptrw() + y * subdivision_width,
					province_shape_data + (v * divided_dims.y + y) * map_dims.x + u * divided_dims.x, subdivision_width
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
	Ref<Image> terrain_sheet = asset_manager->get_image(terrain_texturesheet_path, AssetManager::LOAD_FLAG_NONE);
	ERR_FAIL_NULL_V_MSG(terrain_sheet, FAILED, vformat("Failed to load terrain texture sheet: %s", terrain_texturesheet_path));

	static constexpr int32_t SHEET_DIMS = 8, SHEET_SIZE = SHEET_DIMS * SHEET_DIMS;

	const int32_t sheet_width = terrain_sheet->get_width(), sheet_height = terrain_sheet->get_height();
	ERR_FAIL_COND_V_MSG(
		sheet_width < 1 || sheet_width % SHEET_DIMS != 0 || sheet_width != sheet_height, FAILED,
		vformat(
			"Invalid terrain texture sheet dims: %dx%d (must be square with dims positive multiples of %d)", sheet_width,
			sheet_height, SHEET_DIMS
		)
	);
	const int32_t slice_size = sheet_width / SHEET_DIMS;

	TypedArray<Image> terrain_images;
	ERR_FAIL_COND_V(terrain_images.resize(SHEET_SIZE + 1) != OK, FAILED);
	{
		/* This is a placeholder image so that we don't have to branch to avoid looking up terrain index 0 (water).
		 * It should never appear in game, and so is bright red to to make it obvious if it slips through. */
		const Ref<Image> water_image =
			Utilities::make_solid_colour_image({ 1.0f, 0.0f, 0.0f }, slice_size, slice_size, terrain_sheet->get_format());
		ERR_FAIL_NULL_V_EDMSG(water_image, FAILED, "Failed to create water terrain image");
		terrain_images[0] = water_image;
	}

	for (int32_t idx = 0; idx < SHEET_SIZE; ++idx) {
		const Rect2i slice { idx % SHEET_DIMS * slice_size, idx / SHEET_DIMS * slice_size, slice_size, slice_size };
		const Ref<Image> terrain_image = terrain_sheet->get_region(slice);

		ERR_FAIL_COND_V_MSG(
			terrain_image.is_null() || terrain_image->is_empty(), FAILED,
			vformat("Failed to extract terrain texture slice %s from %s", slice, terrain_texturesheet_path)
		);

		terrain_images[idx + 1] = terrain_image;
	}

	terrain_texture.instantiate();
	ERR_FAIL_COND_V_MSG(
		terrain_texture->create_from_images(terrain_images) != OK, FAILED, "Failed to create terrain texture array!"
	);
	return OK;
}

Error GameSingleton::_load_flag_sheet() {
	ERR_FAIL_COND_V_MSG(
		flag_sheet_image.is_valid() || flag_sheet_texture.is_valid(), FAILED,
		"Flag sheet image and/or texture has already been generated!"
	);

	GovernmentTypeManager const& government_type_manager =
		get_definition_manager().get_politics_manager().get_government_type_manager();
	ERR_FAIL_COND_V_MSG(
		government_type_manager.get_flag_types().empty() || !government_type_manager.government_types_are_locked(), FAILED,
		"Cannot load flag images if flag types are empty or government types are not locked!"
	);
	CountryDefinitionManager const& country_definition_manager = get_definition_manager().get_country_definition_manager();
	ERR_FAIL_COND_V_MSG(
		country_definition_manager.country_definitions_empty() || !country_definition_manager.country_definitions_are_locked(),
		FAILED, "Cannot load flag images if countries are empty or not locked!"
	);

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);

	/* Generate flag type - index lookup map */
	flag_type_index_map.clear();
	for (std::string const& type : government_type_manager.get_flag_types()) {
		flag_type_index_map.emplace(Utilities::std_to_godot_string(type), static_cast<int32_t>(flag_type_index_map.size()));
	}

	flag_sheet_count = country_definition_manager.get_country_definition_count() * flag_type_index_map.size();

	std::vector<Ref<Image>> flag_images;
	flag_images.reserve(flag_sheet_count);

	static constexpr Image::Format flag_format = Image::FORMAT_RGB8;

	Error ret = OK;
	for (CountryDefinition const& country : country_definition_manager.get_country_definitions()) {
		const String country_name = Utilities::std_to_godot_string(country.get_identifier());

		for (auto const& [flag_type, flag_type_index] : flag_type_index_map) {
			static const String flag_directory = "gfx/flags/";
			static const String flag_separator = "_";
			static const String flag_extension = ".tga";

			const StringName flag_path =
				flag_directory + country_name + (flag_type.is_empty() ? "" : flag_separator + flag_type) + flag_extension;

			/* Do not cache flag image, they should be freed after the flag sheet has been generated. */
			const Ref<Image> flag_image = asset_manager->get_image(flag_path, AssetManager::LOAD_FLAG_NONE);

			if (flag_image.is_valid()) {

				if (flag_image->get_format() != flag_format) {
					flag_image->convert(flag_format);
				}

				if (flag_image->get_size() != flag_dims) {
					if (flag_image->get_width() > flag_dims.x || flag_image->get_height() > flag_dims.y) {
						UtilityFunctions::push_warning(
							"Flag image ", flag_path, " (", flag_image->get_size(), ") is larger than the sheet flag size (",
							flag_dims, ")"
						);
					}

					flag_image->resize(flag_dims.x, flag_dims.y, Image::INTERPOLATE_NEAREST);
				}
			} else {
				UtilityFunctions::push_error("Failed to load flag image: ", flag_path);
				ret = FAILED;
			}

			/* Add flag_image to the vector even if it's null to ensure each flag has the right index. */
			flag_images.push_back(flag_image);
		}
	}

	ERR_FAIL_COND_V(flag_images.size() != flag_sheet_count, FAILED);

	/* Calculate the width that will make the sheet as close to a square as possible (taking flag dimensions into account.) */
	flag_sheet_dims.x = (fixed_point_t { static_cast<int32_t>(flag_images.size()) } * flag_dims.y / flag_dims.x).sqrt().ceil();

	/* Calculated corresponding height (rounded up). */
	flag_sheet_dims.y = (static_cast<int32_t>(flag_images.size()) + flag_sheet_dims.x - 1) / flag_sheet_dims.x;

	const Vector2i sheet_dims = flag_sheet_dims * flag_dims;

	flag_sheet_image = Image::create(sheet_dims.x, sheet_dims.y, false, flag_format);
	ERR_FAIL_NULL_V_MSG(flag_sheet_image, FAILED, "Failed to create flag sheet image!");

	static const Rect2i flag_rect { { 0, 0 }, flag_dims };

	/* Fill the flag sheet with the flag images. */
	for (int32_t index = 0; index < flag_images.size(); ++index) {
		Ref<Image> const& flag_image = flag_images[index];

		const Vector2i sheet_pos = Vector2i { index % flag_sheet_dims.x, index / flag_sheet_dims.x } * flag_dims;

		if (flag_image.is_valid()) {
			flag_sheet_image->blit_rect(flag_image, flag_rect, sheet_pos);
		} else {
			static const Color error_colour { 1.0f, 0.0f, 1.0f, 1.0f }; /* Magenta */

			flag_sheet_image->fill_rect({ sheet_pos, flag_dims }, error_colour);
		}
	}

	flag_sheet_texture = ImageTexture::create_from_image(flag_sheet_image);
	ERR_FAIL_NULL_V_MSG(flag_sheet_texture, FAILED, "Failed to create flag sheet texture!");

	return ret;
}

Error GameSingleton::set_compatibility_mode_roots(
	PackedStringArray const& file_paths, godot::PackedStringArray const& replace_paths
) {
	Dataloader::path_vector_t roots;
	roots.reserve(file_paths.size());
	for (String const& path : file_paths) {
		roots.emplace_back(Utilities::godot_to_std_string(path));
	}

	Dataloader::path_vector_t replace;
	replace.reserve(replace_paths.size());
	for (String const& path : replace_paths) {
		replace.emplace_back(Utilities::godot_to_std_string(path));
	}

	ERR_FAIL_COND_V_MSG(!game_manager.set_roots(roots, replace), FAILED, "Failed to set dataloader roots!");
	return OK;
}

Error GameSingleton::load_defines_compatibility_mode() {
	Error err = OK;
	auto add_message = std::bind_front(&LoadLocalisation::add_message, LoadLocalisation::get_singleton());

	if (!game_manager.load_definitions(add_message)) {
		UtilityFunctions::push_error("Failed to load defines!");
		err = FAILED;
	}

	if (_load_terrain_variants() != OK) {
		UtilityFunctions::push_error("Failed to load terrain variants!");
		err = FAILED;
	}
	if (_load_flag_sheet() != OK) {
		UtilityFunctions::push_error("Failed to load flag sheet!");
		err = FAILED;
	}
	if (_load_map_images() != OK) {
		UtilityFunctions::push_error("Failed to load map images!");
		err = FAILED;
	}

	AssetManager* asset_manager = AssetManager::get_singleton();
	if (asset_manager == nullptr || asset_manager->preload_textures() != OK) {
		UtilityFunctions::push_error("Failed to preload assets!");
		err = FAILED;
	}

	return err;
}

String GameSingleton::search_for_game_path(String const& hint_path) {
	return Utilities::std_to_godot_string(Dataloader::search_for_game_path(Utilities::godot_to_std_string(hint_path)).string());
}

String GameSingleton::lookup_file_path(String const& path) const {
	return Utilities::std_to_godot_string(get_dataloader().lookup_file(Utilities::godot_to_std_string(path)).string());
}
