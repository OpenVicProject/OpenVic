#include "openvic2/map/Building.hpp"

#include <cassert>

#include "openvic2/Logger.hpp"

using namespace OpenVic2;

Building::Building(BuildingType const& new_type) : type{ new_type } {}

bool Building::_can_expand() const {
	return level < type.get_max_level();
}

BuildingType const& Building::get_type() const {
	return type;
}

Building::level_t Building::get_level() const {
	return level;
}

Building::ExpansionState Building::get_expansion_state() const {
	return expansion_state;
}

Date const& Building::get_start_date() const {
	return start;
}

Date const& Building::get_end_date() const {
	return end;
}

float Building::get_expansion_progress() const {
	return expansion_progress;
}

return_t Building::expand() {
	if (expansion_state == ExpansionState::CanExpand) {
		expansion_state = ExpansionState::Preparing;
		expansion_progress = 0.0f;
		return SUCCESS;
	}
	return FAILURE;
}

void Building::update_state(Date const& today) {
	switch (expansion_state) {
		case ExpansionState::Preparing:
			start = today;
			end = start + type.get_build_time();
			break;
		case ExpansionState::Expanding:
			expansion_progress = static_cast<double>(today - start) / static_cast<double>(end - start);
			break;
		default: expansion_state = _can_expand() ? ExpansionState::CanExpand : ExpansionState::CannotExpand;
	}
}

void Building::tick(Date const& today) {
	if (expansion_state == ExpansionState::Preparing) {
		expansion_state = ExpansionState::Expanding;
	}
	if (expansion_state == ExpansionState::Expanding) {
		if (end <= today) {
			level++;
			expansion_state = ExpansionState::CannotExpand;
		}
	}
}

BuildingType::BuildingType(std::string const& new_identifier, Building::level_t new_max_level, Timespan new_build_time) :
	HasIdentifier{ new_identifier }, max_level{ new_max_level }, build_time{ new_build_time } {
	assert(new_max_level >= 0);
	assert(build_time >= 0);
}

Building::level_t BuildingType::get_max_level() const {
	return max_level;
}

Timespan BuildingType::get_build_time() const {
	return build_time;
}

return_t BuildingManager::add_building_type(std::string const& identifier, Building::level_t max_level, Timespan build_time) {
	if (building_types_locked) {
		Logger::error("The building type list has already been locked!");
		return FAILURE;
	}
	if (identifier.empty()) {
		Logger::error("Empty building type identifier!");
		return FAILURE;
	}
	if (max_level < 0) {
		Logger::error("Invalid building type max level: ", max_level);
		return FAILURE;
	}
	if (build_time < 0) {
		Logger::error("Invalid building type build time: ", build_time);
		return FAILURE;
	}
	BuildingType new_building_type{ identifier, max_level, build_time };
	BuildingType const* old_building_type = get_building_type_by_identifier(identifier);
	if (old_building_type != nullptr) {
		Logger::error("Duplicate building type identifiers: ", old_building_type->get_identifier(), " and ", identifier);
		return FAILURE;
	}
	building_types.push_back(new_building_type);
	return SUCCESS;
}

void BuildingManager::lock_building_types() {
	building_types_locked = true;
	Logger::info("Locked building types after registering ", building_types.size());
}

BuildingType const* BuildingManager::get_building_type_by_identifier(std::string const& identifier) const {
	if (!identifier.empty())
		for (BuildingType const& building_type : building_types)
			if (building_type.get_identifier() == identifier) return &building_type;
	return nullptr;
}

void BuildingManager::generate_province_buildings(std::vector<Building>& buildings) const {
	for (BuildingType const& type : building_types)
		buildings.push_back(Building{ type });
}
