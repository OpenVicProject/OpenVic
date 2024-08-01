#pragma once

#include <godot_cpp/classes/image.hpp>

#include <openvic-simulation/pop/Pop.hpp>
#include <openvic-simulation/types/IndexedMap.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>

namespace OpenVic {
	struct CountryInstance;
	struct State;
	struct ProvinceInstance;

	class MenuSingleton : public godot::Object {
		GDCLASS(MenuSingleton, godot::Object)

		static inline MenuSingleton* singleton = nullptr;

	public:
		struct population_menu_t {
			enum ProvinceListEntry {
				LIST_ENTRY_NONE, LIST_ENTRY_COUNTRY, LIST_ENTRY_STATE, LIST_ENTRY_PROVINCE
			};

			struct country_entry_t {
				CountryInstance const& country;
				bool selected = true;
			};

			struct state_entry_t {
				State const& state;
				bool selected = true, expanded = false;
			};

			struct province_entry_t {
				ProvinceInstance const& province;
				bool selected = true;
			};

			using province_list_entry_t = std::variant<country_entry_t, state_entry_t, province_entry_t>;

			std::vector<province_list_entry_t> province_list_entries;
			int32_t visible_province_list_entries = 0;

			struct pop_filter_t {
				Pop::pop_size_t count, promotion_demotion_change;
				bool selected;
			};
			ordered_map<PopType const*, pop_filter_t> pop_filters;

			static constexpr int32_t DISTRIBUTION_COUNT = 6;
			/* Distributions:
			 *  - Workforce (PopType)
			 *  - Religion
			 *  - Ideology
			 *  - Nationality (Culture)
			 *  - Issues
			 *  - Vote */
			std::array<fixed_point_map_t<HasIdentifierAndColour const*>, DISTRIBUTION_COUNT> distributions;

			enum PopSortKey {
				NONE, SORT_SIZE, SORT_TYPE, SORT_CULTURE, SORT_RELIGION, SORT_LOCATION, SORT_MILITANCY, SORT_CONSCIOUSNESS,
				SORT_IDEOLOGY, SORT_ISSUES, SORT_UNEMPLOYMENT, SORT_CASH, SORT_LIFE_NEEDS, SORT_EVERYDAY_NEEDS,
				SORT_LUXURY_NEEDS, SORT_REBEL_FACTION, SORT_SIZE_CHANGE, SORT_LITERACY, MAX_SORT_KEY
			} sort_key = NONE;
			bool sort_descending = true;
			IndexedMap<PopType, size_t> pop_type_sort_cache;
			IndexedMap<Culture, size_t> culture_sort_cache;
			IndexedMap<Religion, size_t> religion_sort_cache;
			IndexedMap<ProvinceInstance, size_t> province_sort_cache;
			IndexedMap<RebelType, size_t> rebel_type_sort_cache;

			std::vector<Pop const*> pops, filtered_pops;
		};

		struct search_panel_t {
			struct entry_t {
				std::variant<ProvinceInstance const*, State const*, CountryInstance const*> target;
				godot::String display_name, search_name, identifier;
			};
			std::vector<entry_t> entry_cache;
			std::vector<size_t> result_indices;
		};

	private:
		population_menu_t population_menu;
		search_panel_t search_panel;

		/* Emitted when the number of visible province list rows changes (list generated or state entry expanded).*/
		static godot::StringName const& _signal_population_menu_province_list_changed();
		/* Emitted when the state of visible province list rows changes (selection changes). Provides an integer argument
		 * which, if not negative, the province list scroll index should be updated to. */
		static godot::StringName const& _signal_population_menu_province_list_selected_changed();
		/* Emitted when the selected/filtered collection of pops changes. */
		static godot::StringName const& _signal_population_menu_pops_changed();
		/* Emitted when the collection of possible search results changes. */
		static godot::StringName const& _signal_search_cache_changed();

		godot::String get_state_name(State const& state) const;
		godot::String get_country_name(CountryInstance const& country) const;
		godot::String get_country_adjective(CountryInstance const& country) const;

	protected:
		static void _bind_methods();

	public:
		static MenuSingleton* get_singleton();

		/* This should only be called AFTER GameSingleton has been initialised! */
		MenuSingleton();
		~MenuSingleton();

		/* PROVINCE OVERVIEW PANEL */
		/* Get info to display in Province Overview Panel, packaged in a Dictionary using StringName constants as keys. */
		godot::Dictionary get_province_info_from_index(int32_t index) const;
		int32_t get_province_building_count() const;
		godot::String get_province_building_identifier(int32_t building_index) const;
		godot::Error expand_selected_province_building(int32_t building_index);
		int32_t get_slave_pop_icon_index() const;
		int32_t get_administrative_pop_icon_index() const;
		int32_t get_rgo_owner_pop_icon_index() const;

		/* TIME/SPEED CONTROL PANEL */
		void set_paused(bool paused);
		void toggle_paused();
		bool is_paused() const;
		void increase_speed();
		void decrease_speed();
		int32_t get_speed() const;
		bool can_increase_speed() const;
		bool can_decrease_speed() const;
		godot::String get_longform_date() const;

		/* POPULATION MENU */
		godot::Error _population_menu_update_provinces();
		int32_t get_population_menu_province_list_row_count() const;
		godot::TypedArray<godot::Dictionary> get_population_menu_province_list_rows(int32_t start, int32_t count) const;
		godot::Error population_menu_select_province_list_entry(int32_t select_index, bool set_scroll_index = false);
		godot::Error population_menu_select_province(int32_t province_index);
		godot::Error population_menu_toggle_expanded(int32_t toggle_index, bool emit_selected_changed = true);

		godot::Error _population_menu_update_pops();
		godot::Error _population_menu_update_filtered_pops();
		using sort_func_t = std::function<bool(Pop const*, Pop const*)>;
		sort_func_t _get_population_menu_sort_func(population_menu_t::PopSortKey sort_key) const;
		godot::Error _population_menu_sort_pops();
		godot::Error population_menu_update_locale_sort_cache();
		godot::Error population_menu_select_sort_key(population_menu_t::PopSortKey sort_key);
		godot::TypedArray<godot::Dictionary> get_population_menu_pop_rows(int32_t start, int32_t count) const;
		int32_t get_population_menu_pop_row_count() const;

		godot::Error _population_menu_generate_pop_filters();
		godot::PackedInt32Array get_population_menu_pop_filter_setup_info();
		godot::TypedArray<godot::Dictionary> get_population_menu_pop_filter_info() const;
		godot::Error population_menu_toggle_pop_filter(int32_t filter_index);
		godot::Error population_menu_select_all_pop_filters();
		godot::Error population_menu_deselect_all_pop_filters();

		godot::PackedStringArray get_population_menu_distribution_setup_info() const;
		/* Array of GFXPieChartTexture::godot_pie_chart_data_t. */
		godot::TypedArray<godot::Array> get_population_menu_distribution_info() const;

		/* Find/Search Panel */
		// TODO - update on country government type change and state creation/destruction
		// (which automatically includes country creation/destruction)
		godot::Error generate_search_cache();
		void update_search_results(godot::String const& text);
		godot::PackedStringArray get_search_result_rows(int32_t start, int32_t count) const;
		int32_t get_search_result_row_count() const;
		godot::Vector2 get_search_result_position(int32_t result_index) const;
	};
}

VARIANT_ENUM_CAST(OpenVic::MenuSingleton::population_menu_t::ProvinceListEntry);
VARIANT_ENUM_CAST(OpenVic::MenuSingleton::population_menu_t::PopSortKey);
