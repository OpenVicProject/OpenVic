#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <openvic-simulation/military/UnitInstanceGroup.hpp>
#include <openvic-simulation/types/IndexedMap.hpp>
#include <openvic-simulation/types/PopSize.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>
#include <openvic-simulation/modifier/ModifierEffect.hpp>
#include <openvic-simulation/pop/Pop.hpp>

namespace OpenVic {
	struct CountryInstance;
	struct State;
	struct ProvinceInstance;
	struct Pop;
	struct PopType;
	struct Religion;
	struct Ideology;
	struct Culture;
	struct Issue;
	struct CountryParty;
	struct RebelType;
	struct ModifierValue;
	struct ModifierSum;
	struct RuleSet;
	struct LeaderInstance;
	struct GUIScrollbar;

	class MenuSingleton : public godot::Object {
		GDCLASS(MenuSingleton, godot::Object)

		static inline MenuSingleton* singleton = nullptr;

	public:
		enum ProvinceListEntry {
			LIST_ENTRY_NONE, LIST_ENTRY_COUNTRY, LIST_ENTRY_STATE, LIST_ENTRY_PROVINCE
		};

		enum PopSortKey {
			SORT_NONE, SORT_SIZE, SORT_TYPE, SORT_CULTURE, SORT_RELIGION, SORT_LOCATION, SORT_MILITANCY, SORT_CONSCIOUSNESS,
			SORT_IDEOLOGY, SORT_ISSUES, SORT_UNEMPLOYMENT, SORT_CASH, SORT_LIFE_NEEDS, SORT_EVERYDAY_NEEDS,
			SORT_LUXURY_NEEDS, SORT_REBEL_FACTION, SORT_SIZE_CHANGE, SORT_LITERACY, MAX_SORT_KEY
		};

		struct population_menu_t {
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
				pop_size_t count, promotion_demotion_change;
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
			fixed_point_map_t<PopType const*> workforce_distribution;
			fixed_point_map_t<Religion const*> religion_distribution;
			fixed_point_map_t<Ideology const*> ideology_distribution;
			fixed_point_map_t<Culture const*> culture_distribution;
			fixed_point_map_t<Issue const*> issue_distribution;
			fixed_point_map_t<CountryParty const*> vote_distribution;

			PopSortKey sort_key = SORT_NONE;
			bool sort_descending = true;
			IndexedMap<PopType, size_t> pop_type_sort_cache;
			IndexedMap<Culture, size_t> culture_sort_cache;
			IndexedMap<Religion, size_t> religion_sort_cache;
			IndexedMap<ProvinceInstance, size_t> province_sort_cache;
			IndexedMap<RebelType, size_t> rebel_type_sort_cache;

			std::vector<Pop const*> pops, filtered_pops;
		};

		enum TradeSettingBit {
			TRADE_SETTING_NONE = 0, TRADE_SETTING_AUTOMATED = 1, TRADE_SETTING_BUYING = 2, TRADE_SETTING_SELLING = 4
		};

		enum LeaderSortKey {
			LEADER_SORT_NONE, LEADER_SORT_PRESTIGE, LEADER_SORT_TYPE, LEADER_SORT_NAME, LEADER_SORT_ASSIGNMENT,
			MAX_LEADER_SORT_KEY
		};
		ordered_map<LeaderInstance const*, godot::Dictionary> cached_leader_dicts;
		enum UnitGroupSortKey {
			UNIT_GROUP_SORT_NONE, UNIT_GROUP_SORT_NAME, UNIT_GROUP_SORT_STRENGTH, MAX_UNIT_GROUP_SORT_KEY
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
		/* Emitted when the current tooltip changes. Arguments: text (godot::String), substitution_dict (godot::Dictionary),
		 * position (godot::Vector2). If text is empty then the tooltip will be hidden, otherwise the text will be shown at
		 * the given position. */
		static godot::StringName const& _signal_update_tooltip();

		godot::String _get_state_name(State const& state) const;
		godot::String _get_country_name(CountryInstance const& country) const;
		godot::String _get_country_adjective(CountryInstance const& country) const;

		static godot::String _make_modifier_effect_value(
			ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
		);

		static godot::String _make_modifier_effect_value_coloured(
			ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
		);

		godot::String _make_modifier_effect_tooltip(ModifierEffect const& effect, const fixed_point_t value) const;
		godot::String _make_modifier_effects_tooltip(ModifierValue const& modifier) const;

		template<typename T>
		requires std::same_as<T, CountryInstance> || std::same_as<T, ProvinceInstance>
		godot::String _make_modifier_effect_contributions_tooltip(
			T const& modifier_sum, ModifierEffect const& effect, fixed_point_t* tech_contributions = nullptr,
			fixed_point_t* other_contributions = nullptr, godot::String const& prefix = "\n", godot::String const& suffix = {}
		) const;

		godot::String _make_rules_tooltip(RuleSet const& rules) const;

		godot::String _make_mobilisation_impact_tooltip() const;

	protected:
		static void _bind_methods();

	public:
		static MenuSingleton* get_singleton();

		/* This should only be called AFTER GameSingleton has been initialised! */
		MenuSingleton();
		~MenuSingleton();

		static godot::String get_tooltip_separator();
		static godot::String get_tooltip_condition_met();
		static godot::String get_tooltip_condition_unmet();

		godot::String get_country_name_from_identifier(godot::String const& country_identifier) const;
		godot::String get_country_adjective_from_identifier(godot::String const& country_identifier) const;

		/* TOOLTIP */
		void show_tooltip(
			godot::String const& text, godot::Dictionary const& substitution_dict, godot::Vector2 const& position
		);
		void show_control_tooltip(
			godot::String const& text, godot::Dictionary const& substitution_dict, godot::Control const* control
		);
		void hide_tooltip();

		/* PROVINCE OVERVIEW PANEL */
		/* Get info to display in Province Overview Panel, packaged in a Dictionary using StringName constants as keys. */
		godot::Dictionary get_province_info_from_index(int32_t index) const;
		int32_t get_province_building_count() const;
		godot::String get_province_building_identifier(int32_t building_index) const;
		int32_t get_slave_pop_icon_index() const;
		int32_t get_administrative_pop_icon_index() const;
		int32_t get_rgo_owner_pop_icon_index() const;

		/* TOPBAR */
		godot::Dictionary get_topbar_info() const;

		/* TIME/SPEED CONTROL PANEL */
		bool is_paused() const;
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
		sort_func_t _get_population_menu_sort_func(PopSortKey sort_key) const;
		godot::Error _population_menu_sort_pops();
		godot::Error population_menu_update_locale_sort_cache();
		godot::Error population_menu_select_sort_key(PopSortKey sort_key);
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

		/* TECHNOLOGY MENU */
		godot::Dictionary get_technology_menu_defines() const;
		godot::Dictionary get_technology_menu_info() const;
		godot::Dictionary get_specific_technology_info(godot::String technology_identifier) const;

		/* TRADE MENU */
		godot::Dictionary get_trade_menu_good_categories_info() const;
		godot::Dictionary get_trade_menu_trade_details_info(
			int32_t trade_detail_good_index, GUIScrollbar* stockpile_cutoff_slider
		) const;
		godot::Dictionary get_trade_menu_tables_info() const;
		static constexpr fixed_point_t calculate_trade_menu_stockpile_cutoff_amount_fp(fixed_point_t value) {
			// TODO - replace this with: pow(2001, value / 2000) - 1
			return value;
		}
		static float calculate_trade_menu_stockpile_cutoff_amount(GUIScrollbar const* slider);

		/* MILITARY MENU */
		godot::Dictionary make_leader_dict(LeaderInstance const& leader);
		template<UnitType::branch_t Branch>
		godot::Dictionary make_unit_group_dict(UnitInstanceGroupBranched<Branch> const& unit_group);
		godot::Dictionary make_in_progress_unit_dict() const;
		godot::Dictionary get_military_menu_info(
			LeaderSortKey leader_sort_key, bool sort_leaders_descending,
			UnitGroupSortKey army_sort_key, bool sort_armies_descending,
			UnitGroupSortKey navy_sort_key, bool sort_navies_descending
		);

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

VARIANT_ENUM_CAST(OpenVic::MenuSingleton::ProvinceListEntry);
VARIANT_ENUM_CAST(OpenVic::MenuSingleton::PopSortKey);
VARIANT_ENUM_CAST(OpenVic::MenuSingleton::TradeSettingBit);
VARIANT_ENUM_CAST(OpenVic::MenuSingleton::LeaderSortKey);
VARIANT_ENUM_CAST(OpenVic::MenuSingleton::UnitGroupSortKey);
