#include "Map.hpp"

#include <cassert>

using namespace OpenVic2;

static const std::string SEPARATOR = "\n - ";

Mapmode::Mapmode(index_t new_index, std::string const& new_identifier, colour_func_t new_colour_func)
	: index(new_index), identifier(new_identifier), colour_func(new_colour_func) {
	assert(!identifier.empty());
	assert(colour_func != nullptr);
}

Mapmode::index_t Mapmode::get_index() const {
	return index;
}

std::string const& Mapmode::get_identifier() const {
	return identifier;
}

Mapmode::colour_func_t Mapmode::get_colour_func() const {
	return colour_func;
}

return_t Map::add_province(std::string const& identifier, Province::colour_t colour, std::string& error_message) {
	if (provinces_locked) {
		error_message = "The map's province list has already been locked!";
		return FAILURE;
	}
	if (provinces.size() >= Province::MAX_INDEX) {
		error_message = "The map's province list is full - there can be at most " + std::to_string(Province::MAX_INDEX) + " provinces";
		return FAILURE;
	}
	if (identifier.empty()) {
		error_message = "Empty province identifier for colour " + Province::colour_to_hex_string(colour);
		return FAILURE;
	}
	if (colour == Province::NULL_COLOUR || colour > Province::MAX_COLOUR) {
		error_message = "Invalid province colour: " + Province::colour_to_hex_string(colour);
		return FAILURE;
	}
	Province new_province{ static_cast<Province::index_t>(provinces.size() + 1), identifier, colour };
	for (Province const& province : provinces) {
		if (province.get_identifier() == identifier) {
			error_message = "Duplicate province identifiers: " + province.to_string() + " and " + new_province.to_string();
			return FAILURE;
		}
		if (province.get_colour() == colour) {
			error_message = "Duplicate province colours: " + province.to_string() + " and " + new_province.to_string();
			return FAILURE;
		}
	}
	provinces.push_back(new_province);
	error_message = "Added province: " + new_province.to_string();
	return SUCCESS;
}

void Map::lock_provinces() {
	provinces_locked = true;
}

return_t Map::add_region(std::string const& identifier, std::vector<std::string> const& province_identifiers, std::string& error_message) {
	if (regions_locked) {
		error_message = "The map's region list has already been locked!";
		return FAILURE;
	}
	if (identifier.empty()) {
		error_message = "Empty region identifier!";
		return FAILURE;
	}
	if (provinces.empty()) {
		error_message = "Empty province list for region " + identifier;
		return FAILURE;
	}
	Region new_region{ identifier };
	error_message = "Error message for region: " + identifier;
	for (std::string const& province_identifier : province_identifiers) {
		Province* province = get_province_by_identifier(province_identifier);
		if (province != nullptr) {
			if (new_region.contains_province(province))
				error_message += SEPARATOR + "Duplicate province identifier " + province_identifier;
			else {
				size_t other_region_index = reinterpret_cast<size_t>(province->get_region());
				if (other_region_index != 0) {
					other_region_index--;
					error_message += SEPARATOR + "Province " + province_identifier + " is already part of ";
					if (other_region_index < regions.size())
						error_message += regions[other_region_index].get_identifier();
					else
						error_message += "an unknown region with index " + std::to_string(other_region_index);
				} else new_region.provinces.push_back(province);
			}
		} else error_message += SEPARATOR + "Invalid province identifier " + province_identifier;
	}
	if (!new_region.get_province_count()) {
		error_message += SEPARATOR + "No valid provinces in region's list";
		return FAILURE;
	}
	for (Region const& region : regions) {
		if (region.get_identifier() == identifier) {
			error_message += SEPARATOR + "Duplicate region identifiers: " + region.get_identifier() + " and " + identifier;
			return FAILURE;
		}
	}
	regions.push_back(new_region);
	error_message += SEPARATOR + "Added region: " + identifier;
	// Used to detect provinces listed in multiple regions, will
	// be corrected once regions is stable (i.e. lock_regions).
	Region* tmp_region_index = reinterpret_cast<Region*>(regions.size());
	for (Province* province : new_region.get_provinces())
		province->region = tmp_region_index;
	return SUCCESS;
}

void Map::lock_regions() {
	regions_locked = true;
	for (Region& region : regions)
		for (Province* province : region.get_provinces())
			province->region = &region;
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

static Province::colour_t colour_at(uint8_t const* colour_data, int32_t idx) {
	return (colour_data[idx * 3] << 16) | (colour_data[idx * 3 + 1] << 8) | colour_data[idx * 3 + 2];
}

return_t Map::generate_province_index_image(size_t new_width, size_t new_height, uint8_t const* colour_data, std::string& error_message) {
	if (!province_index_image.empty()) {
		error_message = "Province index image has already been generated!";
		return FAILURE;
	}
	if (!provinces_locked) {
		error_message = "Province index image cannot be generated until after provinces are locked!";
		return FAILURE;
	}
	if (new_width < 1 || new_height < 1) {
		error_message = "Invalid province image dimensions: " + std::to_string(new_width) + "x" + std::to_string(new_height);
		return FAILURE;
	}
	if (colour_data == nullptr) {
		error_message = "Province colour data pointer is null!";
		return FAILURE;
	}
	width = new_width;
	height = new_height;
	province_index_image.resize(width * height);

	std::vector<bool> province_checklist(provinces.size());
	return_t ret = SUCCESS;

	error_message = "Error message for province index image generation:";

	for (int32_t y = 0; y < height; ++y) {
		for (int32_t x = 0; x < width; ++x) {
			const int32_t idx = x + y * width;
			const Province::colour_t colour = colour_at(colour_data, idx);
			if (colour == Province::NULL_COLOUR) {
				province_index_image[idx] = Province::NULL_INDEX;
				continue;
			}
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
			error_message += SEPARATOR + "Unrecognised province colour " + Province::colour_to_hex_string(colour) + " at (" + std::to_string(x) + ", " + std::to_string(x) + ")";
			ret = FAILURE;
			province_index_image[idx] = Province::NULL_INDEX;
		}
	}

	for (size_t idx = 0; idx < province_checklist.size(); ++idx) {
		if (!province_checklist[idx]) {
			error_message += SEPARATOR + "Province missing from shape image: " + provinces[idx].to_string();
			ret = FAILURE;
		}
	}

	error_message += SEPARATOR + "Generated province index image";
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

return_t Map::add_mapmode(std::string const& identifier, Mapmode::colour_func_t colour_func, std::string& error_message) {
	if (identifier.empty()) {
		error_message = "Empty mapmode identifier!";
		return FAILURE;
	}
	if (colour_func == nullptr) {
		error_message = "Mapmode colour function is null for identifier: " + identifier;
		return FAILURE;
	}
	Mapmode new_mapmode{ mapmodes.size(), identifier, colour_func };
	for (Mapmode const& mapmode : mapmodes) {
		if (mapmode.get_identifier() == identifier) {
			error_message = "Duplicate mapmode identifiers: " + mapmode.get_identifier() + " and " + identifier;
			return FAILURE;
		}
	}
	mapmodes.push_back(new_mapmode);
	error_message = "Added mapmode: " + identifier;
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

return_t Map::generate_mapmode_colours(Mapmode::index_t index, uint8_t* target, std::string& error_message) const {
	if (target == nullptr) {
		error_message = "Mapmode colour target pointer is null!";
		return FAILURE;
	}
	if (index >= mapmodes.size()) {
		error_message = "Invalid mapmode index: " + std::to_string(index);
		return FAILURE;
	}
	Mapmode const& mapmode = mapmodes[index];
	target += 3; // Skip past Province::NULL_INDEX
	for (Province const& province : provinces) {
		const Province::colour_t colour = mapmode.get_colour_func()(*this, province);
		*target++ = (colour >> 16) & 0xFF;
		*target++ = (colour >> 8) & 0xFF;
		*target++ = colour & 0xFF;
	}
	return SUCCESS;
}
