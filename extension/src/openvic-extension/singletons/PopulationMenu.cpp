#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/DefinitionManager.hpp>
#include <openvic-simulation/InstanceManager.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

/* POPULATION MENU */

Error MenuSingleton::_population_menu_update_provinces() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, FAILED);

	population_menu.province_list_entries.clear();
	population_menu.visible_province_list_entries = 0;
	ERR_FAIL_COND_V(_population_menu_generate_pop_filters() != OK, FAILED);

	MapInstance const& map_instance = instance_manager->get_map_instance();
	ERR_FAIL_COND_V(!map_instance.province_instances_are_locked(), FAILED);

	for (CountryInstance const* country : {
		// Example country
		instance_manager->get_country_instance_manager().get_country_instance_by_identifier("ENG")
	}) {
		ERR_CONTINUE(country == nullptr);

		population_menu.province_list_entries.emplace_back(population_menu_t::country_entry_t { *country });
		population_menu.visible_province_list_entries++;

		for (StateSet const& state_set : map_instance.get_state_manager().get_state_sets()) {
			for (State const& state : state_set.get_states()) {

				population_menu.province_list_entries.emplace_back(population_menu_t::state_entry_t { state });
				population_menu.visible_province_list_entries++;

				for (ProvinceInstance const* province : state.get_provinces()) {
					population_menu.province_list_entries.emplace_back(population_menu_t::province_entry_t { *province });
				}
			}
		}
	}

	population_menu.sort_key = SORT_NONE;

	emit_signal(_signal_population_menu_province_list_changed());

	// TODO - may need to emit population_menu_province_list_selected_changed if _update_info cannot be guaranteed

	return _population_menu_update_pops();
}

int32_t MenuSingleton::get_population_menu_province_list_row_count() const {
	return population_menu.visible_province_list_entries;
}

TypedArray<Dictionary> MenuSingleton::get_population_menu_province_list_rows(int32_t start, int32_t count) const {
	if (population_menu.province_list_entries.empty()) {
		return {};
	}

	ERR_FAIL_INDEX_V_MSG(
		start, population_menu.visible_province_list_entries, {},
		vformat("Invalid start for population menu province list rows: %d", start)
	);
	ERR_FAIL_COND_V_MSG(count <= 0, {}, vformat("Invalid count for population menu province list rows: %d", count));

	static const StringName type_key = "type";
	static const StringName index_key = "index";
	static const StringName name_key = "name";
	static const StringName size_key = "size";
	static const StringName change_key = "change";
	static const StringName selected_key = "selected";
	/* State-only keys */
	static const StringName expanded_key = "expanded";
	static const StringName colonial_status_key = "colony";
	// TODO - national focus

	struct entry_visitor_t {

		MenuSingleton const& menu_singleton;

		int32_t& start_counter;
		int32_t& count_counter;

		/* This is the index among all entries, not just visible ones unlike start and count. */
		int32_t index = 0;

		bool is_expanded = true;

		TypedArray<Dictionary> array;

		/* Overloads return false if count_counter reaches 0 and the function should return,
		 * otherwise true indicating the province list loop should continue. */

		bool operator()(population_menu_t::country_entry_t const& country_entry) {
			if (start_counter-- <= 0) {
				Dictionary country_dict;

				country_dict[type_key] = LIST_ENTRY_COUNTRY;
				country_dict[index_key] = index;
				country_dict[name_key] = menu_singleton._get_country_name(country_entry.country);
				country_dict[size_key] = country_entry.country.get_total_population();
				country_dict[change_key] = 0;
				country_dict[selected_key] = country_entry.selected;

				array.push_back(country_dict);

				return --count_counter > 0;
			}

			return true;
		}

		bool operator()(population_menu_t::state_entry_t const& state_entry) {
			is_expanded = state_entry.expanded;

			if (start_counter-- <= 0) {
				Dictionary state_dict;

				state_dict[type_key] = LIST_ENTRY_STATE;
				state_dict[index_key] = index;
				state_dict[name_key] = menu_singleton._get_state_name(state_entry.state);
				state_dict[size_key] = state_entry.state.get_total_population();
				state_dict[change_key] = 0;
				state_dict[selected_key] = state_entry.selected;
				state_dict[expanded_key] = state_entry.expanded;
				state_dict[colonial_status_key] = false;

				array.push_back(state_dict);

				return --count_counter > 0;
			}

			return true;
		}

		bool operator()(population_menu_t::province_entry_t const& province_entry) {
			if (is_expanded && start_counter-- <= 0) {
				Dictionary province_dict;

				province_dict[type_key] = LIST_ENTRY_PROVINCE;
				province_dict[index_key] = index;
				province_dict[name_key] = Utilities::std_to_godot_string(province_entry.province.get_identifier());
				province_dict[size_key] = province_entry.province.get_total_population();
				province_dict[change_key] = 0;
				province_dict[selected_key] = province_entry.selected;

				array.push_back(province_dict);

				return --count_counter > 0;
			}

			return true;
		}
	} entry_visitor { *this, start, count };

	while (entry_visitor.index < population_menu.province_list_entries.size()
		&& std::visit(entry_visitor, population_menu.province_list_entries[entry_visitor.index])) {
		entry_visitor.index++;
	}

	return entry_visitor.array;
}

Error MenuSingleton::population_menu_select_province_list_entry(int32_t select_index, bool set_scroll_index) {
	ERR_FAIL_INDEX_V(select_index, population_menu.province_list_entries.size(), FAILED);

	struct entry_visitor_t {

		const int32_t _select_index = 0;

		int32_t index = 0, visible_index = 0;
		bool is_expanded = true;

		int32_t selected_visible_index = -1;

		ProvinceListEntry select_level = LIST_ENTRY_NONE;

		void operator()(population_menu_t::country_entry_t& country_entry) {
			if (index == _select_index) {
				select_level = LIST_ENTRY_COUNTRY;

				country_entry.selected = true;

				selected_visible_index = visible_index;
			} else {
				select_level = LIST_ENTRY_NONE;

				country_entry.selected = false;
			}

			visible_index++;
		}

		void operator()(population_menu_t::state_entry_t& state_entry) {
			if (select_level == LIST_ENTRY_COUNTRY) {
				state_entry.selected = true;
			} else if (index == _select_index) {
				select_level = LIST_ENTRY_STATE;

				state_entry.selected = true;

				selected_visible_index = visible_index;
			} else {
				select_level = LIST_ENTRY_NONE;
				state_entry.selected = false;
			}

			visible_index++;

			is_expanded = state_entry.expanded;
		}

		void operator()(population_menu_t::province_entry_t& province_entry) {
			if (select_level == LIST_ENTRY_COUNTRY || select_level == LIST_ENTRY_STATE) {
				province_entry.selected = true;
			} else if (index == _select_index) {
				province_entry.selected = true;

				selected_visible_index = visible_index;
			} else {
				province_entry.selected = false;
			}

			if (is_expanded) {
				visible_index++;
			}
		}

	} entry_visitor { select_index };

	while (entry_visitor.index < population_menu.province_list_entries.size()) {
		std::visit(entry_visitor, population_menu.province_list_entries[entry_visitor.index]);
		entry_visitor.index++;
	}

	emit_signal(
		_signal_population_menu_province_list_selected_changed(),
		set_scroll_index ? entry_visitor.selected_visible_index : -1
	);

	return _population_menu_update_pops();
}

Error MenuSingleton::population_menu_select_province(int32_t province_index) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, FAILED);

	ERR_FAIL_COND_V(
		province_index <= 0 || province_index > instance_manager->get_map_instance().get_province_instance_count(), FAILED
	);

	struct entry_visitor_t {

		MenuSingleton& menu_singleton;

		const int32_t _province_index = 0;

		int32_t index = 0;

		int32_t state_entry_to_expand = -1;

		bool ret = true;

		/* Overloads return false if the province entry is found and the loop can stop, true otherwise. */

		bool operator()(population_menu_t::country_entry_t& country_entry) {
			return true;
		}

		bool operator()(population_menu_t::state_entry_t& state_entry) {
			if (state_entry.expanded) {
				state_entry_to_expand = -1;
			} else {
				state_entry_to_expand = index;
			}
			return true;
		}

		bool operator()(population_menu_t::province_entry_t& province_entry) {
			if (province_entry.province.get_index() == _province_index) {

				if (state_entry_to_expand >= 0) {
					ret &= menu_singleton.population_menu_toggle_expanded(state_entry_to_expand, false) == OK;
				}

				ret &= menu_singleton.population_menu_select_province_list_entry(index, true) == OK;

				return false;
			}
			return true;
		}

	} entry_visitor { *this, province_index };

	while (entry_visitor.index < population_menu.province_list_entries.size()
		&& std::visit(entry_visitor, population_menu.province_list_entries[entry_visitor.index])) {
		entry_visitor.index++;
	}

	ERR_FAIL_COND_V_MSG(
		entry_visitor.index >= population_menu.province_list_entries.size(), FAILED,
		vformat("Cannot select province index %d - not found in population menu province list!", province_index)
	);

	return ERR(entry_visitor.ret);
}

Error MenuSingleton::population_menu_toggle_expanded(int32_t toggle_index, bool emit_selected_changed) {
	ERR_FAIL_INDEX_V(toggle_index, population_menu.province_list_entries.size(), FAILED);

	population_menu_t::state_entry_t* state_entry =
		std::get_if<population_menu_t::state_entry_t>(&population_menu.province_list_entries[toggle_index]);

	ERR_FAIL_NULL_V_MSG(state_entry, FAILED, vformat("Cannot toggle expansion of a non-state entry! (%d)", toggle_index));

	int32_t provinces = 0;

	while (++toggle_index < population_menu.province_list_entries.size()
		&& std::holds_alternative<population_menu_t::province_entry_t>(population_menu.province_list_entries[toggle_index])) {
		provinces++;
	}

	if (state_entry->expanded) {
		state_entry->expanded = false;
		population_menu.visible_province_list_entries -= provinces;
	} else {
		state_entry->expanded = true;
		population_menu.visible_province_list_entries += provinces;
	}

	emit_signal(_signal_population_menu_province_list_changed());

	if (emit_selected_changed) {
		emit_signal(_signal_population_menu_province_list_selected_changed(), -1);
	}

	return OK;
}

Error MenuSingleton::_population_menu_update_pops() {
	for (auto [pop_type, filter] : mutable_iterator(population_menu.pop_filters)) {
		filter.count = 0;
		filter.promotion_demotion_change = 0;
	}

	population_menu.pops.clear();

	for (int32_t index = 0; index < population_menu.province_list_entries.size(); index++) {
		population_menu_t::province_entry_t const* province_entry =
			std::get_if<population_menu_t::province_entry_t>(&population_menu.province_list_entries[index]);

		if (province_entry != nullptr && province_entry->selected) {
			for (Pop const& pop : province_entry->province.get_pops()) {
				population_menu.pops.push_back(&pop);
				population_menu_t::pop_filter_t& filter = population_menu.pop_filters[pop.get_type()];
				filter.count += pop.get_size();
				// TODO - set filter.promotion_demotion_change
			}
		}
	}

	return _population_menu_update_filtered_pops();
}

Error MenuSingleton::_population_menu_update_filtered_pops() {
	population_menu.filtered_pops.clear();

	population_menu.workforce_distribution.clear();
	population_menu.religion_distribution.clear();
	population_menu.ideology_distribution.clear();
	population_menu.culture_distribution.clear();
	population_menu.issue_distribution.clear();
	population_menu.vote_distribution.clear();

	for (Pop const* pop : population_menu.pops) {
		if (population_menu.pop_filters[pop->get_type()].selected) {
			population_menu.filtered_pops.push_back(pop);
		}
	}

	for (Pop const* pop : population_menu.filtered_pops) {
		const fixed_point_t pop_size = fixed_point_t::parse(pop->get_size());

		population_menu.workforce_distribution[pop->get_type()] += pop_size;
		population_menu.religion_distribution[&pop->get_religion()] += pop_size;
		population_menu.ideology_distribution += pop->get_ideology_distribution();
		population_menu.culture_distribution[&pop->get_culture()] += pop_size;
		population_menu.issue_distribution += pop->get_issue_distribution();
		population_menu.vote_distribution += pop->get_vote_distribution();
	}

	normalise_fixed_point_map(population_menu.workforce_distribution);
	normalise_fixed_point_map(population_menu.religion_distribution);
	normalise_fixed_point_map(population_menu.ideology_distribution);
	normalise_fixed_point_map(population_menu.culture_distribution);
	normalise_fixed_point_map(population_menu.issue_distribution);
	normalise_fixed_point_map(population_menu.vote_distribution);

	return _population_menu_sort_pops();
}

template<typename T>
static constexpr bool indexed_map_lookup_less_than(IndexedMap<T, size_t> const& map, T const& lhs, T const& rhs) {
	return map[lhs] < map[rhs];
}

template<typename T>
static constexpr bool indexed_map_lookup_less_than(IndexedMap<T, size_t> const& map, T const* lhs, T const* rhs) {
	return lhs != nullptr && rhs != nullptr && map[*lhs] < map[*rhs];
}

MenuSingleton::sort_func_t MenuSingleton::_get_population_menu_sort_func(PopSortKey sort_key) const {
	switch (sort_key) {
	case SORT_SIZE:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_size() < b->get_size();
		};
	case SORT_TYPE:
		return [this](Pop const* a, Pop const* b) -> bool {
			return indexed_map_lookup_less_than(population_menu.pop_type_sort_cache, a->get_type(), b->get_type());
		};
	case SORT_CULTURE:
		return [this](Pop const* a, Pop const* b) -> bool {
			return indexed_map_lookup_less_than(population_menu.culture_sort_cache, a->get_culture(), b->get_culture());
		};
	case SORT_RELIGION:
		return [this](Pop const* a, Pop const* b) -> bool {
			return indexed_map_lookup_less_than(population_menu.religion_sort_cache, a->get_religion(), b->get_religion());
		};
	case SORT_LOCATION:
		return [this](Pop const* a, Pop const* b) -> bool {
			return indexed_map_lookup_less_than(population_menu.province_sort_cache, a->get_location(), b->get_location());
		};
	case SORT_MILITANCY:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_militancy() < b->get_militancy();
		};
	case SORT_CONSCIOUSNESS:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_consciousness() < b->get_consciousness();
		};
	case SORT_IDEOLOGY:
		return [](Pop const* a, Pop const* b) -> bool {
			return sorted_indexed_map_less_than(a->get_ideology_distribution(), b->get_ideology_distribution());
		};
	case SORT_ISSUES:
		return [](Pop const* a, Pop const* b) -> bool {
			return sorted_fixed_map_less_than(a->get_issue_distribution(), b->get_issue_distribution());
		};
	case SORT_UNEMPLOYMENT:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_unemployment_fraction() < b->get_unemployment_fraction();
		};
	case SORT_CASH:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_cash().get_copy_of_value() < b->get_cash().get_copy_of_value();
		};
	case SORT_LIFE_NEEDS:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_life_needs_fulfilled() < b->get_life_needs_fulfilled();
		};
	case SORT_EVERYDAY_NEEDS:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_everyday_needs_fulfilled() < b->get_everyday_needs_fulfilled();
		};
	case SORT_LUXURY_NEEDS:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_luxury_needs_fulfilled() < b->get_luxury_needs_fulfilled();
		};
	case SORT_REBEL_FACTION:
		return [this](Pop const* a, Pop const* b) -> bool {
			// TODO - include country adjective for [pan-]nationalist rebels
			// TODO - handle social/political reform movements
			return indexed_map_lookup_less_than(
				population_menu.rebel_type_sort_cache, a->get_rebel_type(), b->get_rebel_type()
			);
		};
	case SORT_SIZE_CHANGE:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_total_change() < b->get_total_change();
		};
	case SORT_LITERACY:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_literacy() < b->get_literacy();
		};
	default:
		UtilityFunctions::push_error("Invalid population menu sort key: ", sort_key);
		return [](Pop const* a, Pop const* b) -> bool { return false; };
	}
}

Error MenuSingleton::_population_menu_sort_pops() {
	if (population_menu.sort_key != SORT_NONE) {
		if (
			!population_menu.pop_type_sort_cache.has_keys() || !population_menu.culture_sort_cache.has_keys() ||
			!population_menu.religion_sort_cache.has_keys() || !population_menu.province_sort_cache.has_keys() ||
			!population_menu.rebel_type_sort_cache.has_keys()
		) {
			ERR_FAIL_COND_V(population_menu_update_locale_sort_cache() != OK, FAILED);
		}

		const sort_func_t base_sort_func = _get_population_menu_sort_func(population_menu.sort_key);

		const sort_func_t sort_func = population_menu.sort_descending
			? base_sort_func
			: [base_sort_func](Pop const* a, Pop const* b) { return base_sort_func(b, a); };

		std::sort(population_menu.filtered_pops.begin(), population_menu.filtered_pops.end(), sort_func);
	}

	emit_signal(_signal_population_menu_pops_changed());

	return OK;
}

Error MenuSingleton::population_menu_update_locale_sort_cache() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, FAILED);

	std::vector<String> localised_items;
	std::vector<size_t> sorted_items;

	const auto generate_sort_cache = [this, &localised_items, &sorted_items]<HasGetIdentifier T>(
		IndexedMap<T, size_t>& cache, memory::vector<T> const& items
	) {
		localised_items.resize(items.size());
		sorted_items.resize(items.size());

		for (size_t idx = 0; idx < items.size(); ++idx) {
			String identifier = Utilities::std_to_godot_string(items[idx].get_identifier());
			if constexpr (std::is_same_v<T, ProvinceInstance>) {
				identifier = GUINode::format_province_name(identifier);
			}
			localised_items[idx] = tr(identifier).to_lower();
			sorted_items[idx] = idx;
		}

		std::sort(
			sorted_items.begin(), sorted_items.end(), [&localised_items](size_t a, size_t b) -> bool {
				return localised_items[a] < localised_items[b];
			}
		);

		cache.set_keys(&items);
		for (size_t idx = 0; idx < sorted_items.size(); ++idx) {
			cache[sorted_items[idx]] = idx;
		}
	};

	generate_sort_cache(
		population_menu.pop_type_sort_cache,
		game_singleton->get_definition_manager().get_pop_manager().get_pop_types()
	);
	generate_sort_cache(
		population_menu.culture_sort_cache,
		game_singleton->get_definition_manager().get_pop_manager().get_culture_manager().get_cultures()
	);
	generate_sort_cache(
		population_menu.religion_sort_cache,
		game_singleton->get_definition_manager().get_pop_manager().get_religion_manager().get_religions()
	);
	generate_sort_cache(
		population_menu.province_sort_cache,
		instance_manager->get_map_instance().get_province_instances()
	);
	generate_sort_cache(
		population_menu.rebel_type_sort_cache,
		game_singleton->get_definition_manager().get_politics_manager().get_rebel_manager().get_rebel_types()
	);

	return OK;
}

Error MenuSingleton::population_menu_select_sort_key(PopSortKey sort_key) {
	/* sort_key must be cast here to avoid causing clang to segfault during compilation. */
	ERR_FAIL_INDEX_V_MSG(
		static_cast<int32_t>(sort_key), static_cast<int32_t>(MAX_SORT_KEY), FAILED,
		vformat("Invalid population menu sort key: %d (must be under %d)", sort_key, MAX_SORT_KEY)
	);

	if (sort_key == population_menu.sort_key) {
		/* Re-selecting the current sort key reverses sort order. */
		population_menu.sort_descending = !population_menu.sort_descending;
	} else {
		/* Selecting a new sort key switches sorting to that key, preserving the existing sort order. */
		population_menu.sort_key = sort_key;
	}

	return _population_menu_sort_pops();
}

template<IsPieChartDistribution Container>
GFXPieChartTexture::godot_pie_chart_data_t MenuSingleton::generate_population_menu_pop_row_pie_chart_data(
	Container const& distribution, String const& identifier_suffix
) const {
	using key_type = std::remove_pointer_t<typename Container::key_type>;

	ordered_map<key_type const*, String> tooltips;
	tooltips.reserve(distribution.size());

	String full_tooltip = get_tooltip_separator().trim_suffix("\n");

	float total_weight = 0.0f;
	for (auto [key, weight] : distribution) {
		total_weight += static_cast<float>(weight);
	}

	static const String pie_chart_tooltip_format_key = "%s: " + GUILabel::get_colour_marker() + "Y%s%%" +
		GUILabel::get_colour_marker() + "!";

	for (auto [key_ref_or_ptr, weight] : distribution) {
		if (weight > 0.0f) {
			key_type const* key_ptr;
			if constexpr (std::same_as<decltype(key_ptr), decltype(key_ref_or_ptr)>) {
				key_ptr = key_ref_or_ptr;
			} else {
				key_ptr = &key_ref_or_ptr;
			}
			String tooltip = vformat(
				pie_chart_tooltip_format_key,
				tr(Utilities::std_to_godot_string(key_ptr->get_identifier()) + identifier_suffix),
				Utilities::float_to_string_dp(100.0f * static_cast<float>(weight) / total_weight, 1)
			);
			full_tooltip += "\n" + tooltip;
			tooltips.emplace(key_ptr, std::move(tooltip));
		}
		// No need to handle negative (invalid) weights here, GFXPieChartTexture::distribution_to_slices_array will
		// log errors for them and discard them without attempting to generate a tooltip.
	}

	return GFXPieChartTexture::distribution_to_slices_array(
		distribution,
		[&tooltips, full_tooltip](key_type const* key, String const& identifier, float weight, float total_weight) -> String {
			return tooltips.at(key) + full_tooltip;
		},
		identifier_suffix
	);
}

TypedArray<Dictionary> MenuSingleton::get_population_menu_pop_rows(int32_t start, int32_t count) const {
	if (population_menu.filtered_pops.empty()) {
		return {};
	}
	ERR_FAIL_INDEX_V_MSG(
		start, population_menu.filtered_pops.size(), {}, vformat("Invalid start for population menu pop rows: %d", start)
	);
	ERR_FAIL_COND_V_MSG(count <= 0, {}, vformat("Invalid count for population menu pop rows: %d", count));

	if (start + count > population_menu.filtered_pops.size()) {
		count = population_menu.filtered_pops.size() - start;
	}

	static const StringName pop_size_key = "size";

	static const StringName pop_type_icon_key = "pop_type_icon";
	// TODO - pop type name
	// TODO - promotions (target pop type and count)
	// TODO - demotions (target pop type and count)
	// TODO - good being produced (artisans, farmers, labourers, slaves)
	// TODO - military unit and army (soldiers)

	static const StringName pop_culture_key = "culture";
	// TODO - cultural assimilation (primary/accepted, or number, target culture, and conditional weights breakdown)

	static const StringName pop_religion_icon_key = "religion_icon";
	// TODO - religion name
	// TODO - religious conversion (accepted, or number, target religion, and conditional weights breakdown)

	static const StringName pop_location_key = "location";
	// TODO - internal, external and colonial migration

	static const StringName pop_militancy_key = "militancy";
	// TODO - monthly militancy change and modifier breakdown

	static const StringName pop_consciousness_key = "consciousness";
	// TODO - monthly consciousness change and modifier breakdown

	static const StringName pop_ideology_key = "ideology";

	static const StringName pop_issues_key = "issues";

	static const StringName pop_unemployment_key = "unemployment";

	static const StringName pop_cash_key = "cash";
	static const StringName pop_daily_money_key = "daily_money";
	// TODO - daily income, needs, salary and savings

	static const StringName pop_life_needs_key = "life_needs";
	static const StringName pop_everyday_needs_key = "everyday_needs";
	static const StringName pop_luxury_needs_key = "luxury_needs";
	// TODO - goods not available on market or goods not affordale + price (for all 3 needs types)

	static const StringName pop_rebel_icon_key = "rebel_icon";
	// TODO - rebel faction name/description
	// TODO - icons for social/political reform movements
	// TODO - flags for country-related rebels

	static const StringName pop_size_change_key = "size_change";
	// TODO - size change breakdown

	static const StringName pop_literacy_key = "literacy";
	// TODO - monthly change

	TypedArray<Dictionary> array;
	ERR_FAIL_COND_V(array.resize(count) != OK, {});

	for (int32_t idx = 0; idx < count; ++idx) {
		Pop const* pop = population_menu.filtered_pops[start + idx];
		Dictionary pop_dict;

		pop_dict[pop_size_key] = pop->get_size();
		pop_dict[pop_type_icon_key] = pop->get_type()->get_sprite();
		pop_dict[pop_culture_key] = Utilities::std_to_godot_string(pop->get_culture().get_identifier());
		pop_dict[pop_religion_icon_key] = pop->get_religion().get_icon();
		if (pop->get_location() != nullptr) {
			pop_dict[pop_location_key] = Utilities::std_to_godot_string(pop->get_location()->get_identifier());
		}
		pop_dict[pop_militancy_key] = pop->get_militancy().to_float();
		pop_dict[pop_consciousness_key] = pop->get_consciousness().to_float();
		pop_dict[pop_ideology_key] = generate_population_menu_pop_row_pie_chart_data(pop->get_ideology_distribution());
		pop_dict[pop_issues_key] = generate_population_menu_pop_row_pie_chart_data(
			pop->get_issue_distribution(), get_issue_identifier_suffix()
		);
		pop_dict[pop_unemployment_key] = pop->get_unemployment_fraction().to_float();
		pop_dict[pop_cash_key] = pop->get_cash().get_copy_of_value().to_float();
		pop_dict[pop_daily_money_key] = pop->get_income().to_float();
		pop_dict[pop_life_needs_key] = pop->get_life_needs_fulfilled().to_float();
		pop_dict[pop_everyday_needs_key] = pop->get_everyday_needs_fulfilled().to_float();
		pop_dict[pop_luxury_needs_key] = pop->get_luxury_needs_fulfilled().to_float();
		if (pop->get_rebel_type() != nullptr) {
			pop_dict[pop_rebel_icon_key] = pop->get_rebel_type()->get_icon();
		}
		pop_dict[pop_size_change_key] = pop->get_total_change();
		pop_dict[pop_literacy_key] = pop->get_literacy().to_float();

		array[idx] = std::move(pop_dict);
	}

	return array;
}

int32_t MenuSingleton::get_population_menu_pop_row_count() const {
	return population_menu.filtered_pops.size();
}

Error MenuSingleton::_population_menu_generate_pop_filters() {
	if (population_menu.pop_filters.empty()) {
		GameSingleton const* game_singleton = GameSingleton::get_singleton();
		ERR_FAIL_NULL_V(game_singleton, FAILED);

		for (PopType const& pop_type : game_singleton->get_definition_manager().get_pop_manager().get_pop_types()) {
			population_menu.pop_filters.emplace(&pop_type, population_menu_t::pop_filter_t { 0, 0, true });
		}

		ERR_FAIL_COND_V_MSG(population_menu.pop_filters.empty(), FAILED, "Failed to generate population menu pop filters!");
	}

	return OK;
}

PackedInt32Array MenuSingleton::get_population_menu_pop_filter_setup_info() {
	ERR_FAIL_COND_V(_population_menu_generate_pop_filters() != OK, {});

	PackedInt32Array array;
	ERR_FAIL_COND_V(array.resize(population_menu.pop_filters.size()) != OK, {});

	for (int32_t idx = 0; idx < array.size(); ++idx) {
		array[idx] = population_menu.pop_filters.data()[idx].first->get_sprite();
	}

	return array;
}

TypedArray<Dictionary> MenuSingleton::get_population_menu_pop_filter_info() const {
	static const StringName pop_filter_count_key = "count";
	static const StringName pop_filter_change_key = "change";
	static const StringName pop_filter_selected_key = "selected";

	TypedArray<Dictionary> array;
	ERR_FAIL_COND_V(array.resize(population_menu.pop_filters.size()) != OK, {});

	for (int32_t idx = 0; idx < array.size(); ++idx) {
		population_menu_t::pop_filter_t const& filter = population_menu.pop_filters.data()[idx].second;

		Dictionary filter_dict;

		filter_dict[pop_filter_count_key] = filter.count;
		filter_dict[pop_filter_change_key] = filter.promotion_demotion_change;
		filter_dict[pop_filter_selected_key] = filter.selected;

		array[idx] = std::move(filter_dict);
	}

	return array;
}

Error MenuSingleton::population_menu_toggle_pop_filter(int32_t index) {
	ERR_FAIL_COND_V_MSG(
		index < 0 || index >= population_menu.pop_filters.size(), FAILED, vformat("Invalid pop filter index: %d", index)
	);

	population_menu_t::pop_filter_t& filter = mutable_iterator(population_menu.pop_filters).begin()[index].second;
	filter.selected = !filter.selected;

	return _population_menu_update_filtered_pops();
}

Error MenuSingleton::population_menu_select_all_pop_filters() {
	bool changed = false;

	for (auto [pop_type, filter] : mutable_iterator(population_menu.pop_filters)) {
		if (!filter.selected) {
			filter.selected = true;
			changed = true;
		}
	}

	if (changed) {
		return _population_menu_update_filtered_pops();
	}

	return OK;
}

Error MenuSingleton::population_menu_deselect_all_pop_filters() {
	bool changed = false;
	for (auto [pop_type, filter] : mutable_iterator(population_menu.pop_filters)) {
		if (filter.selected) {
			filter.selected = false;
			changed = true;
		}
	}

	if (changed) {
		return _population_menu_update_filtered_pops();
	}

	return OK;
}

PackedStringArray MenuSingleton::get_population_menu_distribution_setup_info() const {
	static const PackedStringArray distribution_names = []() -> PackedStringArray {
		constexpr std::array<char const*, population_menu_t::DISTRIBUTION_COUNT> NAMES {
			/* Workforce (PopType)   */       "WORKFORCE_DISTTITLE",
			/* Religion              */        "RELIGION_DISTTITLE",
			/* Ideology              */        "IDEOLOGY_DISTTITLE",
			/* Nationality (Culture) */     "NATIONALITY_DISTTITLE",
			/* Issues                */ "DOMINANT_ISSUES_DISTTITLE",
			/* Vote                  */      "ELECTORATE_DISTTITLE"
		};

		PackedStringArray array;
		ERR_FAIL_COND_V(array.resize(NAMES.size()) != OK, {});

		for (int32_t idx = 0; idx < array.size(); ++idx) {
			array[idx] = NAMES[idx];
		}

		return array;
	}();

	return distribution_names;
}

TypedArray<Array> MenuSingleton::get_population_menu_distribution_info() const {
	TypedArray<Array> array;
	ERR_FAIL_COND_V(array.resize(population_menu_t::DISTRIBUTION_COUNT) != OK, {});

	const auto make_pie_chart_tooltip = [this](
		HasGetIdentifierAndGetColour auto const* key, String const& identifier, float weight, float total_weight
	) -> String {
		static const String format_key =
			GUILabel::get_colour_marker() + String { "Y%s" } + GUILabel::get_colour_marker() + "!: %s%%";
		return  vformat(
			format_key,
			tr(identifier),
			Utilities::float_to_string_dp(100.0f * weight / total_weight, 2)
		);
	};

	array[0] = GFXPieChartTexture::distribution_to_slices_array(population_menu.workforce_distribution, make_pie_chart_tooltip);
	array[1] = GFXPieChartTexture::distribution_to_slices_array(population_menu.religion_distribution, make_pie_chart_tooltip);
	array[2] = GFXPieChartTexture::distribution_to_slices_array(population_menu.ideology_distribution, make_pie_chart_tooltip);
	array[3] = GFXPieChartTexture::distribution_to_slices_array(population_menu.culture_distribution, make_pie_chart_tooltip);
	array[4] = GFXPieChartTexture::distribution_to_slices_array(
		population_menu.issue_distribution, make_pie_chart_tooltip, get_issue_identifier_suffix()
	);
	array[5] = GFXPieChartTexture::distribution_to_slices_array(population_menu.vote_distribution, make_pie_chart_tooltip);

	return array;
}
