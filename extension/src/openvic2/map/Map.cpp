#include "openvic2/map/Map.hpp"

#include <cassert>
#include <unordered_set>

#include "openvic2/Logger.hpp"

using namespace OpenVic2;

Mapmode::Mapmode(index_t new_index, std::string const& new_identifier, colour_func_t new_colour_func)
	: HasIdentifier{ new_identifier }, index{ new_index }, colour_func{ new_colour_func } {
	assert(colour_func != nullptr);
}

Mapmode::index_t Mapmode::get_index() const {
	return index;
}

Province::colour_t Mapmode::get_colour(Map const& map, Province const& province) const {
	return colour_func ? colour_func(map, province) : Province::NULL_COLOUR;
}

Map::Map() : provinces{ "provinces" }, regions{ "regions" }, mapmodes{ "mapmodes" } {}

return_t Map::add_province(std::string const& identifier, Province::colour_t colour) {
	if (provinces.get_item_count() >= Province::MAX_INDEX) {
		Logger::error("The map's province list is full - there can be at most ", Province::MAX_INDEX, " provinces");
		return FAILURE;
	}
	if (identifier.empty()) {
		Logger::error("Invalid province identifier - empty!");
		return FAILURE;
	}
	if (colour == Province::NULL_COLOUR || colour > Province::MAX_COLOUR) {
		Logger::error("Invalid province colour: ", Province::colour_to_hex_string(colour));
		return FAILURE;
	}
	Province new_province{ static_cast<Province::index_t>(provinces.get_item_count() + 1), identifier, colour };
	Province const* old_province = get_province_by_colour(colour);
	if (old_province != nullptr) {
		Logger::error("Duplicate province colours: ", old_province->to_string(), " and ", new_province.to_string());
		return FAILURE;
	}
	return provinces.add_item(std::move(new_province));
}

void Map::lock_provinces() {
	provinces.lock();
}

return_t Map::set_water_province(std::string const& identifier) {
	if (water_provinces_locked) {
		Logger::error("The map's water provinces have already been locked!");
		return FAILURE;
	}
	Province* province = get_province_by_identifier(identifier);
	if (province == nullptr) {
		Logger::error("Unrecognised water province identifier: ", identifier);
		return FAILURE;
	}
	if (province->is_water()) {
		Logger::error("Province ", identifier, " is already a water province!");
		return FAILURE;
	}
	province->water = true;
	water_province_count++;
	return SUCCESS;
}

void Map::lock_water_provinces() {
	water_provinces_locked = true;
	Logger::info("Locked water provinces after registering ", water_province_count);
}

return_t Map::add_region(std::string const& identifier, std::vector<std::string> const& province_identifiers) {
	if (identifier.empty()) {
		Logger::error("Invalid region identifier - empty!");
		return FAILURE;
	}
	Region new_region{ identifier };
	return_t ret = SUCCESS;
	for (std::string const& province_identifier : province_identifiers) {
		Province* province = get_province_by_identifier(province_identifier);
		if (province != nullptr) {
			if (new_region.contains_province(province)) {
				Logger::error("Duplicate province identifier ", province_identifier);
				ret = FAILURE;
			} else {
				size_t other_region_index = reinterpret_cast<size_t>(province->get_region());
				if (other_region_index != 0) {
					other_region_index--;
					if (other_region_index < regions.get_item_count())
						Logger::error("Province ", province_identifier, " is already part of ", regions.get_item_by_index(other_region_index)->get_identifier());
					else
						Logger::error("Province ", province_identifier, " is already part of an unknown region with index ", other_region_index);
					ret = FAILURE;
				} else new_region.provinces.insert(province);
			}
		} else {
			Logger::error("Invalid province identifier ", province_identifier);
			ret = FAILURE;
		}
	}
	if (!new_region.get_province_count()) {
		Logger::error("No valid provinces in region's list");
		return FAILURE;
	}

	// Used to detect provinces listed in multiple regions, will
	// be corrected once regions is stable (i.e. lock_regions).
	Region* tmp_region_index = reinterpret_cast<Region*>(regions.get_item_count());
	for (Province* province : new_region.get_provinces())
		province->region = tmp_region_index;
	if (regions.add_item(std::move(new_region)) != SUCCESS) ret = FAILURE;
	return ret;
}

void Map::lock_regions() {
	regions.lock();
	for (Region& region : regions.get_items())
		for (Province* province : region.get_provinces())
			province->region = &region;
}

size_t Map::get_province_count() const {
	return provinces.get_item_count();
}

Province* Map::get_province_by_index(Province::index_t index) {
	return index != Province::NULL_INDEX ? provinces.get_item_by_index(index - 1) : nullptr;
}

Province const* Map::get_province_by_index(Province::index_t index) const {
	return index != Province::NULL_INDEX ? provinces.get_item_by_index(index - 1) : nullptr;
}

Province* Map::get_province_by_identifier(std::string const& identifier) {
	return provinces.get_item_by_identifier(identifier);
}

Province const* Map::get_province_by_identifier(std::string const& identifier) const {
	return provinces.get_item_by_identifier(identifier);
}

Province* Map::get_province_by_colour(Province::colour_t colour) {
	if (colour != Province::NULL_COLOUR)
		for (Province& province : provinces.get_items())
			if (province.get_colour() == colour) return &province;
	return nullptr;
}

Province const* Map::get_province_by_colour(Province::colour_t colour) const {
	if (colour != Province::NULL_COLOUR)
		for (Province const& province : provinces.get_items())
			if (province.get_colour() == colour) return &province;
	return nullptr;
}

Province::index_t Map::get_province_index_at(size_t x, size_t y) const {
	if (x < width && y < height) return province_shape_image[x + y * width].index;
	return Province::NULL_INDEX;
}

Region* Map::get_region_by_identifier(std::string const& identifier) {
	return regions.get_item_by_identifier(identifier);
}

Region const* Map::get_region_by_identifier(std::string const& identifier) const {
	return regions.get_item_by_identifier(identifier);
}

static Province::colour_t colour_at(uint8_t const* colour_data, int32_t idx) {
	return (colour_data[idx * 3] << 16) | (colour_data[idx * 3 + 1] << 8) | colour_data[idx * 3 + 2];
}

return_t Map::generate_province_shape_image(size_t new_width, size_t new_height, uint8_t const* colour_data) {
	if (!province_shape_image.empty()) {
		Logger::error("Province index image has already been generated!");
		return FAILURE;
	}
	if (!provinces.is_locked()) {
		Logger::error("Province index image cannot be generated until after provinces are locked!");
		return FAILURE;
	}
	if (new_width < 1 || new_height < 1) {
		Logger::error("Invalid province image dimensions: ", new_width, "x", new_height);
		return FAILURE;
	}
	if (colour_data == nullptr) {
		Logger::error("Province colour data pointer is null!");
		return FAILURE;
	}
	width = new_width;
	height = new_height;
	province_shape_image.resize(width * height);

	std::vector<bool> province_checklist(provinces.get_item_count());
	return_t ret = SUCCESS;
	std::unordered_set<Province::colour_t> unrecognised_colours;

	for (int32_t y = 0; y < height; ++y) {
		for (int32_t x = 0; x < width; ++x) {
			const int32_t idx = x + y * width;
			const Province::colour_t colour = colour_at(colour_data, idx);
			if (x > 0) {
				const int32_t jdx = idx - 1;
				if (colour_at(colour_data, jdx) == colour) {
					province_shape_image[idx] = province_shape_image[jdx];
					continue;
				}
			}
			if (y > 0) {
				const int32_t jdx = idx - width;
				if (colour_at(colour_data, jdx) == colour) {
					province_shape_image[idx] = province_shape_image[jdx];
					continue;
				}
			}
			Province const* province = get_province_by_colour(colour);
			if (province != nullptr) {
				const Province::index_t index = province->get_index();
				province_checklist[index - 1] = true;
				province_shape_image[idx].index = index;
				province_shape_image[idx].terrain = !province->is_water();
				continue;
			}
			if (unrecognised_colours.find(colour) == unrecognised_colours.end()) {
				unrecognised_colours.insert(colour);
				Logger::error("Unrecognised province colour ", Province::colour_to_hex_string(colour), " at (", x, ", ", y, ")");
				ret = FAILURE;
			}
			province_shape_image[idx].index = Province::NULL_INDEX;
			province_shape_image[idx].terrain = 0;
		}
	}

	for (size_t idx = 0; idx < province_checklist.size(); ++idx) {
		if (!province_checklist[idx]) {
			Logger::error("Province missing from shape image: ", provinces.get_item_by_index(idx)->to_string());
			ret = FAILURE;
		}
	}
	return ret;
}

size_t Map::get_width() const {
	return width;
}

size_t Map::get_height() const {
	return height;
}

std::vector<Map::shape_pixel_t> const& Map::get_province_shape_image() const {
	return province_shape_image;
}

return_t Map::add_mapmode(std::string const& identifier, Mapmode::colour_func_t colour_func) {
	if (identifier.empty()) {
		Logger::error("Invalid mapmode identifier - empty!");
		return FAILURE;
	}
	if (colour_func == nullptr) {
		Logger::error("Mapmode colour function is null for identifier: ", identifier);
		return FAILURE;
	}
	return mapmodes.add_item({ mapmodes.get_item_count(), identifier, colour_func });
}

void Map::lock_mapmodes() {
	mapmodes.lock();
}

size_t Map::get_mapmode_count() const {
	return mapmodes.get_item_count();
}

Mapmode const* Map::get_mapmode_by_index(size_t index) const {
	return mapmodes.get_item_by_index(index);
}

Mapmode const* Map::get_mapmode_by_identifier(std::string const& identifier) const {
	return mapmodes.get_item_by_identifier(identifier);
}

return_t Map::generate_mapmode_colours(Mapmode::index_t index, uint8_t* target) const {
	if (target == nullptr) {
		Logger::error("Mapmode colour target pointer is null!");
		return FAILURE;
	}
	Mapmode const* mapmode = mapmodes.get_item_by_index(index);
	if (mapmode == nullptr) {
		Logger::error("Invalid mapmode index: ", index);
		return FAILURE;
	}
	// Skip past Province::NULL_INDEX
	for (size_t i = 0; i < MAPMODE_COLOUR_SIZE; ++i)
		*target++ = 0;
	for (Province const& province : provinces.get_items()) {
		const Province::colour_t colour = mapmode->get_colour(*this, province);
		*target++ = (colour >> 16) & 0xFF;
		*target++ = (colour >> 8) & 0xFF;
		*target++ = colour & 0xFF;
	}
	return SUCCESS;
}

return_t Map::generate_province_buildings(BuildingManager const& manager) {
	return_t ret = SUCCESS;
	for (Province& province : provinces.get_items())
		if (manager.generate_province_buildings(province) != SUCCESS) ret = FAILURE;
	return ret;
}

void Map::update_state(Date const& today) {
	for (Province& province : provinces.get_items())
		province.update_state(today);
}

void Map::tick(Date const& today) {
	for (Province& province : provinces.get_items())
		province.tick(today);
}
