#pragma once

#include <godot_cpp/core/error_macros.hpp>

#include <openvic-simulation/types/TypedIndices.hpp>

#include "openvic-extension/singletons/GameSingleton.hpp"

namespace OpenVic {
	static TypedSpan<ideology_index_t, const Ideology> get_ideologies() {
		return GameSingleton::get_singleton()->get_definition_manager().get_politics_manager().get_ideology_manager().get_ideologies();
	}
	static Ideology const& get_ideology(const ideology_index_t i) {
		CRASH_BAD_INDEX_MSG(type_safe::get(i), type_safe::get(get_ideologies().size()), "get_ideology");
		return get_ideologies()[i];
	}
	
	static TypedSpan<party_policy_index_t, const PartyPolicy> get_party_policies() {
		return GameSingleton::get_singleton()->get_definition_manager().get_politics_manager().get_issue_manager().get_party_policies();
	}
	static PartyPolicy const& get_party_policy(const party_policy_index_t i) {
		CRASH_BAD_INDEX_MSG(type_safe::get(i), type_safe::get(get_party_policies().size()), "get_party_policy");
		return get_party_policies()[i];
	}

	static TypedSpan<pop_type_index_t, const PopType> get_pop_types() {
		return GameSingleton::get_singleton()->get_definition_manager().get_pop_manager().get_pop_types();
	}
	static PopType const& get_pop_type(const pop_type_index_t i) {
		CRASH_BAD_INDEX_MSG(type_safe::get(i), type_safe::get(get_pop_types().size()), "get_pop_types");
		return get_pop_types()[i];
	}

	static TypedSpan<province_index_t, const ProvinceDefinition> get_province_definitions() {
		return GameSingleton::get_singleton()->get_definition_manager().get_map_definition().get_province_definitions();
	}
	static ProvinceDefinition const& get_province_definition(const province_index_t i) {
		CRASH_BAD_INDEX_MSG(type_safe::get(i), type_safe::get(get_province_definitions().size()), "get_province_definition");
		return get_province_definitions()[i];
	}

	static TypedSpan<rebel_type_index_t, const RebelType> get_rebel_types() {
		return GameSingleton::get_singleton()->get_definition_manager().get_politics_manager().get_rebel_manager().get_rebel_types();
	}
	static RebelType const& get_rebel_type(const rebel_type_index_t i) {
		CRASH_BAD_INDEX_MSG(type_safe::get(i), type_safe::get(get_rebel_types().size()), "get_rebel_type");
		return get_rebel_types()[i];
	}
	
	static TypedSpan<reform_index_t, const Reform> get_reforms() {
		return GameSingleton::get_singleton()->get_definition_manager().get_politics_manager().get_issue_manager().get_reforms();
	}
	static Reform const& get_reform(const reform_index_t i) {
		CRASH_BAD_INDEX_MSG(type_safe::get(i), type_safe::get(get_reforms().size()), "get_reform");
		return get_reforms()[i];
	}
}