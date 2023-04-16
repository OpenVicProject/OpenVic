#pragma once

#include <functional>

#include <godot_cpp/classes/image.hpp>

#include "openvic2/Map.hpp"

namespace OpenVic2 {
	class MapSingleton : public godot::Object {
		using parse_json_entry_func_t = std::function<godot::Error (godot::String const&, godot::Variant const&)>;

		GDCLASS(MapSingleton, godot::Object)

		static MapSingleton* singleton;

		godot::Ref<godot::Image> provinceIndexImage, provinceColourImage;
		Map map;
		Mapmode::index_t mapmodeIndex = 0;

		godot::Error parseJsonDictionaryFile(godot::String const& fileDescription, godot::String const& filePath,
			godot::String const& identifierPrefix, parse_json_entry_func_t parseEntry) const;
		godot::Error _parseProvinceIdentifierEntry(godot::String const& identifier, godot::Variant const& entry);
		godot::Error _parseRegionEntry(godot::String const& identifier, godot::Variant const& entry);
	protected:
		static void _bind_methods();

	public:
		static MapSingleton* get_singleton();

		MapSingleton();
		~MapSingleton();

		godot::Error loadProvinceIdentifierFile(godot::String const& filePath);
		godot::Error loadRegionFile(godot::String const& filePath);
		godot::Error loadProvinceShapeFile(godot::String const& filePath);

		Province* getProvinceFromUVCoords(godot::Vector2 const& coords);
		Province const* getProvinceFromUVCoords(godot::Vector2 const& coords) const;
		int32_t getProvinceIndexFromUVCoords(godot::Vector2 const& coords) const;
		godot::String getProvinceIdentifierFromUVCoords(godot::Vector2 const& coords) const;
		int32_t getWidth() const;
		int32_t getHeight() const;
		godot::Ref<godot::Image> getProvinceIndexImage() const;
		godot::Ref<godot::Image> getProvinceColourImage() const;

		godot::Error updateColourImage();
		int32_t getMapmodeCount() const;
		godot::String getMapmodeIdentifier(int32_t index) const;
		godot::Error setMapmode(godot::String const& identifier);
	};
}
