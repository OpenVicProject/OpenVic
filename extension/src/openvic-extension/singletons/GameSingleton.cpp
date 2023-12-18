#include "GameSingleton.hpp"

#include <functional>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/utility/Logger.hpp>

#include "openvic-extension/singletons/LoadLocalisation.hpp"
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

void GameSingleton::_bind_methods() {
	OV_BIND_SMETHOD(setup_logger);

	OV_BIND_METHOD(GameSingleton::load_defines_compatibility_mode, { "file_paths" });
	OV_BIND_SMETHOD(search_for_game_path, { "hint_path" }, DEFVAL(String {}));

	OV_BIND_METHOD(GameSingleton::setup_game, { "bookmark_index" });

	OV_BIND_METHOD(GameSingleton::get_province_index_from_uv_coords, { "coords" });
	OV_BIND_METHOD(GameSingleton::get_province_info_from_index, { "index" });

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
	OV_BIND_METHOD(GameSingleton::get_selected_province_index);
	OV_BIND_METHOD(GameSingleton::set_selected_province, { "index" });

	OV_BIND_METHOD(GameSingleton::expand_building, { "province_index", "building_type_identifier" });
	OV_BIND_METHOD(GameSingleton::get_slave_pop_icon_index);
	OV_BIND_METHOD(GameSingleton::get_administrative_pop_icon_index);
	OV_BIND_METHOD(GameSingleton::get_rgo_owner_pop_icon_index);
	OV_BIND_SMETHOD(int_to_formatted_string, { "val" });
	OV_BIND_SMETHOD(float_to_formatted_string, { "val" });

	OV_BIND_METHOD(GameSingleton::set_paused, { "paused" });
	OV_BIND_METHOD(GameSingleton::toggle_paused);
	OV_BIND_METHOD(GameSingleton::is_paused);
	OV_BIND_METHOD(GameSingleton::increase_speed);
	OV_BIND_METHOD(GameSingleton::decrease_speed);
	OV_BIND_METHOD(GameSingleton::get_speed);
	OV_BIND_METHOD(GameSingleton::can_increase_speed);
	OV_BIND_METHOD(GameSingleton::can_decrease_speed);
	OV_BIND_METHOD(GameSingleton::get_longform_date);
	OV_BIND_METHOD(GameSingleton::try_tick);

	ADD_SIGNAL(MethodInfo("state_updated"));
	ADD_SIGNAL(MethodInfo("province_selected", PropertyInfo(Variant::INT, "index")));
}

GameSingleton* GameSingleton::get_singleton() {
	return singleton;
}

void GameSingleton::_on_state_updated() {
	_update_colour_image();
	emit_signal("state_updated");
}

/* REQUIREMENTS:
 * MAP-21, MAP-23, MAP-25, MAP-32, MAP-33, MAP-34
 */
GameSingleton::GameSingleton()
	: game_manager { std::bind(&GameSingleton::_on_state_updated, this) } {
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

GameManager const& GameSingleton::get_game_manager() const {
	return game_manager;
}

Dataloader const& GameSingleton::get_dataloader() const {
	return dataloader;
}

Error GameSingleton::setup_game(int32_t bookmark_index) {
	Bookmark const* bookmark = game_manager.get_history_manager().get_bookmark_manager().get_bookmark_by_index(bookmark_index);
	if (bookmark == nullptr) {
		UtilityFunctions::push_error("Failed to get bookmark with index: ", bookmark_index);
		return FAILED;
	}
	bool ret = game_manager.load_bookmark(bookmark);
	for (Province& province : game_manager.get_map().get_provinces()) {
		province.set_crime(
			game_manager.get_crime_manager().get_crime_modifier_by_index(
				(province.get_index() - 1) % game_manager.get_crime_manager().get_crime_modifier_count()
			)
		);
	}
	return ERR(ret);
}

int32_t GameSingleton::get_province_index_from_uv_coords(Vector2 const& coords) const {
	const size_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * get_map_width();
	const size_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * get_map_height();
	return game_manager.get_map().get_province_index_at(x_mod_w, y_mod_h);
}

template<std::derived_from<HasIdentifierAndColour> T>
static Array _distribution_to_pie_chart_array(fixed_point_map_t<T const*> const& dist) {
	using entry_t = std::pair<T const*, fixed_point_t>;
	std::vector<entry_t> sorted_dist;
	sorted_dist.reserve(dist.size());
	for (entry_t const& entry : dist) {
		if (entry.first != nullptr) {
			sorted_dist.push_back(entry);
		} else {
			UtilityFunctions::push_error("Null distribution key with value ", entry.second.to_float());
		}
	}
	std::sort(sorted_dist.begin(), sorted_dist.end(), [](entry_t const& lhs, entry_t const& rhs) -> bool {
		return lhs.second < rhs.second;
	});
	static const StringName identifier_key = "identifier";
	static const StringName colour_key = "colour";
	static const StringName weight_key = "weight";
	Array array;
	for (auto const& [key, val] : sorted_dist) {
		Dictionary sub_dict;
		sub_dict[identifier_key] = std_view_to_godot_string(key->get_identifier());
		sub_dict[colour_key] = Utilities::to_godot_color(key->get_colour());
		sub_dict[weight_key] = val.to_float();
		array.push_back(sub_dict);
	}
	return array;
}

Dictionary GameSingleton::get_province_info_from_index(int32_t index) const {
	static const StringName province_info_province_key = "province";
	static const StringName province_info_region_key = "region";
	static const StringName province_info_controller_key = "controller";
	static const StringName province_info_life_rating_key = "life_rating";
	static const StringName province_info_terrain_type_key = "terrain_type";
	static const StringName province_info_crime_name_key = "crime_name";
	static const StringName province_info_crime_icon_key = "crime_icon";
	static const StringName province_info_total_population_key = "total_population";
	static const StringName province_info_pop_types_key = "pop_types";
	static const StringName province_info_pop_ideologies_key = "pop_ideologies";
	static const StringName province_info_pop_cultures_key = "pop_cultures";
	static const StringName province_info_rgo_name_key = "rgo_name";
	static const StringName province_info_rgo_icon_key = "rgo_icon";
	static const StringName province_info_colony_status_key = "colony_status";
	static const StringName province_info_slave_status_key = "slave_status";
	static const StringName province_info_buildings_key = "buildings";

	Province const* province = game_manager.get_map().get_province_by_index(index);
	if (province == nullptr) {
		return {};
	}
	Dictionary ret;

	ret[province_info_province_key] = std_view_to_godot_string(province->get_identifier());

	Region const* region = province->get_region();
	if (region != nullptr) {
		ret[province_info_region_key] = std_view_to_godot_string(region->get_identifier());
	}

	Country const* controller = province->get_controller();
	if (controller != nullptr) {
		ret[province_info_controller_key] = std_view_to_godot_string(controller->get_identifier());
	}

	Good const* rgo = province->get_rgo();
	if (rgo != nullptr) {
		ret[province_info_rgo_name_key] = std_view_to_godot_string(rgo->get_identifier());
		ret[province_info_rgo_icon_key] = static_cast<int32_t>(rgo->get_index());
	}

	ret[province_info_life_rating_key] = province->get_life_rating();

	TerrainType const* terrain_type = province->get_terrain_type();
	if (terrain_type != nullptr) {
		ret[province_info_terrain_type_key] = std_view_to_godot_string(terrain_type->get_identifier());
	}

	Crime const* crime = province->get_crime();
	if (crime != nullptr) {
		ret[province_info_crime_name_key] = std_view_to_godot_string(crime->get_identifier());
		ret[province_info_crime_icon_key] = static_cast<int32_t>(crime->get_icon());
	}

	ret[province_info_colony_status_key] = static_cast<int32_t>(province->get_colony_status());
	ret[province_info_slave_status_key] = province->get_slave();

	ret[province_info_total_population_key] = province->get_total_population();
	fixed_point_map_t<PopType const*> const& pop_types = province->get_pop_type_distribution();
	if (!pop_types.empty()) {
		ret[province_info_pop_types_key] = _distribution_to_pie_chart_array(pop_types);
	}
	fixed_point_map_t<Ideology const*> const& ideologies = province->get_ideology_distribution();
	if (!ideologies.empty()) {
		ret[province_info_pop_ideologies_key] = _distribution_to_pie_chart_array(ideologies);
	}
	fixed_point_map_t<Culture const*> const& cultures = province->get_culture_distribution();
	if (!cultures.empty()) {
		ret[province_info_pop_cultures_key] = _distribution_to_pie_chart_array(cultures);
	}

	static const StringName building_info_building_key = "building";
	static const StringName building_info_level_key = "level";
	static const StringName building_info_expansion_state_key = "expansion_state";
	static const StringName building_info_start_date_key = "start_date";
	static const StringName building_info_end_date_key = "end_date";
	static const StringName building_info_expansion_progress_key = "expansion_progress";

	std::vector<BuildingInstance> const& buildings = province->get_buildings();
	if (!buildings.empty()) {
		Array buildings_array;
		buildings_array.resize(buildings.size());
		for (size_t idx = 0; idx < buildings.size(); ++idx) {
			BuildingInstance const& building = buildings[idx];

			Dictionary building_dict;
			building_dict[building_info_building_key] = std_view_to_godot_string(building.get_identifier());
			building_dict[building_info_level_key] = static_cast<int32_t>(building.get_level());
			building_dict[building_info_expansion_state_key] = static_cast<int32_t>(building.get_expansion_state());
			building_dict[building_info_start_date_key] = std_to_godot_string(building.get_start_date().to_string());
			building_dict[building_info_end_date_key] = std_to_godot_string(building.get_end_date().to_string());
			building_dict[building_info_expansion_progress_key] = building.get_expansion_progress();

			buildings_array[idx] = building_dict;
		}
		ret[province_info_buildings_key] = buildings_array;
	}
	return ret;
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

Ref<Texture> GameSingleton::get_terrain_texture() const {
	return terrain_texture;
}

Ref<Image> GameSingleton::get_flag_image(Country const* country, StringName const& flag_type) const {
	if (country != nullptr) {
		const typename decltype(flag_image_map)::const_iterator it = flag_image_map.find(country);
		if (it != flag_image_map.end()) {
			const typename decltype(it->second)::const_iterator it2 = it->second.find(flag_type);
			if (it2 != it->second.end()) {
				return it2->second;
			} else {
				UtilityFunctions::push_error(
					"Failed to find ", flag_type, " flag for country: ", std_view_to_godot_string(country->get_identifier())
				);
			}
		} else {
			UtilityFunctions::push_error(
				"Failed to find flags for country: ", std_view_to_godot_string(country->get_identifier())
			);
		}
	}
	return nullptr;
}

Vector2i GameSingleton::get_province_shape_image_subdivisions() const {
	return image_subdivisions;
}

Ref<Texture> GameSingleton::get_province_shape_texture() const {
	return province_shape_texture;
}

Ref<Texture> GameSingleton::get_province_colour_texture() const {
	return province_colour_texture;
}

Error GameSingleton::_update_colour_image() {
	Map const& map = game_manager.get_map();
	if (!map.provinces_are_locked()) {
		UtilityFunctions::push_error("Cannot generate province colour image before provinces are locked!");
		return FAILED;
	}
	/* We reshape the list of colours into a square, as each texture dimensions cannot exceed 16384. */
	static constexpr int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(Province::index_t) * 4);
	static constexpr int32_t colour_image_width = PROVINCE_INDEX_SQRT * sizeof(Mapmode::base_stripe_t) / sizeof(colour_t);
	/* Province count + null province, rounded up to next multiple of PROVINCE_INDEX_SQRT.
	 * Rearranged from: (map.get_province_count() + 1) + (PROVINCE_INDEX_SQRT - 1) */
	static const int32_t colour_image_height = (map.get_province_count() + PROVINCE_INDEX_SQRT) / PROVINCE_INDEX_SQRT;

	static PackedByteArray colour_data_array;
	static const int64_t colour_data_array_size = colour_image_width * colour_image_height * sizeof(colour_t);
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
	if (mapmode == nullptr) {
		UtilityFunctions::push_error("Failed to set mapmode to: ", identifier);
		return FAILED;
	}
	mapmode_index = mapmode->get_index();
	_update_colour_image();
	return OK;
}

int32_t GameSingleton::get_selected_province_index() const {
	return game_manager.get_map().get_selected_province_index();
}

void GameSingleton::set_selected_province(int32_t index) {
	game_manager.get_map().set_selected_province(index);
	_update_colour_image();
	emit_signal("province_selected", index);
}

Error GameSingleton::expand_building(int32_t province_index, String const& building_type_identifier) {
	if (!game_manager.expand_building(province_index, godot_to_std_string(building_type_identifier))) {
		UtilityFunctions::push_error("Failed to expand ", building_type_identifier, " at province index ", province_index);
		return FAILED;
	}
	return OK;
}

int32_t GameSingleton::get_slave_pop_icon_index() const {
	const PopType::sprite_t sprite = game_manager.get_pop_manager().get_slave_sprite();
	if (sprite <= 0) {
		UtilityFunctions::push_error("Slave sprite unset!");
		return 0;
	}
	return sprite;
}

int32_t GameSingleton::get_administrative_pop_icon_index() const {
	const PopType::sprite_t sprite = game_manager.get_pop_manager().get_administrative_sprite();
	if (sprite <= 0) {
		UtilityFunctions::push_error("Administrative sprite unset!");
		return 0;
	}
	return sprite;
}

int32_t GameSingleton::get_rgo_owner_pop_icon_index() const {
	const PopType::sprite_t sprite = game_manager.get_economy_manager().get_production_type_manager().get_rgo_owner_sprite();
	if (sprite <= 0) {
		UtilityFunctions::push_error("RGO owner sprite unset!");
		return 0;
	}
	return sprite;
}

String GameSingleton::int_to_formatted_string(int64_t val) {
	return Utilities::int_to_formatted_string(val);
}

String GameSingleton::float_to_formatted_string(float val) {
	return Utilities::float_to_formatted_string(val);
}

void GameSingleton::set_paused(bool paused) {
	game_manager.get_clock().is_paused = paused;
}

void GameSingleton::toggle_paused() {
	game_manager.get_clock().is_paused = !game_manager.get_clock().is_paused;
}

bool GameSingleton::is_paused() const {
	return game_manager.get_clock().is_paused;
}

void GameSingleton::increase_speed() {
	game_manager.get_clock().increase_simulation_speed();
}

void GameSingleton::decrease_speed() {
	game_manager.get_clock().decrease_simulation_speed();
}

int32_t GameSingleton::get_speed() const {
	return game_manager.get_clock().get_simulation_speed();
}

bool GameSingleton::can_increase_speed() const {
	return game_manager.get_clock().can_increase_simulation_speed();
}

bool GameSingleton::can_decrease_speed() const {
	return game_manager.get_clock().can_decrease_simulation_speed();
}

String GameSingleton::get_longform_date() const {
	return Utilities::date_to_formatted_string(game_manager.get_today());
}

void GameSingleton::try_tick() {
	game_manager.get_clock().conditionally_advance_game();
}

Error GameSingleton::_load_map_images(bool flip_vertical) {
	if (province_shape_texture.is_valid()) {
		UtilityFunctions::push_error("Map images have already been loaded!");
		return FAILED;
	}

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
	Array province_shape_images;
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
	if (terrain_texture.is_valid()) {
		UtilityFunctions::push_error("Terrain variants have already been loaded!");
		return FAILED;
	}

	static const String terrain_texturesheet_path = "map/terrain/texturesheet.tga";

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);
	// Load the terrain texture sheet and prepare to slice it up
	Ref<Image> terrain_sheet = asset_manager->get_image(terrain_texturesheet_path);
	if (terrain_sheet.is_null()) {
		UtilityFunctions::push_error("Failed to load terrain texture sheet: ", terrain_texturesheet_path);
		return FAILED;
	}

	static constexpr int32_t SHEET_DIMS = 8, SHEET_SIZE = SHEET_DIMS * SHEET_DIMS;

	terrain_sheet->flip_y();
	const int32_t sheet_width = terrain_sheet->get_width(), sheet_height = terrain_sheet->get_height();
	if (sheet_width < 1 || sheet_width % SHEET_DIMS != 0 || sheet_width != sheet_height) {
		UtilityFunctions::push_error(
			"Invalid terrain texture sheet dims: ", sheet_width, "x", sheet_height,
			" (must be square with dims positive multiples of ", SHEET_DIMS, ")"
		);
		return FAILED;
	}
	const int32_t slice_size = sheet_width / SHEET_DIMS;

	Array terrain_images;
	{
		static constexpr colour_t TERRAIN_WATER_INDEX_COLOUR = 0xFFFFFF;
		Ref<Image> water_image = Image::create(slice_size, slice_size, false, terrain_sheet->get_format());
		ERR_FAIL_NULL_V_EDMSG(water_image, FAILED, "Failed to create water terrain image");
		water_image->fill({ 0.1f, 0.1f, 0.5f });
		terrain_images.append(water_image);
	}
	Error err = OK;
	for (int32_t idx = 0; idx < SHEET_SIZE; ++idx) {
		const Rect2i slice { (idx % SHEET_DIMS) * slice_size, (7 - (idx / SHEET_DIMS)) * slice_size, slice_size, slice_size };
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
	if (terrain_texture->create_from_images(terrain_images) != OK) {
		UtilityFunctions::push_error("Failed to create terrain texture array!");
		return FAILED;
	}
	return err;
}

Error GameSingleton::_load_flag_images() {
	if (!flag_image_map.empty()) {
		UtilityFunctions::push_error("Flag images have already been loaded!");
		return FAILED;
	}

	GovernmentTypeManager const& government_type_manager = game_manager.get_politics_manager().get_government_type_manager();
	if (!government_type_manager.government_types_are_locked()) {
		UtilityFunctions::push_error("Cannot load flag images before government types are locked!");
		return FAILED;
	}
	CountryManager const& country_manager = game_manager.get_country_manager();
	if (!country_manager.countries_are_locked()) {
		UtilityFunctions::push_error("Cannot load flag images before countries are locked!");
		return FAILED;
	}

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);

	static const String flag_directory = "gfx/flags/";
	static const String flag_separator = "_";
	static const String flag_extension = ".tga";

	std::vector<StringName> flag_types;
	for (std::string const& type : government_type_manager.get_flag_types()) {
		flag_types.emplace_back(std_to_godot_string_name(type));
	}

	Error ret = OK;
	for (Country const& country : country_manager.get_countries()) {
		std::map<StringName, Ref<Image>>& flag_images = flag_image_map[&country];
		const String country_name = std_view_to_godot_string(country.get_identifier());
		for (StringName const& flag_type : flag_types) {
			String flag_path = flag_directory + country_name;
			if (!flag_type.is_empty()) {
				flag_path += flag_separator + flag_type;
			}
			flag_path += flag_extension;
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
	if (_load_map_images(true) != OK) {
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

String GameSingleton::search_for_game_path(String hint_path) {
	return std_to_godot_string(Dataloader::search_for_game_path(godot_to_std_string(hint_path)).string());
}
