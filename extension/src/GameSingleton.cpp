#include "GameSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>

#include "openvic2/Logger.hpp"
#include "openvic2/Types.hpp"

using namespace godot;
using namespace OpenVic2;

#define ERR(x) ((x) == SUCCESS ? OK : FAILED)

GameSingleton* GameSingleton::singleton = nullptr;

void GameSingleton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_province_identifier_file", "file_path"), &GameSingleton::load_province_identifier_file);
	ClassDB::bind_method(D_METHOD("load_water_province_file", "file_path"), &GameSingleton::load_water_province_file);
	ClassDB::bind_method(D_METHOD("load_region_file", "file_path"), &GameSingleton::load_region_file);
	ClassDB::bind_method(D_METHOD("load_province_shape_file", "file_path"), &GameSingleton::load_province_shape_file);
	ClassDB::bind_method(D_METHOD("setup"), &GameSingleton::setup);

	ClassDB::bind_method(D_METHOD("get_province_index_from_uv_coords", "coords"), &GameSingleton::get_province_index_from_uv_coords);
	ClassDB::bind_method(D_METHOD("get_province_info_from_index", "index"), &GameSingleton::get_province_info_from_index);
	ClassDB::bind_method(D_METHOD("get_width"), &GameSingleton::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &GameSingleton::get_height);
	ClassDB::bind_method(D_METHOD("get_province_index_images"), &GameSingleton::get_province_index_images);
	ClassDB::bind_method(D_METHOD("get_province_colour_image"), &GameSingleton::get_province_colour_image);

	ClassDB::bind_method(D_METHOD("update_colour_image"), &GameSingleton::update_colour_image);
	ClassDB::bind_method(D_METHOD("get_mapmode_count"), &GameSingleton::get_mapmode_count);
	ClassDB::bind_method(D_METHOD("get_mapmode_identifier", "index"), &GameSingleton::get_mapmode_identifier);
	ClassDB::bind_method(D_METHOD("set_mapmode", "identifier"), &GameSingleton::set_mapmode);

	ClassDB::bind_method(D_METHOD("expand_building", "province_index", "building_type_identifier"), &GameSingleton::expand_building);

	ClassDB::bind_method(D_METHOD("set_paused", "paused"), &GameSingleton::set_paused);
	ClassDB::bind_method(D_METHOD("toggle_paused"), &GameSingleton::toggle_paused);
	ClassDB::bind_method(D_METHOD("is_paused"), &GameSingleton::is_paused);
	ClassDB::bind_method(D_METHOD("increase_speed"), &GameSingleton::increase_speed);
	ClassDB::bind_method(D_METHOD("decrease_speed"), &GameSingleton::decrease_speed);
	ClassDB::bind_method(D_METHOD("can_increase_speed"), &GameSingleton::can_increase_speed);
	ClassDB::bind_method(D_METHOD("can_decrease_speed"), &GameSingleton::can_decrease_speed);
	ClassDB::bind_method(D_METHOD("get_longform_date"), &GameSingleton::get_longform_date);
	ClassDB::bind_method(D_METHOD("try_tick"), &GameSingleton::try_tick);

	ADD_SIGNAL(MethodInfo("state_updated"));
}

GameSingleton* GameSingleton::get_singleton() {
	return singleton;
}

/* REQUIREMENTS:
 * MAP-21, MAP-25
 */
GameSingleton::GameSingleton() : game_manager{ [this]() { emit_signal("state_updated"); } } {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	Logger::set_info_func([](std::string&& str) { UtilityFunctions::print(str.c_str()); });
	Logger::set_error_func([](std::string&& str) { UtilityFunctions::push_error(str.c_str()); });

	using mapmode_t = std::pair<std::string, Mapmode::colour_func_t>;
	const std::vector<mapmode_t> mapmodes = {
		{ "mapmode_province", [](Map const&, Province const& province) -> colour_t { return province.get_colour(); } },
		{ "mapmode_region", [](Map const&, Province const& province) -> colour_t {
			Region const* region = province.get_region();
			if (region != nullptr) return region->get_colour();
			return province.get_colour();
		} },
		{ "mapmode_terrain", [](Map const&, Province const& province) -> colour_t {
			return province.is_water() ? 0x4287F5 : 0x0D7017;
		} },
		{ "mapmode_index", [](Map const& map, Province const& province) -> colour_t {
			const uint8_t f = static_cast<float>(province.get_index()) / static_cast<float>(map.get_province_count()) * 255.0f;
			return (f << 16) | (f << 8) | f;
		} }
	};
	for (mapmode_t const& mapmode : mapmodes)
		game_manager.map.add_mapmode(mapmode.first, mapmode.second);
	game_manager.map.lock_mapmodes();

	using building_type_t = std::tuple<std::string, Building::level_t, Timespan>;
	const std::vector<building_type_t> building_types = {
		{ "building_fort", 4, 8 }, { "building_naval_base", 6, 15 }, { "building_railroad", 5, 10 }
	};
	for (building_type_t const& type : building_types)
		game_manager.building_manager.add_building_type(std::get<0>(type), std::get<1>(type), std::get<2>(type));
	game_manager.building_manager.lock_building_types();

}

GameSingleton::~GameSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

static Error load_json_file(String const& file_description, String const& file_path, Variant& result) {
	result.clear();
	UtilityFunctions::print("Loading ", file_description, " file: ", file_path);
	const Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load ", file_description, " file: ", file_path);
		return err == OK ? FAILED : err;
	}
	const String json_string = file->get_as_text();
	JSON json;
	err = json.parse(json_string);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to parse ", file_description, " file as JSON: ", file_path,
			"\nError at line ", json.get_error_line(), ": ", json.get_error_message());
		return err;
	}
	result = json.get_data();
	return err;
}

using parse_json_entry_func_t = std::function<godot::Error (godot::String const&, godot::Variant const&)>;

static Error parse_json_dictionary_file(String const& file_description, String const& file_path,
	String const& identifier_prefix, parse_json_entry_func_t parse_entry) {
	Variant json_var;
	Error err = load_json_file(file_description, file_path, json_var);
	if (err != OK) return err;
	const Variant::Type type = json_var.get_type();
	if (type != Variant::DICTIONARY) {
		UtilityFunctions::push_error("Invalid ", file_description, " JSON: root has type ",
			Variant::get_type_name(type), " (expected Dictionary)");
		return FAILED;
	}
	Dictionary const& dict = json_var;
	const Array identifiers = dict.keys();
	for (int64_t idx = 0; idx < identifiers.size(); ++idx) {
		String const& identifier = identifiers[idx];
		Variant const& entry = dict[identifier];
		if (identifier.is_empty()) {
			UtilityFunctions::push_error("Empty identifier in ", file_description, " file with entry: ", entry);
			err = FAILED;
			continue;
		}
		if (!identifier.begins_with(identifier_prefix))
			UtilityFunctions::push_warning("Identifier in ", file_description, " file missing \"", identifier_prefix, "\" prefix: ", identifier);
		if (parse_entry(identifier, entry) != OK) err = FAILED;
	}
	return err;
}

Error GameSingleton::_parse_province_identifier_entry(String const& identifier, Variant const& entry) {
	const Variant::Type type = entry.get_type();
	colour_t colour = NULL_COLOUR;
	if (type == Variant::ARRAY) {
		Array const& colour_array = entry;
		if (colour_array.size() == 3) {
			for (int jdx = 0; jdx < 3; ++jdx) {
				Variant const& var = colour_array[jdx];
				if (var.get_type() != Variant::FLOAT) {
					colour = NULL_COLOUR;
					break;
				}
				const double colour_double = var;
				if (std::trunc(colour_double) != colour_double) {
					colour = NULL_COLOUR;
					break;
				}
				const int64_t colour_int = static_cast<int64_t>(colour_double);
				if (colour_int < 0 || colour_int > 255) {
					colour = NULL_COLOUR;
					break;
				}
				colour = (colour << 8) | colour_int;
			}
		}
	}
	else if (type == Variant::STRING) {
		String const& colour_string = entry;
		if (colour_string.is_valid_hex_number()) {
			const int64_t colour_int = colour_string.hex_to_int();
			if (0 <= colour_int && colour_int <= 0xFFFFFF)
				colour = colour_int;
		}
	}
	if (colour == NULL_COLOUR) {
		UtilityFunctions::push_error("Invalid colour for province identifier \"", identifier, "\": ", entry);
		return FAILED;
	}
	return ERR(game_manager.map.add_province(identifier.utf8().get_data(), colour));
}

Error GameSingleton::load_province_identifier_file(String const& file_path) {
	const Error err = parse_json_dictionary_file("province identifier", file_path, "prov_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return _parse_province_identifier_entry(identifier, entry);
		});
	game_manager.map.lock_provinces();
	return err;
}

Error GameSingleton::_parse_region_entry(String const& identifier, Variant const& entry) {
	Error err = OK;
	Variant::Type type = entry.get_type();
	std::vector<std::string> province_identifiers;
	if (type == Variant::ARRAY) {
		Array const& province_array = entry;
		for (int64_t idx = 0; idx < province_array.size(); ++idx) {
			Variant const& province_var = province_array[idx];
			type = province_var.get_type();
			if (type == Variant::STRING) {
				String const& province_string = province_var;
				province_identifiers.push_back(province_string.utf8().get_data());
			}
			else {
				UtilityFunctions::push_error("Invalid province identifier for region \"", identifier, "\": ", entry);
				err = FAILED;
			}
		}
	}
	if (province_identifiers.empty()) {
		UtilityFunctions::push_error("Invalid province list for region \"", identifier, "\": ", entry);
		return FAILED;
	}
	return ERR(game_manager.map.add_region(identifier.utf8().get_data(), province_identifiers));
}

Error GameSingleton::load_region_file(String const& file_path) {
	const Error err = parse_json_dictionary_file("region", file_path, "region_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return _parse_region_entry(identifier, entry);
		});
	game_manager.map.lock_regions();
	return err;
}

Error GameSingleton::load_province_shape_file(String const& file_path) {
	if (province_index_image[0].is_valid()) {
		UtilityFunctions::push_error("Province shape file has already been loaded, cannot load: ", file_path);
		return FAILED;
	}
	Ref<Image> province_shape_image;
	province_shape_image.instantiate();
	Error err = province_shape_image->load(file_path);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to load province shape file: ", file_path);
		return err;
	}
	const int32_t width = province_shape_image->get_width();
	const int32_t height = province_shape_image->get_height();
	if (width < 1 || height < 1) {
		UtilityFunctions::push_error("Invalid dimensions (", width, "x", height, ") for province shape file: ", file_path);
		err = FAILED;
	}
	if (width % image_width_divide != 0) {
		UtilityFunctions::push_error("Invalid width ", width, " (must be divisible by ", image_width_divide, ") for province shape file: ", file_path);
		err = FAILED;
	}
	static constexpr Image::Format expected_format = Image::FORMAT_RGB8;
	const Image::Format format = province_shape_image->get_format();
	if (format != expected_format) {
		UtilityFunctions::push_error("Invalid format (", format, ", should be ", expected_format, ") for province shape file: ", file_path);
		err = FAILED;
	}
	if (err != OK) return err;
	err = ERR(game_manager.map.generate_province_index_image(width, height, province_shape_image->get_data().ptr()));

	std::vector<index_t> const& province_index_data = game_manager.map.get_province_index_image();
	const int32_t divided_width = width / image_width_divide;
	for (int32_t i = 0; i < image_width_divide; ++i) {
		PackedByteArray index_data_array;
		index_data_array.resize(divided_width * height * sizeof(index_t));
		for (int32_t y = 0; y < height; ++y)
			memcpy(index_data_array.ptrw() + y * divided_width * sizeof(index_t),
				province_index_data.data() + y * width + i * divided_width,
				divided_width * sizeof(index_t));
		province_index_image[i] = Image::create_from_data(divided_width, height, false, Image::FORMAT_RG8, index_data_array);
		if (province_index_image[i].is_null()) {
			UtilityFunctions::push_error("Failed to create province ID image #", i);
			err = FAILED;
		}
	}

	if (update_colour_image() != OK) err = FAILED;

	return err;
}

godot::Error GameSingleton::setup() {
	return ERR(game_manager.setup());
}

Error GameSingleton::load_water_province_file(String const& file_path) {
	Variant json_var;
	Error err = load_json_file("water province", file_path, json_var);
	if (err != OK) return err;
	Variant::Type type = json_var.get_type();
	if (type != Variant::ARRAY) {
		UtilityFunctions::push_error("Invalid water province JSON: root has type ",
			Variant::get_type_name(type), " (expected Array)");
		err = FAILED;
	}
	else {
		Array const& array = json_var;
		for (int64_t idx = 0; idx < array.size(); ++idx) {
			Variant const& entry = array[idx];
			type = entry.get_type();
			if (type != Variant::STRING) {
				UtilityFunctions::push_error("Invalid water province identifier: ", entry);
				err = FAILED;
				continue;
			}
			String const& identifier = entry;
			if (game_manager.map.set_water_province(identifier.utf8().get_data()) != SUCCESS)
				err = FAILED;
		}
	}
	game_manager.map.lock_water_provinces();
	return err;
}

int32_t GameSingleton::get_province_index_from_uv_coords(Vector2 const& coords) const {
	const size_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * get_width();
	const size_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * get_height();
	return game_manager.map.get_province_index_at(x_mod_w, y_mod_h);
}

#define KEY(x) static const StringName x##_key = #x;
Dictionary GameSingleton::get_province_info_from_index(int32_t index) const {
	Province const* province = game_manager.map.get_province_by_index(index);
	if (province == nullptr) return {};
	KEY(province) KEY(region) KEY(life_rating) KEY(buildings)
	Dictionary ret;

	ret[province_key] = province->get_identifier().c_str();

	Region const* region = province->get_region();
	if (region != nullptr) ret[region_key] = region->get_identifier().c_str();

	ret[life_rating_key] = province->get_life_rating();

	std::vector<Building> const& buildings = province->get_buildings();
	if (!buildings.empty()) {
		Array buildings_array;
		buildings_array.resize(buildings.size());
		for (size_t idx = 0; idx < buildings.size(); ++idx) {
			KEY(building) KEY(level) KEY(expansion_state) KEY(start_date) KEY(end_date) KEY(expansion_progress)
			
			Dictionary building_dict;
			Building const& building = buildings[idx];
			building_dict[building_key] = building.get_identifier().c_str();
			building_dict[level_key] = static_cast<int32_t>(building.get_level());
			building_dict[expansion_state_key] = static_cast<int32_t>(building.get_expansion_state());
			building_dict[start_date_key] = static_cast<std::string>(building.get_start_date()).c_str();
			building_dict[end_date_key] = static_cast<std::string>(building.get_end_date()).c_str();
			building_dict[expansion_progress_key] = building.get_expansion_progress();

			buildings_array[idx] = building_dict;
		}
		ret[buildings_key] = buildings_array;
	}
	return ret;
}
#undef KEY

int32_t GameSingleton::get_width() const {
	return game_manager.map.get_width();
}

int32_t GameSingleton::get_height() const {
	return game_manager.map.get_height();
}

Array GameSingleton::get_province_index_images() const {
	Array ret;
	for (int i = 0; i < image_width_divide; ++i)
		ret.append(province_index_image[i]);
	return ret;
}

Ref<Image> GameSingleton::get_province_colour_image() const {
	return province_colour_image;
}

Error GameSingleton::update_colour_image() {
	static PackedByteArray colour_data_array;
	static constexpr int64_t colour_data_array_size = (MAX_INDEX + 1) * 4;
	colour_data_array.resize(colour_data_array_size);

	Error err = OK;
	if (game_manager.map.generate_mapmode_colours(mapmode_index, colour_data_array.ptrw()) != SUCCESS)
		err = FAILED;

	static constexpr int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(index_t) * 4);
	if (province_colour_image.is_null())
		province_colour_image.instantiate();
	province_colour_image->set_data(PROVINCE_INDEX_SQRT, PROVINCE_INDEX_SQRT,
		false, Image::FORMAT_RGBA8, colour_data_array);
	if (province_colour_image.is_null()) {
		UtilityFunctions::push_error("Failed to update province colour image");
		return FAILED;
	}
	return err;
}

int32_t GameSingleton::get_mapmode_count() const {
	return game_manager.map.get_mapmode_count();
}

String GameSingleton::get_mapmode_identifier(int32_t index) const {
	Mapmode const* mapmode = game_manager.map.get_mapmode_by_index(index);
	if (mapmode != nullptr) return mapmode->get_identifier().c_str();
	return String{};
}

Error GameSingleton::set_mapmode(godot::String const& identifier) {
	Mapmode const* mapmode = game_manager.map.get_mapmode_by_identifier(identifier.utf8().get_data());
	if (mapmode == nullptr) {
		UtilityFunctions::push_error("Failed to set mapmode to: ", identifier);
		return FAILED;
	}
	mapmode_index = mapmode->get_index();
	return OK;
}

Error GameSingleton::expand_building(int32_t province_index, String const& building_type_identifier) {
	if (game_manager.expand_building(province_index, building_type_identifier.utf8().get_data()) != SUCCESS) {
		UtilityFunctions::push_error("Failed to expand ", building_type_identifier, " at province index ", province_index);
		return FAILED;
	}
	return OK;
}

void GameSingleton::set_paused(bool paused) {
	game_manager.clock.isPaused = paused;
}

void GameSingleton::toggle_paused() {
	game_manager.clock.isPaused = !game_manager.clock.isPaused;
}

bool GameSingleton::is_paused() const {
	return game_manager.clock.isPaused;
}

void GameSingleton::increase_speed() {
	game_manager.clock.increaseSimulationSpeed();
}

void GameSingleton::decrease_speed() {
	game_manager.clock.decreaseSimulationSpeed();
}

bool GameSingleton::can_increase_speed() const {
	return game_manager.clock.canIncreaseSimulationSpeed();
}

bool GameSingleton::can_decrease_speed() const {
	return game_manager.clock.canDecreaseSimulationSpeed();
}

String GameSingleton::get_longform_date() const {
	return static_cast<std::string>(game_manager.get_today()).c_str();
}

void GameSingleton::try_tick() {
	game_manager.clock.conditionallyAdvanceGame();
}
