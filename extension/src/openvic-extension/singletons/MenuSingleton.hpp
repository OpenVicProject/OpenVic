#pragma once

#include <cstdint>
#include <variant>

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/image.hpp>

#include <openvic-simulation/military/UnitInstanceGroup.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>
#include <openvic-simulation/types/IndexedFlatMap.hpp>
#include <openvic-simulation/types/PopSize.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>
#include <openvic-simulation/types/UnitBranchType.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/components/budget/BudgetMenu.hpp"
#include "openvic-extension/components/overview/TopBar.hpp"

namespace OpenVic {
	struct CountryInstance;
	struct State;
	struct ProvinceInstance;
	struct Pop;
	struct PopType;
	struct Religion;
	struct Ideology;
	struct Culture;
	struct BaseIssue;
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
			fixed_point_map_t<BaseIssue const*> issue_distribution;
			fixed_point_map_t<CountryParty const*> vote_distribution;

			PopSortKey sort_key = SORT_NONE;
			bool sort_descending = true;
			IndexedFlatMap<PopType, size_t> pop_type_sort_cache;
			ordered_map<Culture const*, size_t> culture_sort_cache;
			ordered_map<Religion const*, size_t> religion_sort_cache;
			IndexedFlatMap<ProvinceInstance, size_t> province_sort_cache;
			IndexedFlatMap<RebelType, size_t> rebel_type_sort_cache;

			std::vector<Pop const*> pops, filtered_pops;
			population_menu_t();
		};

		enum TradeSettingBit {
			TRADE_SETTING_NONE = 0, TRADE_SETTING_AUTOMATED = 1, TRADE_SETTING_BUYING = 2, TRADE_SETTING_SELLING = 4
		};

		ordered_map<LeaderInstance const*, godot::Dictionary> cached_leader_dicts;

		struct search_panel_t {
			struct entry_t {
				std::variant<ProvinceInstance const*, State const*, CountryInstance const*> target;
				godot::String display_name, search_name, identifier;
			};
			std::vector<entry_t> entry_cache;
			std::vector<size_t> result_indices;
		};

	private:
		memory::unique_ptr<BudgetMenu> budget_menu;
		memory::unique_ptr<TopBar> top_bar;
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

		godot::String _make_modifier_effect_value(
			ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
		) const;

		godot::String _make_modifier_effect_value_coloured(
			ModifierEffect const& format_effect, fixed_point_t value, bool plus_for_non_negative
		) const;

		godot::String _make_modifier_effects_tooltip(ModifierValue const& modifier) const;

		template<typename T>
		requires std::same_as<T, CountryInstance> || std::same_as<T, ProvinceInstance>
		godot::String _make_modifier_effect_contributions_tooltip(
			T const& modifier_sum, ModifierEffect const& effect, fixed_point_t* effect_value = nullptr,
			godot::String const& prefix = "\n", godot::String const& suffix = {}
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

		static godot::String get_issue_identifier_suffix();

		static godot::String get_tooltip_separator();
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
		godot::Dictionary get_province_info_from_number(int32_t province_number) const;
		int32_t get_province_building_count() const;
		godot::String get_province_building_identifier(int32_t building_index) const;
		int32_t get_slave_pop_icon_index() const;
		int32_t get_administrative_pop_icon_index() const;
		int32_t get_rgo_owner_pop_icon_index() const;

		/* TOPBAR */
		godot::Dictionary get_topbar_info() const;
		void link_top_bar_to_cpp(GUINode const* const godot_top_bar);
		void unlink_top_bar_from_cpp();

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
		godot::Error population_menu_select_province(int32_t province_number);
		godot::Error population_menu_toggle_expanded(int32_t toggle_index, bool emit_selected_changed = true);

		godot::Error _population_menu_update_pops();
		godot::Error _population_menu_update_filtered_pops();
		using sort_func_t = std::function<bool(Pop const*, Pop const*)>;
		sort_func_t _get_population_menu_sort_func(PopSortKey sort_key) const;
		godot::Error _population_menu_sort_pops();
		godot::Error population_menu_update_locale_sort_cache();
		godot::Error population_menu_select_sort_key(PopSortKey sort_key);
		template<IsPieChartDistribution Container>
		GFXPieChartTexture::godot_pie_chart_data_t generate_population_menu_pop_row_pie_chart_data(
			Container const& distribution, godot::String const& identifier_suffix = {}
		) const;
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

		/* TRADE MENU */
		godot::Dictionary get_trade_menu_good_categories_info() const;
		godot::Dictionary get_trade_menu_trade_details_info(
			int32_t trade_detail_good_index, GUIScrollbar* stockpile_cutoff_slider
		) const;
		godot::Dictionary get_trade_menu_tables_info() const;

		static constexpr int32_t calculate_slider_value_from_trade_menu_stockpile_cutoff(
			const fixed_point_t stockpile_cutoff,
			const int32_t max_slider_value
		) {
			// Math.log(2)/Math.log(Math.exp(Math.log(2001)/2000)) = 182.37...
			constexpr fixed_point_t DOUBLES_AFTER_STEPS = fixed_point_t::parse_raw(11952029);
			int32_t times_halved = 0;
			fixed_point_t copy_plus_one = stockpile_cutoff+1;
			while (copy_plus_one >= 2) {
				copy_plus_one /= 2;
				times_halved++;
			}
			int32_t slider_value = times_halved * DOUBLES_AFTER_STEPS.truncate<int32_t>();
			while (
				calculate_trade_menu_stockpile_cutoff_amount_fp(
					slider_value
				) < stockpile_cutoff
			) {
				if (slider_value >= max_slider_value) {
					return max_slider_value;
				}
				++slider_value;
			}

			return slider_value;
		}
		static constexpr fixed_point_t calculate_trade_menu_stockpile_cutoff_amount_fp(fixed_point_t value) {
			return fixed_point_t::exp_2001(value / 2000) - fixed_point_t::_1;
		}
		static float calculate_trade_menu_stockpile_cutoff_amount(GUIScrollbar const* slider);

		/* MILITARY MENU */
		godot::Dictionary make_leader_dict(LeaderInstance const& leader);
		template<unit_branch_t Branch>
		godot::Dictionary make_unit_group_dict(UnitInstanceGroupBranched<Branch> const& unit_group);
		godot::Dictionary make_in_progress_unit_dict() const;
		godot::Dictionary get_military_menu_info();

		/* BUDGET MENU */
		void link_budget_menu_to_cpp(GUINode const* const godot_budget_menu);
		void unlink_budget_menu_from_cpp();

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
