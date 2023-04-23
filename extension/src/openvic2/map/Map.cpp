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

Mapmode::colour_func_t Mapmode::get_colour_func() const {
	return colour_func;
}

return_t Map::add_province(std::string const& identifier, Province::colour_t colour) {
	if (provinces_locked) {
		Logger::error("The map's province list has already been locked!");
		return FAILURE;
	}
	if (provinces.size() >= Province::MAX_INDEX) {
		Logger::error("The map's province list is full - there can be at most ", Province::MAX_INDEX, " provinces");
		return FAILURE;
	}
	if (identifier.empty()) {
		Logger::error("Empty province identifier for colour ", Province::colour_to_hex_string(colour));
		return FAILURE;
	}
	if (colour == Province::NULL_COLOUR || colour > Province::MAX_COLOUR) {
		Logger::error("Invalid province colour: ", Province::colour_to_hex_string(colour));
		return FAILURE;
	}
	Province new_province{ static_cast<Province::index_t>(provinces.size() + 1), identifier, colour };
	Province const* old_province = get_province_by_identifier(identifier);
	if (old_province != nullptr) {
		Logger::error("Duplicate province identifiers: ", old_province->to_string(), " and ", new_province.to_string());
		return FAILURE;
	}
	old_province = get_province_by_colour(colour);
	if (old_province != nullptr) {
		Logger::error("Duplicate province colours: ", old_province->to_string(), " and ", new_province.to_string());
		return FAILURE;
	}
	provinces.push_back(new_province);
	return SUCCESS;
}

void Map::lock_provinces() {
	provinces_locked = true;
	Logger::info("Locked provinces after registering ", provinces.size());
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
	if (regions_locked) {
		Logger::error("The map's region list has already been locked!");
		return FAILURE;
	}
	if (identifier.empty()) {
		Logger::error("Empty region identifier!");
		return FAILURE;
	}
	if (provinces.empty()) {
		Logger::error("Empty province list for region ", identifier);
		return FAILURE;
	}
	return_t ret = SUCCESS;
	Region new_region{ identifier };
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
					if (other_region_index < regions.size())
						Logger::error("Province ", province_identifier, " is already part of ", regions[other_region_index].get_identifier());
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
	Region const* old_region = get_region_by_identifier(identifier);
	if (old_region != nullptr) {
		Logger::error("Duplicate region identifiers: ", old_region->get_identifier(), " and ", identifier);
		return FAILURE;
	}

	regions.push_back(new_region);
	// Used to detect provinces listed in multiple regions, will
	// be corrected once regions is stable (i.e. lock_regions).
	Region* tmp_region_index = reinterpret_cast<Region*>(regions.size());
	for (Province* province : new_region.get_provinces())
		province->region = tmp_region_index;
	return ret;
}

void Map::lock_regions() {
	regions_locked = true;
	for (Region& region : regions)
		for (Province* province : region.get_provinces())
			province->region = &region;
	Logger::info("Locked regions after registering ", regions.size());
}

size_t Map::get_province_count() const {
	return provinces.size();
}

Province* Map::get_province_by_index(Province::index_t index) {
	return index != Province::NULL_INDEX && index <= provinces.size() ? &provinces[index - 1] : nullptr;
}

Province const* Map::get_province_by_index(Province::index_t index) const {
	return index != Province::NULL_INDEX && index <= provinces.size() ? &provinces[index - 1] : nullptr;
}

Province* Map::get_province_by_identifier(std::string const& identifier) {
	if (!identifier.empty())
		for (Province& province : provinces)
			if (province.get_identifier() == identifier) return &province;
	return nullptr;
}

Province const* Map::get_province_by_identifier(std::string const& identifier) const {
	if (!identifier.empty())
		for (Province const& province : provinces)
			if (province.get_identifier() == identifier) return &province;
	return nullptr;
}

Province* Map::get_province_by_colour(Province::colour_t colour) {
	if (colour != Province::NULL_COLOUR)
		for (Province& province : provinces)
			if (province.get_colour() == colour) return &province;
	return nullptr;
}

Province const* Map::get_province_by_colour(Province::colour_t colour) const {
	if (colour != Province::NULL_COLOUR)
		for (Province const& province : provinces)
			if (province.get_colour() == colour) return &province;
	return nullptr;
}

Province::index_t Map::get_province_index_at(size_t x, size_t y) const {
	if (x < width && y < height) return province_index_image[x + y * width];
	return Province::NULL_INDEX;
}

Region* Map::get_region_by_identifier(std::string const& identifier) {
	if (!identifier.empty())
		for (Region& region : regions)
			if (region.get_identifier() == identifier) return &region;
	return nullptr;
}

Region const* Map::get_region_by_identifier(std::string const& identifier) const {
	if (!identifier.empty())
		for (Region const& region : regions)
			if (region.get_identifier() == identifier) return &region;
	return nullptr;
}

static Province::colour_t colour_at(uint8_t const* colour_data, int32_t idx) {
	return (colour_data[idx * 3] << 16) | (colour_data[idx * 3 + 1] << 8) | colour_data[idx * 3 + 2];
}

return_t Map::generate_province_index_image(size_t new_width, size_t new_height, uint8_t const* colour_data) {
	if (!province_index_image.empty()) {
		Logger::error("Province index image has already been generated!");
		return FAILURE;
	}
	if (!provinces_locked) {
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
	province_index_image.resize(width * height);

	std::vector<bool> province_checklist(provinces.size());
	return_t ret = SUCCESS;
	std::unordered_set<Province::colour_t> unrecognised_colours;

	for (int32_t y = 0; y < height; ++y) {
		for (int32_t x = 0; x < width; ++x) {
			const int32_t idx = x + y * width;
			const Province::colour_t colour = colour_at(colour_data, idx);
			if (x > 0) {
				const int32_t jdx = idx - 1;
				if (colour_at(colour_data, jdx) == colour) {
					province_index_image[idx] = province_index_image[jdx];
					continue;
				}
			}
			if (y > 0) {
				const int32_t jdx = idx - width;
				if (colour_at(colour_data, jdx) == colour) {
					province_index_image[idx] = province_index_image[jdx];
					continue;
				}
			}
			Province const* province = get_province_by_colour(colour);
			if (province != nullptr) {
				const Province::index_t index = province->get_index();
				province_index_image[idx] = index;
				province_checklist[index - 1] = true;
				continue;
			}
			if (unrecognised_colours.find(colour) == unrecognised_colours.end()) {
				unrecognised_colours.insert(colour);
				Logger::error("Unrecognised province colour ", Province::colour_to_hex_string(colour), " at (", x, ", ", y, ")");
				ret = FAILURE;
			}
			province_index_image[idx] = Province::NULL_INDEX;
		}
	}

	for (size_t idx = 0; idx < province_checklist.size(); ++idx) {
		if (!province_checklist[idx]) {
			Logger::error("Province missing from shape image: ", provinces[idx].to_string());
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

std::vector<Province::index_t> const& Map::get_province_index_image() const {
	return province_index_image;
}

return_t Map::add_mapmode(std::string const& identifier, Mapmode::colour_func_t colour_func) {
	if (identifier.empty()) {
		Logger::error("Empty mapmode identifier!");
		return FAILURE;
	}
	if (colour_func == nullptr) {
		Logger::error("Mapmode colour function is null for identifier: ", identifier);
		return FAILURE;
	}
	Mapmode new_mapmode{ mapmodes.size(), identifier, colour_func };
	Mapmode const* old_mapmode = get_mapmode_by_identifier(identifier);
	if (old_mapmode != nullptr) {
		Logger::error("Duplicate mapmode identifiers: ", old_mapmode->get_identifier(), " and ", identifier);
		return FAILURE;
	}
	mapmodes.push_back(new_mapmode);
	return SUCCESS;
}

size_t Map::get_mapmode_count() const {
	return mapmodes.size();
}

Mapmode const* Map::get_mapmode_by_index(size_t index) const {
	return index < mapmodes.size() ? &mapmodes[index] : nullptr;
}

Mapmode const* Map::get_mapmode_by_identifier(std::string const& identifier) const {
	if (!identifier.empty())
		for (Mapmode const& mapmode : mapmodes)
			if (mapmode.get_identifier() == identifier) return &mapmode;
	return nullptr;
}

return_t Map::generate_mapmode_colours(Mapmode::index_t index, uint8_t* target) const {
	if (target == nullptr) {
		Logger::error("Mapmode colour target pointer is null!");
		return FAILURE;
	}
	if (index >= mapmodes.size()) {
		Logger::error("Invalid mapmode index: ", index);
		return FAILURE;
	}
	Mapmode const& mapmode = mapmodes[index];
	target += 4; // Skip past Province::NULL_INDEX
	for (Province const& province : provinces) {
		const Province::colour_t colour = mapmode.get_colour_func()(*this, province);
		*target++ = (colour >> 16) & 0xFF;
		*target++ = (colour >> 8) & 0xFF;
		*target++ = colour & 0xFF;
		*target++ = province.is_water() ? 0 : 255;
	}
	return SUCCESS;
}

void Map::generate_province_buildings(BuildingManager const& manager) {
	for (Province& province : provinces)
		manager.generate_province_buildings(province.buildings);
}

void Map::update_state(Date const& today) {
	for (Province& province : provinces)
		province.update_state(today);
}

void Map::tick(Date const& today) {
	for (Province& province : provinces)
		province.tick(today);
}
