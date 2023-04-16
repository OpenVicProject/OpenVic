#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <functional>

#include "Types.hpp"

namespace OpenVic2 {
	struct Region;
	struct Map;

	/* REQUIREMENTS:
	 * MAP-43, MAP-47
	 */
	struct Province {
		friend struct Map;

		using colour_t = uint32_t;
		using index_t = uint16_t;

		static const colour_t NULL_COLOUR = 0, MAX_COLOUR = 0xFFFFFF;
		static const index_t NULL_INDEX = 0, MAX_INDEX = 0xFFFF;
	private:
		index_t index;
		std::string identifier;
		colour_t colour;
		Region* region = nullptr;

		Province(index_t newIndex, std::string const& newIdentifier, colour_t newColour);
	public:
		static std::string colourToHexString(colour_t colour);

		index_t getIndex() const;
		std::string const& getIdentifier() const;
		colour_t getColour() const;
		Region* getRegion() const;
		std::string toString() const;
	};

	/* REQUIREMENTS:
	 * MAP-6, MAP-44, MAP-48
	 */
	struct Region {
		friend struct Map;
	private:
		std::string identifier;
		std::vector<Province*> provinces;

		Region(std::string const& newIdentifier);
	public:
		std::string const& getIdentifier() const;
		size_t getProvinceCount() const;
		bool containsProvince(Province const* province) const;
		std::vector<Province*> const& getProvinces() const;
	};

	struct Mapmode {
		friend struct Map;

		using colour_func_t = std::function<Province::colour_t (Map const&, Province const&)>;
		using index_t = size_t;
	private:
		index_t index;
		std::string identifier;
		colour_func_t colourFunc;

		Mapmode(index_t newIndex, std::string const& newIdentifier, colour_func_t newColourFunc);
	public:
		index_t getIndex() const;
		std::string const& getIdentifier() const;
		colour_func_t getColourFunc() const;
	};

	/* REQUIREMENTS:
	 * MAP-4
	 */
	struct Map {
	private:
		std::vector<Province> provinces;
		std::vector<Region> regions;
		bool provinces_locked = false, regions_locked = false;

		size_t width = 0, height = 0;
		std::vector<Province::index_t> province_index_image;
		std::vector<Mapmode> mapmodes;
	public:
		return_t addProvince(std::string const& identifier, Province::colour_t colour, std::string& error_message);
		void lockProvinces();
		return_t addRegion(std::string const& identifier, std::vector<std::string> const& province_identifiers, std::string& error_message);
		void lockRegions();
		size_t getProvinceCount() const;

		Province* getProvinceByIndex(Province::index_t index);
		Province const* getProvinceByIndex(Province::index_t index) const;
		Province* getProvinceByIdentifier(std::string const& identifier);
		Province const* getProvinceByIdentifier(std::string const& identifier) const;
		Province* getProvinceByColour(Province::colour_t colour);
		Province const* getProvinceByColour(Province::colour_t colour) const;

		return_t generateProvinceIndexImage(size_t new_width, size_t new_height, uint8_t const* colour_data, std::string& error_message);
		size_t getWidth() const;
		size_t getHeight() const;
		std::vector<Province::index_t> const& getProvinceIndexImage() const;

		return_t addMapmode(std::string const& identifier, Mapmode::colour_func_t colour_func, std::string& error_message);
		size_t getMapmodeCount() const;
		Mapmode const* getMapmodeByIndex(Mapmode::index_t index) const;
		Mapmode const* getMapmodeByIdentifier(std::string const& identifier) const;
		return_t generateMapmodeColours(Mapmode::index_t index, uint8_t* target, std::string& error_message) const;
	};
}
