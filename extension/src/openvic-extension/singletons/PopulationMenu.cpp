#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/DefinitionManager.hpp>
#include <openvic-simulation/InstanceManager.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::std_view_to_godot_string;

/* POPULATION MENU */

bool MenuSingleton::_population_menu_update_provinces() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, false);
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, false);

	population_menu.province_list_entries.clear();
	population_menu.visible_province_list_entries = 0;
	ERR_FAIL_COND_V(!_population_menu_generate_pop_filters(), false);

	MapInstance const& map_instance = instance_manager->get_map_instance();
	ERR_FAIL_COND_V(!map_instance.province_instances_are_locked(), false);

	for (CountryDefinition const* country : {
		// Example country
		game_singleton->get_definition_manager().get_country_definition_manager().get_country_definition_by_identifier("ENG")
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

	population_menu.sort_key = population_menu_t::NONE;

	emit_signal(_signal_population_menu_province_list_changed());

	// TODO - may need to emit population_menu_province_list_selected_changed if _update_info cannot be guaranteed

	_population_menu_update_pops();

	return true;
}

int32_t MenuSingleton::get_population_menu_province_list_row_count() const {
	return population_menu.visible_province_list_entries;
}

TypedArray<Dictionary> MenuSingleton::get_population_menu_province_list_rows(int32_t start, int32_t count) const {
	// TODO - remove when country population is used instead of total map population
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

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

		// TODO - remove when country population is used instead of total map population
		const Pop::pop_size_t total_map_population;

		/* This is the index among all entries, not just visible ones unlike start and count. */
		int32_t index = 0;

		bool is_expanded = true;

		TypedArray<Dictionary> array {};

		/* Overloads return false if count_counter reaches 0 and the function should return,
		 * otherwise true indicating the province list loop should continue. */

		bool operator()(population_menu_t::country_entry_t const& country_entry) {
			if (start_counter-- <= 0) {
				Dictionary country_dict;

				country_dict[type_key] = population_menu_t::LIST_ENTRY_COUNTRY;
				country_dict[index_key] = index;
				country_dict[name_key] = std_view_to_godot_string(country_entry.country.get_identifier());
				country_dict[size_key] = total_map_population;
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

				state_dict[type_key] = population_menu_t::LIST_ENTRY_STATE;
				state_dict[index_key] = index;
				state_dict[name_key] = menu_singleton.get_state_name(state_entry.state);
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

				province_dict[type_key] = population_menu_t::LIST_ENTRY_PROVINCE;
				province_dict[index_key] = index;
				province_dict[name_key] = std_view_to_godot_string(province_entry.province.get_identifier());
				province_dict[size_key] = province_entry.province.get_total_population();
				province_dict[change_key] = 0;
				province_dict[selected_key] = province_entry.selected;

				array.push_back(province_dict);

				return --count_counter > 0;
			}

			return  true;
		}
	} entry_visitor { *this, start, count, instance_manager->get_map_instance().get_total_map_population() };

	while (entry_visitor.index < population_menu.province_list_entries.size()
		&& std::visit(entry_visitor, population_menu.province_list_entries[entry_visitor.index])) {
		entry_visitor.index++;
	}

	return entry_visitor.array;
}

Error MenuSingleton::population_menu_select_province_list_entry(int32_t select_index, bool set_scroll_index) {
	ERR_FAIL_INDEX_V(select_index, population_menu.province_list_entries.size(), FAILED);

	struct entry_visitor_t {

		const int32_t _select_index;

		int32_t index = 0, visible_index = 0;
		bool is_expanded = true;

		int32_t selected_visible_index = -1;

		using enum population_menu_t::ProvinceListEntry;
		population_menu_t::ProvinceListEntry select_level = LIST_ENTRY_NONE;

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

	_population_menu_update_pops();

	return OK;
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

		const int32_t _province_index;

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
			if (province_entry.province.get_province_definition().get_index() == _province_index) {

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

void MenuSingleton::_population_menu_update_pops() {
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
				population_menu_t::pop_filter_t& filter = population_menu.pop_filters[&pop.get_type()];
				filter.count += pop.get_size();
				// TODO - set filter.promotion_demotion_change
			}
		}
	}

	_population_menu_update_filtered_pops();
}

void MenuSingleton::_population_menu_update_filtered_pops() {
	population_menu.filtered_pops.clear();

	for (fixed_point_map_t<HasIdentifierAndColour const*>& distribution : population_menu.distributions) {
		distribution.clear();
	}

	for (Pop const* pop : population_menu.pops) {
		if (population_menu.pop_filters[&pop->get_type()].selected) {
			population_menu.filtered_pops.push_back(pop);
		}
	}

	for (Pop const* pop : population_menu.filtered_pops) {
		population_menu.distributions[0][&pop->get_type()] += pop->get_size();
		population_menu.distributions[1][&pop->get_religion()] += pop->get_size();
		population_menu.distributions[2] +=
			cast_map<HasIdentifierAndColour>(pop->get_ideologies() * static_cast<int32_t>(pop->get_size()));
		population_menu.distributions[3][&pop->get_culture()] += pop->get_size();
		population_menu.distributions[4] +=
			cast_map<HasIdentifierAndColour>(pop->get_issues() * static_cast<int32_t>(pop->get_size()));
		population_menu.distributions[5] +=
			cast_map<HasIdentifierAndColour>(pop->get_votes() * static_cast<int32_t>(pop->get_size()));
	}

	for (fixed_point_map_t<HasIdentifierAndColour const*>& distribution : population_menu.distributions) {
		normalise_fixed_point_map(distribution);
	}

	_population_menu_sort_pops();
}

template<std::derived_from<HasIdentifier> T>
static bool compare_translated_identifiers(Object const& object, T const& lhs, T const& rhs) {
	return object.tr(std_view_to_godot_string(lhs.get_identifier()))
		< object.tr(std_view_to_godot_string(rhs.get_identifier()));
}

template<std::derived_from<HasIdentifier> T>
static bool compare_translated_identifiers(Object const& object, T const* lhs, T const* rhs) {
	return (lhs != nullptr ? object.tr(std_view_to_godot_string(lhs->get_identifier())) : godot::String {})
		< (rhs != nullptr ? object.tr(std_view_to_godot_string(rhs->get_identifier())) : godot::String {});
}

MenuSingleton::sort_func_t MenuSingleton::_get_population_menu_sort_func(population_menu_t::PopSortKey sort_key) const {
	using enum population_menu_t::PopSortKey;
	switch (sort_key) {
	case SORT_SIZE:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_size() < b->get_size();
		};
	case SORT_TYPE:
		return [this](Pop const* a, Pop const* b) -> bool {
			return compare_translated_identifiers(*this, a->get_type(), b->get_type());
		};
	case SORT_CULTURE:
		return [this](Pop const* a, Pop const* b) -> bool {
			return compare_translated_identifiers(*this, a->get_culture(), b->get_culture());
		};
	case SORT_RELIGION:
		return [this](Pop const* a, Pop const* b) -> bool {
			return compare_translated_identifiers(*this, a->get_religion(), b->get_religion());
		};
	case SORT_LOCATION:
		return [this](Pop const* a, Pop const* b) -> bool {
			return compare_translated_identifiers(*this, a->get_location(), b->get_location());
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
			return sorted_fixed_map_less_than(a->get_ideologies(), b->get_ideologies());
		};
	case SORT_ISSUES:
		return [](Pop const* a, Pop const* b) -> bool {
			return sorted_fixed_map_less_than(a->get_issues(), b->get_issues());
		};
	case SORT_UNEMPLOYMENT:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_unemployment() < b->get_unemployment();
		};
	case SORT_CASH:
		return [](Pop const* a, Pop const* b) -> bool {
			return a->get_cash() < b->get_cash();
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
			return compare_translated_identifiers(*this, a->get_rebel_type(), b->get_rebel_type());
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

void MenuSingleton::_population_menu_sort_pops() {
	if (population_menu.sort_key != population_menu_t::NONE) {
		const sort_func_t base_sort_func = _get_population_menu_sort_func(population_menu.sort_key);

		const sort_func_t sort_func = population_menu.sort_descending
			? base_sort_func
			: [base_sort_func](Pop const* a, Pop const* b) { return base_sort_func(b, a); };

		std::sort(population_menu.filtered_pops.begin(), population_menu.filtered_pops.end(), sort_func);
	}

	emit_signal(_signal_population_menu_pops_changed());
}

Error MenuSingleton::population_menu_select_sort_key(population_menu_t::PopSortKey sort_key) {
	using enum population_menu_t::PopSortKey;
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

	_population_menu_sort_pops();

	return OK;
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
		pop_dict[pop_type_icon_key] = pop->get_type().get_sprite();
		pop_dict[pop_culture_key] = std_view_to_godot_string(pop->get_culture().get_identifier());
		pop_dict[pop_religion_icon_key] = pop->get_religion().get_icon();
		if (pop->get_location() != nullptr) {
			pop_dict[pop_location_key] = std_view_to_godot_string(pop->get_location()->get_identifier());
		}
		pop_dict[pop_militancy_key] = pop->get_militancy().to_float();
		pop_dict[pop_consciousness_key] = pop->get_consciousness().to_float();
		pop_dict[pop_ideology_key] = GFXPieChartTexture::distribution_to_slices_array(pop->get_ideologies());
		pop_dict[pop_issues_key] = GFXPieChartTexture::distribution_to_slices_array(pop->get_issues());
		pop_dict[pop_unemployment_key] = pop->get_unemployment().to_float();
		pop_dict[pop_cash_key] = pop->get_cash().to_float();
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

bool MenuSingleton::_population_menu_generate_pop_filters() {
	if (population_menu.pop_filters.empty()) {
		GameSingleton const* game_singleton = GameSingleton::get_singleton();
		ERR_FAIL_NULL_V(game_singleton, false);

		for (PopType const& pop_type : game_singleton->get_definition_manager().get_pop_manager().get_pop_types()) {
			population_menu.pop_filters.emplace(&pop_type, population_menu_t::pop_filter_t { 0, 0, true });
		}

		ERR_FAIL_COND_V_MSG(population_menu.pop_filters.empty(), false, "Failed to generate population menu pop filters!");
	}

	return true;
}

PackedInt32Array MenuSingleton::get_population_menu_pop_filter_setup_info() {
	ERR_FAIL_COND_V(!_population_menu_generate_pop_filters(), {});

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

	_population_menu_update_filtered_pops();

	return OK;
}

void MenuSingleton::population_menu_select_all_pop_filters() {
	bool changed = false;

	for (auto [pop_type, filter] : mutable_iterator(population_menu.pop_filters)) {
		if (!filter.selected) {
			filter.selected = true;
			changed = true;
		}
	}

	if (changed) {
		_population_menu_update_filtered_pops();
	}
}

void MenuSingleton::population_menu_deselect_all_pop_filters() {
	bool changed = false;
	for (auto [pop_type, filter] : mutable_iterator(population_menu.pop_filters)) {
		if (filter.selected) {
			filter.selected = false;
			changed = true;
		}
	}
	if (changed) {
		_population_menu_update_filtered_pops();
	}
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
	ERR_FAIL_COND_V(array.resize(population_menu.distributions.size()) != OK, {});

	for (int32_t idx = 0; idx < array.size(); ++idx) {
		array[idx] = GFXPieChartTexture::distribution_to_slices_array(population_menu.distributions[idx]);
	}

	return array;
}
