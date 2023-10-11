#pragma once

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/texture2d_array.hpp>

#include <openvic-simulation/GameManager.hpp>
#include <openvic-simulation/dataloader/Dataloader.hpp>

namespace OpenVic {

	class GameSingleton : public godot::Object {
		GDCLASS(GameSingleton, godot::Object)

		static GameSingleton* singleton;

		GameManager game_manager;
		Dataloader dataloader;

		godot::Vector2i image_subdivisions;
		godot::Ref<godot::Texture2DArray> province_shape_texture;
		godot::Ref<godot::Image> province_colour_image;
		godot::Ref<godot::ImageTexture> province_colour_texture;
		Mapmode::index_t mapmode_index = 0;
		godot::Ref<godot::Texture2DArray> terrain_texture;

		godot::Error _generate_terrain_texture_array();
		godot::Error _load_map_images(bool flip_vertical = false);

		godot::Error _load_terrain_variants_compatibility_mode(godot::String const& terrain_texturesheet_path);

		/* Generate the province_colour_texture from the current mapmode.
		 */
		godot::Error _update_colour_image();
		void _on_state_updated();

		godot::Dictionary _distribution_to_dictionary(distribution_t const& dist) const;

	protected:
		static void _bind_methods();

	public:
		static void draw_pie_chart(godot::Ref<godot::Image> image,
			godot::Array const& stopAngles, godot::Array const& colours, float radius,
			godot::Vector2 shadow_displacement, float shadow_tightness, float shadow_radius, float shadow_thickness,
			godot::Color trim_colour, float trim_size, float gradient_falloff, float gradient_base,
			bool donut, bool donut_inner_trim, float donut_inner_radius);

		static godot::Ref<godot::Image> load_image(godot::String const& path);

		static GameSingleton* get_singleton();

		GameSingleton();
		~GameSingleton();

		static void setup_logger();

		/* Load the game's defines in compatiblity mode from the filepath
		 * pointing to the defines folder.
		 */
		godot::Error load_defines_compatibility_mode(godot::PackedStringArray const& file_paths);

		static godot::String search_for_game_path(godot::String hint_path = {});

		godot::String lookup_file(godot::String const& path) const;

		/* Post-load/restart game setup - reset the game to post-load state
		 * and (re)generate starting data, e.g. buildings.
		 */
		godot::Error setup_game();

		int32_t get_province_index_from_uv_coords(godot::Vector2 const& coords) const;

		static godot::StringName const& get_province_info_province_key();
		static godot::StringName const& get_province_info_region_key();
		static godot::StringName const& get_province_info_life_rating_key();
		static godot::StringName const& get_province_info_terrain_type_key();
		static godot::StringName const& get_province_info_total_population_key();
		static godot::StringName const& get_province_info_pop_types_key();
		static godot::StringName const& get_province_info_pop_ideologies_key();
		static godot::StringName const& get_province_info_pop_cultures_key();
		static godot::StringName const& get_province_info_rgo_key();
		static godot::StringName const& get_province_info_buildings_key();

		static godot::StringName const& get_building_info_building_key();
		static godot::StringName const& get_building_info_level_key();
		static godot::StringName const& get_building_info_expansion_state_key();
		static godot::StringName const& get_building_info_start_date_key();
		static godot::StringName const& get_building_info_end_date_key();
		static godot::StringName const& get_building_info_expansion_progress_key();

		static godot::StringName const& get_piechart_info_size_key();
		static godot::StringName const& get_piechart_info_colour_key();

		/* Get info to display in Province Overview Panel, packaged in
		 * a Dictionary using the StringNames above as keys.
		 */
		godot::Dictionary get_province_info_from_index(int32_t index) const;

		int32_t get_width() const;
		int32_t get_height() const;
		float get_aspect_ratio() const;

		/* The cosmetic terrain textures stored in a Texture2DArray.
		 */
		godot::Ref<godot::Texture> get_terrain_texture() const;

		/* Number of (vertical, horizontal) subdivisions the province shape image
		 * was split into when making the province_shape_texture to ensure no
		 * piece had a dimension greater than 16383.
		 */
		godot::Vector2i get_province_shape_image_subdivisions() const;

		/* The map, encoded in RGB8 with RG representing province index and B representing terrain texture.
		 * To support a wider range of GPUs, the image is divided so that no piece has a dimension
		 * greater than 16383 and the pieces are stored in a Texture2DArray.
		 */
		godot::Ref<godot::Texture> get_province_shape_texture() const;

		/* The colour each province should be tinted, arranged in
		 * index order into a 256x256 RGB8 texture.
		 */
		godot::Ref<godot::Texture> get_province_colour_texture() const;

		int32_t get_mapmode_count() const;
		godot::String get_mapmode_identifier(int32_t index) const;
		godot::Error set_mapmode(godot::String const& identifier);
		int32_t get_selected_province_index() const;
		void set_selected_province(int32_t index);

		godot::Error expand_building(int32_t province_index, godot::String const& building_type_identifier);

		void set_paused(bool paused);
		void toggle_paused();
		bool is_paused() const;
		void increase_speed();
		void decrease_speed();
		bool can_increase_speed() const;
		bool can_decrease_speed() const;
		godot::String get_longform_date() const;
		void try_tick();
	};
}
