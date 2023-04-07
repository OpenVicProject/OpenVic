#include "MapSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>

using namespace godot;
using namespace OpenVic2;

MapSingleton* MapSingleton::singleton = nullptr;

void MapSingleton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_province_identifier_file", "file_path"), &MapSingleton::load_province_identifier_file);
	ClassDB::bind_method(D_METHOD("load_region_file", "file_path"), &MapSingleton::load_region_file);
	ClassDB::bind_method(D_METHOD("load_province_shape_file", "file_path"), &MapSingleton::load_province_shape_file);

	ClassDB::bind_method(D_METHOD("get_province_index_from_uv_coords", "coords"), &MapSingleton::get_province_index_from_uv_coords);
	ClassDB::bind_method(D_METHOD("get_province_identifier_from_uv_coords", "coords"), &MapSingleton::get_province_identifier_from_uv_coords);
	ClassDB::bind_method(D_METHOD("get_width"), &MapSingleton::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &MapSingleton::get_height);
	ClassDB::bind_method(D_METHOD("get_province_index_image"), &MapSingleton::get_province_index_image);
	ClassDB::bind_method(D_METHOD("get_province_colour_image"), &MapSingleton::get_province_colour_image);

	ClassDB::bind_method(D_METHOD("update_colour_image"), &MapSingleton::update_colour_image);
	ClassDB::bind_method(D_METHOD("get_mapmode_count"), &MapSingleton::get_mapmode_count);
	ClassDB::bind_method(D_METHOD("get_mapmode_identifier", "index"), &MapSingleton::get_mapmode_identifier);
	ClassDB::bind_method(D_METHOD("set_mapmode", "identifier"), &MapSingleton::set_mapmode);
}

MapSingleton* MapSingleton::get_singleton() {
	return singleton;
}

/* REQUIREMENTS:
 * MAP-21, MAP-25
 */
MapSingleton::MapSingleton() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	using mapmode_t = std::pair<std::string, Mapmode::colour_func_t>;
	const std::vector<mapmode_t> mapmodes = {
		{ "mapmode_province", [](Map const&, Province const& province) -> Province::colour_t { return province.get_colour(); } },
		{ "mapmode_region", [](Map const&, Province const& province) -> Province::colour_t {
			Region const* region = province.get_region();
			if (region != nullptr) return region->get_provinces().front()->get_colour();
			return province.get_colour();
		} },
		{ "mapmode_index", [](Map const& map, Province const& province) -> Province::colour_t {
			const uint8_t f = float(province.get_index()) / float(map.get_province_count()) * 255.0f;
			return (f << 16) | (f << 8) | f;
		} }
	};
	std::string error_message = "";
	for (mapmode_t mapmode : mapmodes)
		if (map.add_mapmode(mapmode.first, mapmode.second, error_message) != SUCCESS)
			UtilityFunctions::push_error(error_message.c_str());
}

MapSingleton::~MapSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

Error MapSingleton::parse_json_dictionary_file(String const& file_description, String const& file_path,
	String const& identifier_prefix, parse_json_entry_func_t parse_entry) const {
	UtilityFunctions::print("Loading ", file_description, " file: ", file_path);
	Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load ", file_description, " file: ", file_path);
		return err == OK ? FAILED : err;
	}
	const String json_string = file->get_as_text();
	Ref<JSON> json;
	json.instantiate();
	err = json->parse(json_string);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to parse ", file_description, " file as JSON: ", file_path,
			"\nError at line ", json->get_error_line(), ": ", json->get_error_message());
		return err;
	}
	const Variant json_var = json->get_data();
	const Variant::Type type = json_var.get_type();
	if (type != Variant::DICTIONARY) {
		UtilityFunctions::push_error("Invalid ", file_description, " JSON: root has type ",
			Variant::get_type_name(type), " (expected Dictionary)");
		return FAILED;
	}
	const Dictionary dict = json_var;
	const Array identifiers = dict.keys();
	for (int idx = 0; idx < identifiers.size(); ++idx) {
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

Error MapSingleton::_parse_province_identifier_entry(String const& identifier, Variant const& entry) {
	const Variant::Type type = entry.get_type();
	Province::colour_t colour = Province::NULL_COLOUR;
	if (type == Variant::ARRAY) {
		const Array colour_array = entry;
		if (colour_array.size() == 3) {
			for (int jdx = 0; jdx < 3; ++jdx) {
				const Variant var = colour_array[jdx];
				if (var.get_type() != Variant::FLOAT) {
					colour = Province::NULL_COLOUR;
					break;
				}
				double colour_double = var;
				if (std::trunc(colour_double) != colour_double) {
					colour = Province::NULL_COLOUR;
					break;
				}
				int64_t colour_int = static_cast<int64_t>(colour_double);
				if (colour_int < 0 || colour_int > 255) {
					colour = Province::NULL_COLOUR;
					break;
				}
				colour = (colour << 8) | colour_int;
			}
		}
	} else if (type == Variant::STRING) {
		String colour_string = entry;
		if (colour_string.is_valid_hex_number()) {
			int64_t colour_int = colour_string.hex_to_int();
			if (0 <= colour_int && colour_int <= 0xFFFFFF)
				colour = colour_int;
		}
	} else {
		UtilityFunctions::push_error("Invalid colour for province identifier \"", identifier, "\": ", entry);
		return FAILED;
	}
	std::string error_message = "";
	if (map.add_province(identifier.utf8().get_data(), colour, error_message) != SUCCESS) {
		UtilityFunctions::push_error(error_message.c_str());
		return FAILED;
	}
	return OK;
}

Error MapSingleton::load_province_identifier_file(String const& file_path) {
	const Error err = parse_json_dictionary_file("province identifier", file_path, "prov_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return this->_parse_province_identifier_entry(identifier, entry);
		});
	map.lock_provinces();
	return err;
}

Error MapSingleton::_parse_region_entry(String const& identifier, Variant const& entry) {
	Error err = OK;
	Variant::Type type = entry.get_type();
	std::vector<std::string> province_identifiers;
	if (type == Variant::ARRAY) {
		const Array province_array = entry;
		for (int64_t idx = 0; idx < province_array.size(); ++idx) {
			const Variant province_var = province_array[idx];
			type = province_var.get_type();
			if (type == Variant::STRING) {
				String province_string = province_var;
				province_identifiers.push_back(province_string.utf8().get_data());
			} else {
				UtilityFunctions::push_error("Invalid province identifier for region \"", identifier, "\": ", entry);
				err = FAILED;
			}
		}
	}
	std::string error_message = "";
	if (map.add_region(identifier.utf8().get_data(), province_identifiers, error_message) != SUCCESS) {
		UtilityFunctions::push_error(error_message.c_str());
		return FAILED;
	}
	return err;
}

Error MapSingleton::load_region_file(String const& file_path) {
	const Error err = parse_json_dictionary_file("region", file_path, "region_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return this->_parse_region_entry(identifier, entry);
		});
	map.lock_regions();
	return err;
}

Error MapSingleton::load_province_shape_file(String const& file_path) {
	if (province_index_image.is_valid()) {
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
	int32_t width = province_shape_image->get_width();
	int32_t height = province_shape_image->get_height();
	if (width < 1 || height < 1) {
		UtilityFunctions::push_error("Invalid dimensions (", width, "x", height, ") for province shape file: ", file_path);
		err = FAILED;
	}
	static const Image::Format expected_format = Image::FORMAT_RGB8;
	const Image::Format format = province_shape_image->get_format();
	if (format != expected_format) {
		UtilityFunctions::push_error("Invalid format (", format, ", should be ", expected_format, ") for province shape file: ", file_path);
		err = FAILED;
	}
	if (err != OK) return err;

	std::string error_message = "";
	if (map.generate_province_index_image(width, height, province_shape_image->get_data().ptr(), error_message) != SUCCESS) {
		UtilityFunctions::push_error(error_message.c_str());
		err = FAILED;
	}

	PackedByteArray index_data_array;
	index_data_array.resize(width * height * sizeof(Province::index_t));
	memcpy(index_data_array.ptrw(), map.get_province_index_image().data(), index_data_array.size());

	province_index_image = Image::create_from_data(width, height, false, Image::FORMAT_RG8, index_data_array);
	if (province_index_image.is_null()) {
		UtilityFunctions::push_error("Failed to create province ID image");
		err = FAILED;
	}

	if (update_colour_image() != OK) err = FAILED;

	return err;
}

Province* MapSingleton::get_province_from_uv_coords(godot::Vector2 const& coords) {
	if (province_index_image.is_valid()) {
		const PackedByteArray index_data_array = province_index_image->get_data();
		Province::index_t const* index_data = reinterpret_cast<Province::index_t const*>(index_data_array.ptr());
		const int32_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * get_width();
		const int32_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * get_height();
		return map.get_province_by_index(index_data[x_mod_w + y_mod_h * get_width()]);
	}
	return nullptr;
}

Province const* MapSingleton::get_province_from_uv_coords(godot::Vector2 const& coords) const {
	if (province_index_image.is_valid()) {
		const PackedByteArray index_data_array = province_index_image->get_data();
		Province::index_t const* index_data = reinterpret_cast<Province::index_t const*>(index_data_array.ptr());
		const int32_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * get_width();
		const int32_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * get_height();
		return map.get_province_by_index(index_data[x_mod_w + y_mod_h * get_width()]);
	}
	return nullptr;
}

int32_t MapSingleton::get_province_index_from_uv_coords(Vector2 const& coords) const {
	Province const* province = get_province_from_uv_coords(coords);
	if (province != nullptr) return province->get_index();
	return Province::NULL_INDEX;
}

String MapSingleton::get_province_identifier_from_uv_coords(Vector2 const& coords) const {
	Province const* province = get_province_from_uv_coords(coords);
	if (province != nullptr) return province->get_identifier().c_str();
	return String{};
}

int32_t MapSingleton::get_width() const {
	return map.get_width();
}

int32_t MapSingleton::get_height() const {
	return map.get_height();
}

Ref<Image> MapSingleton::get_province_index_image() const {
	return province_index_image;
}

Ref<Image> MapSingleton::get_province_colour_image() const {
	return province_colour_image;
}

Error MapSingleton::update_colour_image() {
	static PackedByteArray colour_data_array;
	static const int64_t colour_data_array_size = (Province::MAX_INDEX + 1) * 3;
	colour_data_array.resize(colour_data_array_size);

	Error err = OK;
	std::string error_message = "";
	if (map.generate_mapmode_colours(mapmode_index, colour_data_array.ptrw(), error_message) != SUCCESS) {
		UtilityFunctions::push_error(error_message.c_str());
		err = FAILED;
	}

	static const int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(Province::index_t) * 4);
	if (province_colour_image.is_null())
		province_colour_image.instantiate();
	province_colour_image->set_data(PROVINCE_INDEX_SQRT, PROVINCE_INDEX_SQRT,
		false, Image::FORMAT_RGB8, colour_data_array);
	if (province_colour_image.is_null()) {
		UtilityFunctions::push_error("Failed to update province colour image");
		return FAILED;
	}
	return err;
}

int32_t MapSingleton::get_mapmode_count() const {
	return map.get_mapmode_count();
}

String MapSingleton::get_mapmode_identifier(int32_t index) const {
	Mapmode const* mapmode = map.get_mapmode_by_index(index);
	if (mapmode != nullptr) return mapmode->get_identifier().c_str();
	return String{};
}

Error MapSingleton::set_mapmode(godot::String const& identifier) {
	Mapmode const* mapmode = map.get_mapmode_by_identifier(identifier.utf8().get_data());
	if (mapmode != nullptr) {
		mapmode_index = mapmode->get_index();
		return OK;
	} else {
		UtilityFunctions::push_error("Failed to set mapmode to: ", identifier);
		return FAILED;
	}
}
