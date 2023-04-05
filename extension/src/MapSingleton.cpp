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
	ClassDB::bind_method(D_METHOD("get_province_shape_image"), &MapSingleton::get_province_shape_image);
	ClassDB::bind_method(D_METHOD("get_province_identifier_from_colour", "colour"), &MapSingleton::get_province_identifier_from_colour);
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
		if (map.add_province(identifier.utf8().get_data(), colour, error_message)) {
			UtilityFunctions::print(error_message.c_str());
		} else {
			UtilityFunctions::push_error(error_message.c_str());
			err = FAILED;
		}
	}
	return err;
}

String MapSingleton::get_province_identifier_from_colour(Province::colour_t colour) {
	const Province* province = map.get_province_by_colour(colour);
	if (province) return province->get_identifier().c_str();
	else return String{};
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
	int32_t width = province_shape_image->get_width();
	int32_t height = province_shape_image->get_height();
	if (width < 1 || height < 1) {
		UtilityFunctions::push_error("Invalid dimensions (", width, "x", height, ") for province shape file: ", file_path);
		err = FAILED;
	}
	static const Image::Format expected_format = Image::Format::FORMAT_RGB8; 
	Image::Format format = province_shape_image->get_format();
	if (format != expected_format) {
		UtilityFunctions::push_error("Invalid format (", format, ", should be ", expected_format, ") for province shape file: ", file_path);
		err = FAILED;
	}
	if (err) {
		province_shape_image.unref();
		return err;
	}
	return err;
}

Ref<Image> MapSingleton::get_province_shape_image() const {
	return province_shape_image;
}
