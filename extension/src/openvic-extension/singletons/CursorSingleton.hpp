#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <openvic-simulation/dataloader/Dataloader.hpp>
#include <openvic-extension/singletons/GameSingleton.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>
#include <openvic-simulation/types/IdentifierRegistry.hpp>

namespace OpenVic {

	class CursorSingleton : public godot::Object {
		
		/*
		This singleton only handles the dataloading for the cursors.
		The functionality for actually setting one of these as a cursor
		and animating animated cursors is done by the CursorManager autoloaded
		gd script. With the exception of the shapeMap and its functions??
		*/

		GDCLASS(CursorSingleton, godot::Object);
		static inline CursorSingleton* _singleton = nullptr;

		//an intermediate data type to help with loading cursors
		//the size of images/hotspots corresponds to resolutionsPerCursor
		struct image_hotspot_pair_asset_t {
			std::vector<godot::Vector2i> hotspots;
			std::vector<godot::Ref<godot::ImageTexture>> images;
		};

		//.cur files use all but the last 2 properties, rest are for .ani
		//use vector2i instead of ref<vector2i> because of "unreference is not a member of godot::vector2i"
		//images is an array of ImageTexture, just godot doesn't like typed arrays with image texture for some reason
		struct cursor_asset_t {
			std::vector<godot::TypedArray<godot::Vector2i>> hotspots;
			std::vector<godot::Array> images;
			godot::TypedArray<godot::Vector2i> resolutions;
			int animationLength; //1 for static cursors
			std::optional<godot::TypedArray<float>> displayRates;
			std::optional<godot::TypedArray<int>> sequence;
		};

		//map of "subfolder/fileName.cur/.ani" -> cursor asset, subfolder comes after gfx/cursor
		using cursor_map_t = deque_ordered_map<godot::StringName, cursor_asset_t>;
		cursor_map_t cursors;

		godot::Array PROPERTY(cursor_names);

	public:
		CursorSingleton();
		~CursorSingleton();
		static CursorSingleton* get_singleton();
	
	protected:
		static void _bind_methods();

		godot::String to_define_file_name(godot::String const& path) const;
		godot::String read_riff_str(godot::Ref<godot::FileAccess> const& file, int size=4) const;

	private:
		//helper functions for the loaders
		bool _read_AND_mask(godot::PackedByteArray const& data, godot::Vector2i pixelCoords, godot::Vector2i dimensions, int offset);
		godot::PackedByteArray _pixel_palette_lookup(godot::PackedByteArray const& data, godot::PackedByteArray const& palette, godot::Vector2i coord, godot::Vector2i dimensions, int offset, bool transparent, int bitsPerPixel);
		godot::PackedByteArray _read_32bit_pixel(int i, int offset, godot::PackedByteArray const& imageData, bool notTransparent);
		godot::PackedByteArray _read_24bit_pixel(int i, int offset, godot::PackedByteArray const& imageData, bool notTransparent);

		int _get_row_start(godot::Vector2i dimensions, int y_coord, int bitsPerPixel);
		int _select_bits(godot::PackedByteArray data, int rowStart, int firstBit, int bitCount);
		int _reverse_bits(int byte, int bitsPerPixel=8);
		int _rotate_right(int byte, int size=8);
		int _load_int_256(godot::Ref<godot::FileAccess> const& file);

		//primary loading functions
		image_hotspot_pair_asset_t _load_pair(godot::Ref<godot::FileAccess> const& file);
		bool _load_cursor_ani(godot::String const& name, godot::String const& path);
		bool _load_cursor_cur(godot::String const& name, godot::String const& path);

	public:
		bool load_cursors();
		
		//getters for the different cursor properties
		godot::Array get_frames(godot::String const& name, int res_index = 0);
		godot::TypedArray<godot::Vector2i> get_hotspots(godot::String const& name, int res_index = 0);
		int get_animationLength(godot::String const& name);
		godot::TypedArray<godot::Vector2i> get_resolutions(godot::String const& name);
		godot::TypedArray<float> get_displayRates(godot::String const& name);
		godot::TypedArray<int> get_sequence(godot::String const& name);
		void generate_resolution(godot::String const& name, int base_res_index, godot::Vector2i target_res);

	};

}