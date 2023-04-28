#include "openvic2/GameManager.hpp"

#include "openvic2/Logger.hpp"

using namespace OpenVic2;

GameManager::GameManager(state_updated_func_t state_updated_callback)
	: clock{ [this]() { tick(); }, [this]() { update_state(); } }, state_updated{ state_updated_callback } {}

void GameManager::set_needs_update() {
	needs_update = true;
}

void GameManager::update_state() {
	if (needs_update) {
		Logger::info("Update: ", today);
		map.update_state(today);
		if (state_updated) state_updated();
		needs_update = false;
	}
}

void GameManager::tick() {
	today++;
	Logger::info("Tick: ", today);
	map.tick(today);
	set_needs_update();
}

return_t GameManager::setup() {
	clock.reset();
	today = { 1836 };
	set_needs_update();
	return map.generate_province_buildings(building_manager);
}

Date const& GameManager::get_today() const {
	return today;
}

return_t GameManager::expand_building(index_t province_index, std::string const& building_type_identifier) {
	set_needs_update();
	Province* province = map.get_province_by_index(province_index);
	if (province == nullptr) return FAILURE;
	return province->expand_building(building_type_identifier);
}
