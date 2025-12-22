#include "MenuSingleton.hpp"

#include <cstdint>
#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/economy/GoodDefinition.hpp>
#include <openvic-simulation/GameManager.hpp>
#include <openvic-simulation/modifier/Modifier.hpp>
#include <openvic-simulation/types/fixed_point/FixedPoint.hpp>

#include "openvic-extension/classes/GFXPieChartTexture.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"
#include "openvic-simulation/country/CountryInstance.hpp"
#include "openvic-simulation/politics/Ideology.hpp"
#include "openvic-simulation/politics/Issue.hpp"
#include "openvic-simulation/pop/PopType.hpp"
#include "openvic-simulation/pop/Religion.hpp"
#include "openvic-simulation/types/HasIdentifier.hpp"
#include "openvic-simulation/types/IndexedMap.hpp"

using namespace godot;
using namespace OpenVic;


String country_status_to_string(CountryInstance::country_status_t status) {
	switch (status) {
		case CountryInstance::country_status_t::COUNTRY_STATUS_CIVILISED:
			return "Civilised Nation";
		case CountryInstance::country_status_t::COUNTRY_STATUS_UNCIVILISED:
			return "Uncivilised Nation";
		case CountryInstance::country_status_t::COUNTRY_STATUS_PARTIALLY_CIVILISED:
			return "Partially Westernised Nation";
		case CountryInstance::country_status_t::COUNTRY_STATUS_PRIMITIVE:
			return "Primitive Nation";
		case CountryInstance::country_status_t::COUNTRY_STATUS_GREAT_POWER:
			return "Great Power";
		case CountryInstance::country_status_t::COUNTRY_STATUS_SECONDARY_POWER:
			return "Secondary Power";
		default:
			return "";
	}
}

TypedArray<Array> MenuSingleton::get_nation_rankings() {
	TypedArray<Array> ret = TypedArray<Array>();
	std::vector<CountryInstance> const& countries = GameSingleton::get_singleton()
		->get_instance_manager()
		->get_country_instance_manager()
		.get_country_instances();

	for (CountryInstance const& country : countries) {
		if (!country.exists()) continue;

		const String country_name = get_country_name(country);
		const String country_status = country_status_to_string(country.get_country_status());
		const int32_t military_score = country.get_military_power();
		const int32_t industrial_score = country.get_industrial_power();
		const int32_t prestige_score = country.get_prestige();
		const int32_t total_score = military_score + industrial_score + prestige_score;

		Array info;
		info.append(country_name);
		info.append(country_status);
		info.append(military_score);
		info.append(industrial_score);
		info.append(prestige_score);
		info.append(total_score);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_nation_comparison() {
	TypedArray<Array> ret = TypedArray<Array>();
	std::vector<CountryInstance> const& countries = GameSingleton::get_singleton()
		->get_instance_manager()
		->get_country_instance_manager()
		.get_country_instances();

	for (CountryInstance const& country : countries) {
		if (!country.exists()) continue;

		const String country_name = get_country_name(country);
		const int32_t total_pop = country.get_total_population();
		const int32_t province_count = country.get_owned_provinces().size();
		const int32_t factory_count = -1;
		const int32_t literacy = country.get_national_literacy();
		const int32_t leadership = country.get_leader_count();
		const int32_t brigade_count = -1;
		const int32_t ship_count = country.get_ship_count();

		Array info;
		info.append(country_name);
		info.append(total_pop);
		info.append(province_count);
		info.append(factory_count);
		info.append(literacy);
		info.append(leadership);
		info.append(brigade_count);
		info.append(ship_count);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_political_systems() {
	TypedArray<Array> ret = TypedArray<Array>();
	std::vector<CountryInstance> const& countries = GameSingleton::get_singleton()
		->get_instance_manager()
		->get_country_instance_manager()
		.get_country_instances();

	for (CountryInstance const& country : countries) {
		if (!country.exists()) continue;

		const String country_name = get_country_name(country);
		const String government = country.get_government_type()->get_identifier().data();
		const String national_value = country.get_national_value()->get_identifier().data();
		const String ruling_party = country.get_ruling_party()->get_identifier().data();
		const String party_ideology = country.get_ruling_party()->get_ideology()->get_identifier().data();

		Array info;
		info.append(country_name);
		info.append(government);
		info.append(national_value);
		info.append(ruling_party);
		info.append(party_ideology);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_political_reforms() {
	TypedArray<Array> ret = TypedArray<Array>();
	std::vector<CountryInstance> const& countries = GameSingleton::get_singleton()
		->get_instance_manager()
		->get_country_instance_manager()
		.get_country_instances();

	for (CountryInstance const& country : countries) {
		if (!country.exists()) continue;

		auto reforms = country.get_reforms();
		IssueManager const& issue_man = GameSingleton::get_singleton()
			->get_definition_manager()
			.get_politics_manager()
			.get_issue_manager();

		const String country_name = get_country_name(country);
		// const String slavery = reforms.get_item_by_key(*issue_man.get_reform_group_by_identifier("slavery"));
		const String slavery = "idk";
		const String vote_franchise = "idk";
		const String upper_house = "idk";
		const String voting_system = "idk";
		const String public_meetings = "idk";
		const String press_rights = "idk";
		const String trade_unions = "idk";
		const String political_parties = "idk";

		Array info;
		info.append(country_name);
		info.append(slavery);
		info.append(vote_franchise);
		info.append(upper_house);
		info.append(voting_system);
		info.append(public_meetings);
		info.append(press_rights);
		info.append(trade_unions);
		info.append(political_parties);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_social_reforms() {
	TypedArray<Array> ret = TypedArray<Array>();
	std::vector<CountryInstance> const& countries = GameSingleton::get_singleton()
		->get_instance_manager()
		->get_country_instance_manager()
		.get_country_instances();

	for (CountryInstance const& country : countries) {
		if (!country.exists()) continue;

		const String country_name = get_country_name(country);
		const String minimum_wage = "idk";
		const String max_workhours = "idk";
		const String safety_regulations = "idk";
		const String unemployment_benefits = "idk";
		const String pensions = "idk";
		const String healthcare = "idk";
		const String school_system = "idk";

		Array info;
		info.append(country_name);
		info.append(minimum_wage);
		info.append(max_workhours);
		info.append(safety_regulations);
		info.append(unemployment_benefits);
		info.append(pensions);
		info.append(healthcare);
		info.append(school_system);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_country_population() {
	TypedArray<Array> ret = TypedArray<Array>();
	std::vector<CountryInstance> const& countries = GameSingleton::get_singleton()
		->get_instance_manager()
		->get_country_instance_manager()
		.get_country_instances();

	for (CountryInstance const& country : countries) {
		if (!country.exists()) continue;

		PopManager const& pop_man = GameSingleton::get_singleton()->get_definition_manager().get_pop_manager();
		const String country_name = get_country_name(country);
		const int32_t aristocrats = country.get_strata_population(*pop_man.get_strata_by_index(0));
		const int32_t artisans = country.get_strata_population(*pop_man.get_strata_by_index(10));
		const int32_t bureaucrats = country.get_strata_population(*pop_man.get_strata_by_index(11));
		const int32_t capitalists = country.get_strata_population(*pop_man.get_strata_by_index(3));
		const int32_t clergy = country.get_strata_population(*pop_man.get_strata_by_index(2));
		const int32_t clerks = country.get_strata_population(*pop_man.get_strata_by_index(4));
		const int32_t craftsmen = country.get_strata_population(*pop_man.get_strata_by_index(5));
		const int32_t farmers = country.get_strata_population(*pop_man.get_strata_by_index(7));
		const int32_t laborers = country.get_strata_population(*pop_man.get_strata_by_index(8));
		const int32_t officers = country.get_strata_population(*pop_man.get_strata_by_index(1));
		const int32_t slaves = country.get_strata_population(*pop_man.get_strata_by_index(9));
		const int32_t soldiers = country.get_strata_population(*pop_man.get_strata_by_index(6));

		Array info;
		info.append(country_name);
		info.append(aristocrats);
		info.append(artisans);
		info.append(bureaucrats);
		info.append(capitalists);
		info.append(clergy);
		info.append(clerks);
		info.append(craftsmen);
		info.append(farmers);
		info.append(laborers);
		info.append(officers);
		info.append(slaves);
		info.append(soldiers);

		ret.append(info);
	}
	return ret;
}

template<typename T>
T get_dominant(ordered_map<T, fixed_point_t> map) {
	fixed_point_t max = 0;
	T dominant;
	for (const auto& dist : map) {
		if (dist.second > max) {
			dominant = dist.first;
			max = dist.second;
		}
	}
	return dominant;
}

template<typename T>
const T* get_dominant(IndexedMap<T, fixed_point_t> map) {
	fixed_point_t max = 0;
	const T* dominant;
	for (const auto& dist : map) {
		if (dist.second > max) {
			dominant = &dist.first;
			max = dist.second;
		}
	}
	return dominant;
}

TypedArray<Array> MenuSingleton::get_provinces() {
	TypedArray<Array> ret = TypedArray<Array>();
	const CountryInstance* player_country = GameSingleton::get_singleton()->get_viewed_country();
	if (player_country == nullptr) return ret;

	ordered_set<ProvinceInstance*> const& provinces = player_country->get_owned_provinces();

	for (ProvinceInstance* const& province : provinces) {
		const String province_name = province->get_province_definition().get_identifier().data();
		const int32_t total_pop = province->get_total_population();
		const float avg_mil = province->get_average_militancy().to_float();
		const float avg_con = province->get_average_consciousness().to_float();
		const float avg_lit = province->get_average_literacy().to_float();
		const String religion = get_dominant(province->get_religion_distribution())->get_identifier().data();
		const String culture = get_dominant(province->get_culture_distribution())->get_identifier().data();
		const String issue = get_dominant(province->get_issue_distribution())->get_identifier().data();
		const String ideology = get_dominant(province->get_ideology_distribution())->get_identifier().data();

		Array info;
		info.append(province_name);
		info.append(total_pop);
		info.append(avg_mil);
		info.append(avg_con);
		info.append(avg_lit);
		info.append(religion);
		info.append(culture);
		info.append(issue);
		info.append(ideology);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_province_population() {
	TypedArray<Array> ret = TypedArray<Array>();
	const CountryInstance* player_country = GameSingleton::get_singleton()->get_viewed_country();
	if (player_country == nullptr) return ret;

	ordered_set<ProvinceInstance*> const& provinces = player_country->get_owned_provinces();

	for (ProvinceInstance* const& province : provinces) {
		PopManager const& pop_man = GameSingleton::get_singleton()->get_definition_manager().get_pop_manager();
		const String province_name = province->get_province_definition().get_identifier().data();
		const int32_t aristocrats = province->get_strata_population(*pop_man.get_strata_by_index(0));
		const int32_t artisans = province->get_strata_population(*pop_man.get_strata_by_index(10));
		const int32_t bureaucrats = province->get_strata_population(*pop_man.get_strata_by_index(11));
		const int32_t capitalists = province->get_strata_population(*pop_man.get_strata_by_index(3));
		const int32_t clergy = province->get_strata_population(*pop_man.get_strata_by_index(2));
		const int32_t clerks = province->get_strata_population(*pop_man.get_strata_by_index(4));
		const int32_t craftsmen = province->get_strata_population(*pop_man.get_strata_by_index(5));
		const int32_t farmers = province->get_strata_population(*pop_man.get_strata_by_index(7));
		const int32_t laborers = province->get_strata_population(*pop_man.get_strata_by_index(8));
		const int32_t officers = province->get_strata_population(*pop_man.get_strata_by_index(1));
		const int32_t slaves = province->get_strata_population(*pop_man.get_strata_by_index(9));
		const int32_t soldiers = province->get_strata_population(*pop_man.get_strata_by_index(6));

		Array info;
		info.append(province_name);
		info.append(aristocrats);
		info.append(artisans);
		info.append(bureaucrats);
		info.append(capitalists);
		info.append(clergy);
		info.append(clerks);
		info.append(craftsmen);
		info.append(farmers);
		info.append(laborers);
		info.append(officers);
		info.append(slaves);
		info.append(soldiers);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_province_production() {
	TypedArray<Array> ret = TypedArray<Array>();
	const CountryInstance* player_country = GameSingleton::get_singleton()->get_viewed_country();
	if (player_country == nullptr) return ret;

	ordered_set<ProvinceInstance*> const& provinces = player_country->get_owned_provinces();

	for (ProvinceInstance* const& province : provinces) {
		const String province_name = province->get_province_definition().get_identifier().data();
		const String state_name = get_state_name(*province->get_state());
		const String goods = province->get_rgo_good()->get_identifier().data();
		const float output = province->get_rgo().get_output_quantity_yesterday().to_float();
		const float income = province->get_rgo().get_revenue_yesterday().to_float();
		const int32_t employed = province->get_rgo().get_total_employees_count_cache();
		const int32_t level = province->get_rgo().get_size_multiplier();

		Array info;
		info.append(province_name);
		info.append(state_name);
		info.append(goods);
		info.append(output);
		info.append(income);
		info.append(employed);
		info.append(level);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_factory_production() {
	TypedArray<Array> ret = TypedArray<Array>();
	const CountryInstance* player_country = GameSingleton::get_singleton()->get_viewed_country();
	if (player_country == nullptr) return ret;

	ordered_set<State*> const& states = player_country->get_states();

	for (State* const& state : states) {
		const String state_name = get_state_name(*state);
		const String goods = "idk";
		const int32_t output = -1;
		const int32_t income = -1;
		const int32_t employed = -1;
		const int32_t level = -1;

		Array info;
		info.append(state_name);
		info.append(goods);
		info.append(output);
		info.append(income);
		info.append(employed);
		info.append(level);

		ret.append(info);
	}
	return ret;
}

TypedArray<Array> MenuSingleton::get_ledger_data(int32_t page_index) {
	switch (page_index) {
		case 0:
			return get_nation_rankings();
		case 1:
			return get_nation_comparison();
		case 2:
			return get_political_systems();
		case 3:
			return get_political_reforms();
		case 4:
			return get_social_reforms();
		case 5:
			return get_country_population();
		case 6:
			return get_provinces();
		case 7:
			return get_province_population();
		case 8:
			return get_province_production();
		case 9:
			return get_factory_production();
		case 10:
			break;
		default:
			ERR_FAIL_V_MSG(TypedArray<Array>(), vformat("invalid ledger page_index '%d'", page_index));
	}
	return TypedArray<Array>();
}
