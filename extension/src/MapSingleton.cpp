#include "MapSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>

using namespace godot;
using namespace OpenVic2;

MapSingleton* MapSingleton::singleton = nullptr;

void MapSingleton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_province_identifier_file", "file_path"), &MapSingleton::load_province_identifier_file);
	ClassDB::bind_method(D_METHOD("load_province_shape_file", "file_path"), &MapSingleton::load_province_shape_file);
	ClassDB::bind_method(D_METHOD("get_province_identifier_from_pixel_coords", "coords"), &MapSingleton::get_province_identifier_from_pixel_coords);
	ClassDB::bind_method(D_METHOD("get_width"), &MapSingleton::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &MapSingleton::get_height);
	ClassDB::bind_method(D_METHOD("get_province_index_image"), &MapSingleton::get_province_index_image);
	ClassDB::bind_method(D_METHOD("get_province_colour_image"), &MapSingleton::get_province_colour_image);
}

MapSingleton* MapSingleton::get_singleton() {
	return singleton;
}

MapSingleton::MapSingleton() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

MapSingleton::~MapSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

Error MapSingleton::load_province_identifier_file(String const& file_path) {
	UtilityFunctions::print("Loading identifier file: ", file_path);
	Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load province identifier file: ", file_path);
		return err == OK ? FAILED : err;
	}
	String json_string = file->get_as_text();
	Ref<JSON> json;
	json.instantiate();
	err = json->parse(json_string);
	if (err) {
		UtilityFunctions::push_error("Failed to parse province identifier file as JSON: ", file_path,
			"\nError at line ", json->get_error_line(), ": ", json->get_error_message());
		return err;
	}
	Variant json_var = json->get_data();
	Variant::Type type = json_var.get_type();
	if (type != Variant::DICTIONARY) {
		UtilityFunctions::push_error("Invalid province identifier JSON: root has type ",
			Variant::get_type_name(type), " (expected Dictionary)");
		return FAILED;
	}
	Dictionary prov_dict = json_var;
	Array prov_identifiers = prov_dict.keys();
	for (int idx = 0; idx < prov_identifiers.size(); ++idx) {
		String const& identifier = prov_identifiers[idx];
		Variant const& colour_var = prov_dict[identifier];
		if (identifier.is_empty()) {
			UtilityFunctions::push_error("Empty province identifier with colour: ", colour_var);
			err = FAILED;
			continue;
		}
		static const String prov_prefix = "prov_";
		if (!identifier.begins_with(prov_prefix))
			UtilityFunctions::push_warning("Province identifier missing prefix: ", identifier);
		type = colour_var.get_type();
		Province::colour_t colour = Province::NULL_COLOUR;
		if (type == Variant::ARRAY) {
			Array colour_array = colour_var;
			if (colour_array.size() == 3) {
				for (int jdx = 0; jdx < 3; ++jdx) {
					Variant var = colour_array[jdx];
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
			String colour_string = colour_var;
			if (colour_string.is_valid_hex_number()) {
				int64_t colour_int = colour_string.hex_to_int();
				if (0 <= colour_int && colour_int <= 0xFFFFFF)
					colour = colour_int;
			}
		}
		if (colour == Province::NULL_COLOUR) {
			UtilityFunctions::push_error("Invalid province identifier colour for ", identifier, ": ", colour_var);
			err = FAILED;
			continue;
		}
		std::string error_message;
		if (!map.add_province(identifier.utf8().get_data(), colour, error_message)) {
			UtilityFunctions::push_error(error_message.c_str());
			err = FAILED;
		}
	}
	map.lock_provinces();
	return err;
}

static Province::colour_t colour_at(PackedByteArray const& colour_data_array, int32_t idx) {
	return (colour_data_array[idx * 3] << 16) | (colour_data_array[idx * 3 + 1] << 8) | colour_data_array[idx * 3 + 2];
}

Error MapSingleton::load_province_shape_file(String const& file_path) {
	if (province_shape_image.is_valid()) {
		UtilityFunctions::push_error("Province shape file has already been loaded, cannot load: ", file_path);
		return FAILED;
	}
	province_shape_image.instantiate();
	Error err = province_shape_image->load(file_path);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to load province shape file: ", file_path);
		province_shape_image.unref();
		return err;
	}
	width = province_shape_image->get_width();
	height = province_shape_image->get_height();
	if (width < 1 || height < 1) {
		UtilityFunctions::push_error("Invalid dimensions (", width, "x", height, ") for province shape file: ", file_path);
		err = FAILED;
	}
	static const Image::Format expected_format = Image::FORMAT_RGB8;
	Image::Format format = province_shape_image->get_format();
	if (format != expected_format) {
		UtilityFunctions::push_error("Invalid format (", format, ", should be ", expected_format, ") for province shape file: ", file_path);
		err = FAILED;
	}
	if (err) {
		province_shape_image.unref();
		return err;
	}

	std::vector<bool> province_checklist(map.get_province_count());

	PackedByteArray shape_data_array = province_shape_image->get_data(), index_data_array;
	index_data_array.resize(width * height * sizeof(Province::index_t));
	Province::index_t* index_data = reinterpret_cast<Province::index_t*>(index_data_array.ptrw());

	for (int32_t y = 0; y < height; ++y) {
		for (int32_t x = 0; x < width; ++x) {
			const int32_t idx = x + y * width;
			const Province::colour_t colour = colour_at(shape_data_array, idx);
			if (colour == Province::NULL_COLOUR) {
				index_data[idx] = Province::NULL_INDEX;
				continue;
			}
			if (x > 0) {
				const int32_t jdx = idx - 1;
				if (colour_at(shape_data_array, jdx) == colour) {
					index_data[idx] = index_data[jdx];
					continue;
				}
			}
			if (y > 0) {
				const int32_t jdx = idx - width;
				if (colour_at(shape_data_array, jdx) == colour) {
					index_data[idx] = index_data[jdx];
					continue;
				}
			}
			const Province* province = map.get_province_by_colour(colour);
			if (province) {
				Province::index_t index = province->get_index();
				index_data[idx] = index;
				province_checklist[index - 1] = true;
				continue;
			}
			UtilityFunctions::push_error("Unrecognised province colour ", Province::colour_to_hex_string(colour).c_str(), " at (", x, ", ", y, ")");
			err = FAILED;
			index_data[idx] = Province::NULL_INDEX;
		}
	}

	for (size_t idx = 0; idx < province_checklist.size(); ++idx) {
		if (!province_checklist[idx]) {
			Province* province = map.get_province_by_index(idx + 1);
			if (province) UtilityFunctions::push_error("Province missing from shape image: ", province->to_string().c_str());
			else UtilityFunctions::push_error("Province missing for index: ", static_cast<int32_t>(idx + 1));
			err = FAILED;
		}
	}

	province_index_image = Image::create_from_data(width, height, false, Image::FORMAT_RG8, index_data_array);
	if (province_index_image.is_null()) {
		UtilityFunctions::push_error("Failed to create province ID image");
		err = FAILED;
	}

	PackedByteArray colour_data_array;
	colour_data_array.resize((Province::MAX_INDEX + 1) * 3);
	for (size_t idx = 1; idx <= map.get_province_count(); ++idx) {
		const Province* province = map.get_province_by_index(idx);
		if (province) {
			const Province::colour_t colour = province->get_colour();
			colour_data_array[3 * idx + 0] = (colour >> 16) & 0xFF;
			colour_data_array[3 * idx + 1] = (colour >> 8) & 0xFF;
			colour_data_array[3 * idx + 2] = colour & 0xFF;
		} else UtilityFunctions::push_error("Missing province at index ", static_cast<int32_t>(idx));
	}
	static const int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(Province::index_t) * 4);
	province_colour_image = Image::create_from_data(PROVINCE_INDEX_SQRT, PROVINCE_INDEX_SQRT, false, Image::FORMAT_RGB8, colour_data_array);
	if (province_colour_image.is_null()) {
		UtilityFunctions::push_error("Failed to create province colour image");
		err = FAILED;
	}

	return err;
}

String MapSingleton::get_province_identifier_from_pixel_coords(Vector2i const& coords) {
	if (province_index_image.is_valid()) {
		const PackedByteArray index_data_array = province_index_image->get_data();
		const Province::index_t* index_data = reinterpret_cast<const Province::index_t*>(index_data_array.ptr());
		const int32_t x_mod_w = UtilityFunctions::posmod(coords.x, width);
		const int32_t y_mod_h = UtilityFunctions::posmod(coords.y, height);
		const Province* province = map.get_province_by_index(index_data[x_mod_w + y_mod_h * width]);
		if (province) return province->get_identifier().c_str();
	}
	return String{};
}

int32_t MapSingleton::get_width() const {
	return width;
}

int32_t MapSingleton::get_height() const {
	return height;
}

Ref<Image> MapSingleton::get_province_index_image() const {
	return province_index_image;
}

Ref<Image> MapSingleton::get_province_colour_image() const {
	return province_colour_image;
}
