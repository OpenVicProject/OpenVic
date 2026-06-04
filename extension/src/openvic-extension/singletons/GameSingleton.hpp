#pragma once

#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/texture2d_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include <openvic-simulation/GameManager.hpp>
#include <openvic-simulation/dataloader/Dataloader.hpp>
#include <openvic-simulation/types/TypedIndices.hpp>

namespace OpenVic {
	struct Ideology;
	struct PartyPolicy;
	struct PopType;
	struct ProvinceDefinition;
	struct RebelType;
	struct Reform;

	class GameSingleton : public godot::Object {
		GDCLASS(GameSingleton, godot::Object)

		static inline GameSingleton* singleton = nullptr;

		GameManager game_manager;

		godot::Vector2i image_subdivisions;
		godot::Ref<godot::Texture2DArray> province_shape_texture;
		godot::Ref<godot::Image> province_colour_image;
		godot::Ref<godot::ImageTexture> province_colour_texture;
		Mapmode const* mapmode; // This should never be null, if no mapmode is set then it'll point to Mapmode::ERROR_MAPMODE
		godot::Ref<godot::Texture2DArray> terrain_texture;

		inline static const godot::Vector2i PROPERTY(flag_dims, { 128, 64 }); /* The size in pixels of an individual flag. */
		int32_t flag_sheet_count = 0; /* The number of flags in the flag sheet. */
		godot::Vector2i flag_sheet_dims; /* The size of the flag sheet in flags, rather than pixels. */
		godot::Ref<godot::Image> flag_sheet_image;
		godot::Ref<godot::ImageTexture> flag_sheet_texture;
		godot::HashMap<godot::StringName, int32_t> flag_type_index_map;

		static godot::StringName const& _signal_gamestate_updated();
		static godot::StringName const& _signal_mapmode_changed();

		godot::Error _load_map_images();
		godot::Error _load_terrain_variants();
		godot::Error _load_flag_sheet();

	protected:
		static void _bind_methods();

	public:
		static GameSingleton* get_singleton();
		signal_property<GameSingleton> gamestate_updated;

		GameSingleton();
		~GameSingleton();

		static void setup_logger();

		inline constexpr Dataloader const& get_dataloader() const {
			return game_manager.get_dataloader();
		}

		inline constexpr DefinitionManager const& get_definition_manager() const {
			return game_manager.get_definition_manager();
		}

		inline constexpr InstanceManager* get_instance_manager() {
			return game_manager.get_instance_manager();
		}

		inline constexpr InstanceManager const* get_instance_manager() const {
			return game_manager.get_instance_manager();
		}

		/* Load the game's defines in compatibility mode from the filepath
		 * pointing to the defines folder. */
		godot::Error set_compatibility_mode_roots(godot::String const& path);
		godot::Error load_defines_compatibility_mode(godot::PackedStringArray const& mods = {});

		static godot::String search_for_game_path(godot::String const& hint_path = {});
		godot::String lookup_file_path(godot::String const& path) const;

		godot::TypedArray<godot::Dictionary> get_mod_info() const;

		godot::TypedArray<godot::Dictionary> get_bookmark_info() const;

		/* After initial load or resigning a previous session game setup, all mutable components of the simulation
		   are reset and reinitialised to their initial states, then updated by the history instructions of the
		   chosen bookmark. */
		godot::Error setup_game(int32_t bookmark_index);
		bool is_game_instance_setup() const;
		bool is_bookmark_loaded() const;

		godot::Error start_game_session();
		godot::Error end_game_session();
		bool is_game_session_active() const;

		int32_t get_province_number_from_uv_coords(godot::Vector2 const& coords) const;

		int32_t get_map_width() const;
		int32_t get_map_height() const;
		godot::Vector2i get_map_dims() const;
		float get_map_aspect_ratio() const;
		godot::Vector2 normalise_map_position(fvec2_t const& position) const;
		godot::Vector2 get_billboard_pos(ProvinceDefinition const& province) const;
		godot::Vector2 get_bookmark_start_position() const;

		/* The cosmetic terrain textures stored in a Texture2DArray. */
		godot::Ref<godot::Texture2DArray> get_terrain_texture() const;

		godot::Ref<godot::Image> get_flag_sheet_image() const;
		godot::Ref<godot::ImageTexture> get_flag_sheet_texture() const;

		/* The index of the flag in the flag sheet corresponding to the requested country / flag_type
		 * combination, or -1 if no such flag can be found. */
		int32_t get_flag_sheet_index(const country_index_t country_index, godot::StringName const& flag_type) const;
		godot::Rect2i get_flag_sheet_rect(int32_t flag_index) const;
		godot::Rect2i get_flag_sheet_rect(const country_index_t country_index, godot::StringName const& flag_type) const;

		/* Number of (vertical, horizontal) subdivisions the province shape image
		 * was split into when making the province_shape_texture to ensure no
		 * piece had a dimension greater than 16383. */
		godot::Vector2i get_province_shape_image_subdivisions() const;

		/* The map, encoded in RGB8 with RG representing province index and B representing terrain texture.
		 * To support a wider range of GPUs, the image is divided so that no piece has a dimension
		 * greater than 16383 and the pieces are stored in a Texture2DArray. */
		godot::Ref<godot::Texture2DArray> get_province_shape_texture() const;

		/* The base and stripe colours for each province. */
		godot::Ref<godot::ImageTexture> get_province_colour_texture() const;

		godot::TypedArray<godot::Dictionary> get_province_names() const;

		int32_t get_mapmode_count() const;
		godot::String get_mapmode_identifier(int32_t index) const;
		godot::String get_mapmode_localisation_key(int32_t index) const;
		int32_t get_current_mapmode_index() const;
		godot::Error set_mapmode(int32_t index);
		bool is_parchment_mapmode_allowed() const;

		godot::Error update_clock();
		/* Generate the province_colour_texture from the current mapmode. */
		godot::Error _update_colour_image();
		void _on_gamestate_updated();

		// TODO: Get rid of these functions later
		TypedSpan<ideology_index_t, const Ideology> get_ideologies() {
			return get_definition_manager().get_politics_manager().get_ideology_manager().get_ideologies();
		}
		Ideology const& get_ideology(const ideology_index_t i) {
			TypedSpan<ideology_index_t, const Ideology> span = get_ideologies();
			CRASH_BAD_INDEX(type_safe::get(i), type_safe::get(span.size()));
			return span[i];
		}

		TypedSpan<party_policy_index_t, const PartyPolicy> get_party_policies() {
			return get_definition_manager().get_politics_manager().get_issue_manager().get_party_policies();
		}
		PartyPolicy const& get_party_policy(const party_policy_index_t i) {
			TypedSpan<party_policy_index_t, const PartyPolicy> span = get_party_policies();
			CRASH_BAD_INDEX(type_safe::get(i), type_safe::get(span.size()));
			return span[i];
		}

		TypedSpan<pop_type_index_t, const PopType> get_pop_types() {
			return get_definition_manager().get_pop_manager().get_pop_types();
		}
		PopType const& get_pop_type(const pop_type_index_t i) {
			TypedSpan<pop_type_index_t, const PopType> span = get_pop_types();
			CRASH_BAD_INDEX(type_safe::get(i), type_safe::get(span.size()));
			return span[i];
		}

		TypedSpan<province_index_t, const ProvinceDefinition> get_province_definitions() {
			return get_definition_manager().get_map_definition().get_province_definitions();
		}
		ProvinceDefinition const& get_province_definition(const province_index_t i) {
			TypedSpan<province_index_t, const ProvinceDefinition> span = get_province_definitions();
			CRASH_BAD_INDEX(type_safe::get(i), type_safe::get(span.size()));
			return span[i];
		}

		TypedSpan<rebel_type_index_t, const RebelType> get_rebel_types() {
			return get_definition_manager().get_politics_manager().get_rebel_manager().get_rebel_types();
		}
		RebelType const& get_rebel_type(const rebel_type_index_t i) {
			TypedSpan<rebel_type_index_t, const RebelType> span = get_rebel_types();
			CRASH_BAD_INDEX(type_safe::get(i), type_safe::get(span.size()));
			return span[i];
		}

		TypedSpan<reform_index_t, const Reform> get_reforms() {
			return get_definition_manager().get_politics_manager().get_issue_manager().get_reforms();
		}
		Reform const& get_reform(const reform_index_t i) {
			TypedSpan<reform_index_t, const Reform> span = get_reforms();
			CRASH_BAD_INDEX(type_safe::get(i), type_safe::get(span.size()));
			return span[i];
		}
	};
}
