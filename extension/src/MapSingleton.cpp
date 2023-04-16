#include "MapSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>

using namespace godot;
using namespace OpenVic2;

MapSingleton* MapSingleton::singleton = nullptr;

void MapSingleton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_province_identifier_file", "file_path"), &MapSingleton::loadProvinceIdentifierFile);
	ClassDB::bind_method(D_METHOD("load_region_file", "file_path"), &MapSingleton::loadRegionFile);
	ClassDB::bind_method(D_METHOD("load_province_shape_file", "file_path"), &MapSingleton::loadProvinceShapeFile);

	ClassDB::bind_method(D_METHOD("get_province_index_from_uv_coords", "coords"), &MapSingleton::getProvinceIndexFromUVCoords);
	ClassDB::bind_method(D_METHOD("get_province_identifier_from_uv_coords", "coords"), &MapSingleton::getProvinceIdentifierFromUVCoords);
	ClassDB::bind_method(D_METHOD("get_width"), &MapSingleton::getWidth);
	ClassDB::bind_method(D_METHOD("get_height"), &MapSingleton::getHeight);
	ClassDB::bind_method(D_METHOD("get_province_index_image"), &MapSingleton::getProvinceIndexImage);
	ClassDB::bind_method(D_METHOD("get_province_colour_image"), &MapSingleton::getProvinceColourImage);

	ClassDB::bind_method(D_METHOD("update_colour_image"), &MapSingleton::updateColourImage);
	ClassDB::bind_method(D_METHOD("get_mapmode_count"), &MapSingleton::getMapmodeCount);
	ClassDB::bind_method(D_METHOD("get_mapmode_identifier", "index"), &MapSingleton::getMapmodeIdentifier);
	ClassDB::bind_method(D_METHOD("set_mapmode", "identifier"), &MapSingleton::setMapmode);
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
		{ "mapmode_province", [](Map const&, Province const& province) -> Province::colour_t { return province.getColour(); } },
		{ "mapmode_region", [](Map const&, Province const& province) -> Province::colour_t {
			Region const* region = province.getRegion();
			if (region != nullptr) return region->getProvinces().front()->getColour();
			return province.getColour();
		} },
		{ "mapmode_index", [](Map const& map, Province const& province) -> Province::colour_t {
			const uint8_t f = float(province.getIndex()) / float(map.getProvinceCount()) * 255.0f;
			return (f << 16) | (f << 8) | f;
		} }
	};
	std::string errorMessage = "";
	for (mapmode_t mapmode : mapmodes)
		if (map.addMapmode(mapmode.first, mapmode.second, errorMessage) != SUCCESS)
			UtilityFunctions::push_error(errorMessage.c_str());
}

MapSingleton::~MapSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

Error MapSingleton::parseJsonDictionaryFile(String const& fileDescription, String const& filePath,
	String const& identifierPrefix, parse_json_entry_func_t parseEntry) const {
	UtilityFunctions::print("Loading ", fileDescription, " file: ", filePath);
	Ref<FileAccess> file = FileAccess::open(filePath, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to load ", fileDescription, " file: ", filePath);
		return err == OK ? FAILED : err;
	}
	const String jsonString = file->get_as_text();
	Ref<JSON> json;
	json.instantiate();
	err = json->parse(jsonString);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to parse ", fileDescription, " file as JSON: ", filePath,
			"\nError at line ", json->get_error_line(), ": ", json->get_error_message());
		return err;
	}
	const Variant jsonVar = json->get_data();
	const Variant::Type type = jsonVar.get_type();
	if (type != Variant::DICTIONARY) {
		UtilityFunctions::push_error("Invalid ", fileDescription, " JSON: root has type ",
			Variant::get_type_name(type), " (expected Dictionary)");
		return FAILED;
	}
	const Dictionary dict = jsonVar;
	const Array identifiers = dict.keys();
	for (int idx = 0; idx < identifiers.size(); ++idx) {
		String const& identifier = identifiers[idx];
		Variant const& entry = dict[identifier];
		if (identifier.is_empty()) {
			UtilityFunctions::push_error("Empty identifier in ", fileDescription, " file with entry: ", entry);
			err = FAILED;
			continue;
		}
		if (!identifier.begins_with(identifierPrefix))
			UtilityFunctions::push_warning("Identifier in ", fileDescription, " file missing \"", identifierPrefix, "\" prefix: ", identifier);
		if (parseEntry(identifier, entry) != OK) err = FAILED;
	}
	return err;
}

Error MapSingleton::_parseProvinceIdentifierEntry(String const& identifier, Variant const& entry) {
	const Variant::Type type = entry.get_type();
	Province::colour_t colour = Province::NULL_COLOUR;
	if (type == Variant::ARRAY) {
		const Array colourArray = entry;
		if (colourArray.size() == 3) {
			for (int jdx = 0; jdx < 3; ++jdx) {
				const Variant var = colourArray[jdx];
				if (var.get_type() != Variant::FLOAT) {
					colour = Province::NULL_COLOUR;
					break;
				}
				double colourDouble = var;
				if (std::trunc(colourDouble) != colourDouble) {
					colour = Province::NULL_COLOUR;
					break;
				}
				int64_t colourInt = static_cast<int64_t>(colourDouble);
				if (colourInt < 0 || colourInt > 255) {
					colour = Province::NULL_COLOUR;
					break;
				}
				colour = (colour << 8) | colourInt;
			}
		}
	}
	else if (type == Variant::STRING) {
		String colourString = entry;
		if (colourString.is_valid_hex_number()) {
			int64_t colourInt = colourString.hex_to_int();
			if (0 <= colourInt && colourInt <= 0xFFFFFF)
				colour = colourInt;
		}
	}
	else {
		UtilityFunctions::push_error("Invalid colour for province identifier \"", identifier, "\": ", entry);
		return FAILED;
	}
	std::string error_message = "";
	if (map.addProvince(identifier.utf8().get_data(), colour, error_message) != SUCCESS) {
		UtilityFunctions::push_error(error_message.c_str());
		return FAILED;
	}
	return OK;
}

Error MapSingleton::loadProvinceIdentifierFile(String const& filePath) {
	const Error err = parseJsonDictionaryFile("province identifier", filePath, "prov_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return this->_parseProvinceIdentifierEntry(identifier, entry);
		});
	map.lockProvinces();
	return err;
}

Error MapSingleton::_parseRegionEntry(String const& identifier, Variant const& entry) {
	Error err = OK;
	Variant::Type type = entry.get_type();
	std::vector<std::string> provinceIdentifiers;
	if (type == Variant::ARRAY) {
		const Array provinceArray = entry;
		for (int64_t idx = 0; idx < provinceArray.size(); ++idx) {
			const Variant provinceVar = provinceArray[idx];
			type = provinceVar.get_type();
			if (type == Variant::STRING) {
				String provinceString = provinceVar;
				provinceIdentifiers.push_back(provinceString.utf8().get_data());
			}
			else {
				UtilityFunctions::push_error("Invalid province identifier for region \"", identifier, "\": ", entry);
				err = FAILED;
			}
		}
	}
	std::string errorMessage = "";
	if (map.addRegion(identifier.utf8().get_data(), provinceIdentifiers, errorMessage) != SUCCESS) {
		UtilityFunctions::push_error(errorMessage.c_str());
		return FAILED;
	}
	return err;
}

Error MapSingleton::loadRegionFile(String const& filePath) {
	const Error err = parseJsonDictionaryFile("region", filePath, "region_",
		[this](String const& identifier, Variant const& entry) -> Error {
			return this->_parseRegionEntry(identifier, entry);
		});
	map.lockRegions();
	return err;
}

Error MapSingleton::loadProvinceShapeFile(String const& filePath) {
	if (provinceIndexImage.is_valid()) {
		UtilityFunctions::push_error("Province shape file has already been loaded, cannot load: ", filePath);
		return FAILED;
	}
	Ref<Image> provinceShapeImage;
	provinceShapeImage.instantiate();
	Error err = provinceShapeImage->load(filePath);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to load province shape file: ", filePath);
		return err;
	}
	int32_t width = provinceShapeImage->get_width();
	int32_t height = provinceShapeImage->get_height();
	if (width < 1 || height < 1) {
		UtilityFunctions::push_error("Invalid dimensions (", width, "x", height, ") for province shape file: ", filePath);
		err = FAILED;
	}
	static const Image::Format expectedFormat = Image::FORMAT_RGB8;
	const Image::Format format = provinceShapeImage->get_format();
	if (format != expectedFormat) {
		UtilityFunctions::push_error("Invalid format (", format, ", should be ", expectedFormat, ") for province shape file: ", filePath);
		err = FAILED;
	}
	if (err != OK) return err;

	std::string errorMessage = "";
	if (map.generateProvinceIndexImage(width, height, provinceShapeImage->get_data().ptr(), errorMessage) != SUCCESS) {
		UtilityFunctions::push_error(errorMessage.c_str());
		err = FAILED;
	}

	PackedByteArray indexDataArray;
	indexDataArray.resize(width * height * sizeof(Province::index_t));
	memcpy(indexDataArray.ptrw(), map.getProvinceIndexImage().data(), indexDataArray.size());

	provinceIndexImage = Image::create_from_data(width, height, false, Image::FORMAT_RG8, indexDataArray);
	if (provinceIndexImage.is_null()) {
		UtilityFunctions::push_error("Failed to create province ID image");
		err = FAILED;
	}

	if (updateColourImage() != OK) err = FAILED;

	return err;
}

Province* MapSingleton::getProvinceFromUVCoords(godot::Vector2 const& coords) {
	if (provinceIndexImage.is_valid()) {
		const PackedByteArray indexDataArray = provinceIndexImage->get_data();
		Province::index_t const* indexData = reinterpret_cast<Province::index_t const*>(indexDataArray.ptr());
		const int32_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * getWidth();
		const int32_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * getHeight();
		return map.getProvinceByIndex(indexData[x_mod_w + y_mod_h * getWidth()]);
	}
	return nullptr;
}

Province const* MapSingleton::getProvinceFromUVCoords(godot::Vector2 const& coords) const {
	if (provinceIndexImage.is_valid()) {
		const PackedByteArray indexDataArray = provinceIndexImage->get_data();
		Province::index_t const* indexData = reinterpret_cast<Province::index_t const*>(indexDataArray.ptr());
		const int32_t x_mod_w = UtilityFunctions::fposmod(coords.x, 1.0f) * getWidth();
		const int32_t y_mod_h = UtilityFunctions::fposmod(coords.y, 1.0f) * getHeight();
		return map.getProvinceByIndex(indexData[x_mod_w + y_mod_h * getWidth()]);
	}
	return nullptr;
}

int32_t MapSingleton::getProvinceIndexFromUVCoords(Vector2 const& coords) const {
	Province const* province = getProvinceFromUVCoords(coords);
	if (province != nullptr) return province->getIndex();
	return Province::NULL_INDEX;
}

String MapSingleton::getProvinceIdentifierFromUVCoords(Vector2 const& coords) const {
	Province const* province = getProvinceFromUVCoords(coords);
	if (province != nullptr) return province->getIdentifier().c_str();
	return String{};
}

int32_t MapSingleton::getWidth() const {
	return map.getWidth();
}

int32_t MapSingleton::getHeight() const {
	return map.getHeight();
}

Ref<Image> MapSingleton::getProvinceIndexImage() const {
	return provinceIndexImage;
}

Ref<Image> MapSingleton::getProvinceColourImage() const {
	return provinceColourImage;
}

Error MapSingleton::updateColourImage() {
	static PackedByteArray colourDataArray;
	static const int64_t colourDataArraySize = (Province::MAX_INDEX + 1) * 3;
	colourDataArray.resize(colourDataArraySize);

	Error err = OK;
	std::string errorMessage = "";
	if (map.generateMapmodeColours(mapmodeIndex, colourDataArray.ptrw(), errorMessage) != SUCCESS) {
		UtilityFunctions::push_error(errorMessage.c_str());
		err = FAILED;
	}

	static const int32_t PROVINCE_INDEX_SQRT = 1 << (sizeof(Province::index_t) * 4);
	if (provinceColourImage.is_null())
		provinceColourImage.instantiate();
	provinceColourImage->set_data(PROVINCE_INDEX_SQRT, PROVINCE_INDEX_SQRT,
		false, Image::FORMAT_RGB8, colourDataArray);
	if (provinceColourImage.is_null()) {
		UtilityFunctions::push_error("Failed to update province colour image");
		return FAILED;
	}
	return err;
}

int32_t MapSingleton::getMapmodeCount() const {
	return map.getMapmodeCount();
}

String MapSingleton::getMapmodeIdentifier(int32_t index) const {
	Mapmode const* mapmode = map.getMapmodeByIndex(index);
	if (mapmode != nullptr) return mapmode->getIdentifier().c_str();
	return String{};
}

Error MapSingleton::setMapmode(godot::String const& identifier) {
	Mapmode const* mapmode = map.getMapmodeByIdentifier(identifier.utf8().get_data());
	if (mapmode != nullptr) {
		mapmodeIndex = mapmode->getIndex();
		return OK;
	}
	else {
		UtilityFunctions::push_error("Failed to set mapmode to: ", identifier);
		return FAILED;
	}
}
