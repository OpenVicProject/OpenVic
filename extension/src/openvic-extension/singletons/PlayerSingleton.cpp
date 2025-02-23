#include "PlayerSingleton.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/map/ProvinceInstance.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/MenuSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

/* StringNames cannot be constructed until Godot has called StringName::setup(),
 * so we must use these wrapper functions to delay their initialisation. */
StringName const& PlayerSingleton::_signal_province_selected() {
	static const StringName signal_province_selected = "province_selected";
	return signal_province_selected;
}

void PlayerSingleton::_bind_methods() {
	// Player country
	OV_BIND_METHOD(PlayerSingleton::set_player_country_by_province_index, { "province_index" });
	OV_BIND_METHOD(PlayerSingleton::get_player_country_capital_position);

	// Selected province
	OV_BIND_METHOD(PlayerSingleton::set_selected_province_by_index, { "province_index" });
	OV_BIND_METHOD(PlayerSingleton::unset_selected_province);
	OV_BIND_METHOD(PlayerSingleton::get_selected_province_index);

	// Core
	OV_BIND_METHOD(PlayerSingleton::toggle_paused);
	OV_BIND_METHOD(PlayerSingleton::increase_speed);
	OV_BIND_METHOD(PlayerSingleton::decrease_speed);

	// Production
	OV_BIND_METHOD(PlayerSingleton::expand_selected_province_building, { "building_index" });

	// Budget

	// Technology
	OV_BIND_METHOD(PlayerSingleton::start_research, { "technology_identifier" });

	// Politics

	// Population

	// Trade
	OV_BIND_METHOD(PlayerSingleton::set_good_automated, { "good_index", "is_automated" });
	OV_BIND_METHOD(PlayerSingleton::set_good_trade_order, { "good_index", "is_selling", "amount_slider" });

	// Diplomacy

	// Military
	OV_BIND_METHOD(PlayerSingleton::create_leader, { "is_general" });
	OV_BIND_METHOD(PlayerSingleton::set_can_use_leader, { "leader_id", "can_use" });
	OV_BIND_METHOD(PlayerSingleton::set_auto_create_leaders, { "value" });
	OV_BIND_METHOD(PlayerSingleton::set_auto_assign_leaders, { "value" });
	OV_BIND_METHOD(PlayerSingleton::set_mobilise, { "value" });

	ADD_SIGNAL(MethodInfo(_signal_province_selected(), PropertyInfo(Variant::INT, "index")));
}

PlayerSingleton* PlayerSingleton::get_singleton() {
	return singleton;
}

PlayerSingleton::PlayerSingleton() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

PlayerSingleton::~PlayerSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

// Player country
void PlayerSingleton::set_player_country(CountryInstance const* new_player_country) {
	if (OV_unlikely(player_country == new_player_country)) {
		return;
	}

	GameSingleton& game_singleton = *GameSingleton::get_singleton();
	InstanceManager* instance_manager = game_singleton.get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	if (player_country != nullptr) {
		instance_manager->queue_game_action(
			game_action_type_t::GAME_ACTION_SET_AI,
			std::pair<uint64_t, bool> { player_country->get_index(), true }
		);
	}

	player_country = new_player_country;

	if (player_country != nullptr) {
		instance_manager->queue_game_action(
			game_action_type_t::GAME_ACTION_SET_AI,
			std::pair<uint64_t, bool> { player_country->get_index(), false }
		);
	}

	Logger::info("Set player country to: ", player_country != nullptr ? player_country->get_identifier() : "<NULL>");

	game_singleton._on_gamestate_updated();
}

void PlayerSingleton::set_player_country_by_province_index(int32_t province_index) {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	ProvinceInstance* province_instance = instance_manager->get_map_instance().get_province_instance_by_index(province_index);
	ERR_FAIL_NULL(province_instance);

	set_player_country(province_instance->get_owner());
}

Vector2 PlayerSingleton::get_player_country_capital_position() const {
	if (player_country != nullptr) {
		ProvinceInstance const* capital = player_country->get_capital();

		if (capital != nullptr) {
			return GameSingleton::get_singleton()->get_billboard_pos(capital->get_province_definition());
		}
	}

	return {};
}

// Selected province
void PlayerSingleton::set_selected_province(ProvinceInstance const* new_selected_province) {
	if (selected_province != new_selected_province) {
		selected_province = new_selected_province;

		GameSingleton::get_singleton()->_update_colour_image();

		emit_signal(_signal_province_selected(), get_selected_province_index());
	}
}

void PlayerSingleton::set_selected_province_by_index(int32_t province_index) {
	if (province_index == ProvinceDefinition::NULL_INDEX) {
		unset_selected_province();
	} else {
		InstanceManager const* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
		ERR_FAIL_NULL(instance_manager);

		MapInstance const& map_instance = instance_manager->get_map_instance();

		set_selected_province(map_instance.get_province_instance_by_index(province_index));

		if (selected_province == nullptr) {
			Logger::error(
				"Trying to set selected province to an invalid index ", province_index, " (max index is ",
				map_instance.get_province_instance_count(), ")"
			);
		}
	}
}

void PlayerSingleton::unset_selected_province() {
	set_selected_province(nullptr);
}

int32_t PlayerSingleton::get_selected_province_index() const {
	return selected_province != nullptr ? selected_province->get_index() : ProvinceDefinition::NULL_INDEX;
}

// Core
void PlayerSingleton::toggle_paused() const {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_PAUSE,
		!instance_manager->get_simulation_clock().is_paused()
	);
}

void PlayerSingleton::increase_speed() const {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_SPEED,
		instance_manager->get_simulation_clock().get_simulation_speed() + 1
	);
}

void PlayerSingleton::decrease_speed() const {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_SPEED,
		instance_manager->get_simulation_clock().get_simulation_speed() - 1
	);
}

// Production
void PlayerSingleton::expand_selected_province_building(int32_t building_index) const {
	ERR_FAIL_NULL(selected_province);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_EXPAND_PROVINCE_BUILDING,
		std::pair<uint64_t, uint64_t> { selected_province->get_index(), building_index }
	);
}

// Budget

// Technology
void PlayerSingleton::start_research(String const& technology_identifier) const {
	ERR_FAIL_NULL(player_country);

	GameSingleton& game_singleton = *GameSingleton::get_singleton();

	Technology const* technology =
		game_singleton.get_definition_manager().get_research_manager().get_technology_manager().get_technology_by_identifier(
			Utilities::godot_to_std_string(technology_identifier)
		);
	ERR_FAIL_NULL(technology);

	InstanceManager* instance_manager = game_singleton.get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_START_RESEARCH,
		std::pair<uint64_t, uint64_t> { player_country->get_index(), technology->get_index() }
	);
}

// Politics

// Population

// Trade
void PlayerSingleton::set_good_automated(int32_t good_index, bool is_automated) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_GOOD_AUTOMATED,
		std::tuple<uint64_t, uint64_t, bool> { player_country->get_index(), good_index, is_automated }
	);
}

void PlayerSingleton::set_good_trade_order(int32_t good_index, bool is_selling, GUIScrollbar const* amount_slider) const {
	ERR_FAIL_NULL(amount_slider);
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_GOOD_TRADE_ORDER, std::tuple<uint64_t, uint64_t, bool, fixed_point_t> {
			player_country->get_index(), good_index, is_selling,
			MenuSingleton::calculate_trade_menu_stockpile_cutoff_amount_fp(amount_slider->get_value_scaled_fp())
		}
	);
}

// Diplomacy

// Military
void PlayerSingleton::create_leader(bool is_general) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_CREATE_LEADER,
		std::pair<uint64_t, bool> { player_country->get_index(), is_general }
	);
}

void PlayerSingleton::set_can_use_leader(uint64_t leader_id, bool can_use) const {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_USE_LEADER,
		std::pair<uint64_t, bool> { leader_id, can_use }
	);
}

void PlayerSingleton::set_auto_create_leaders(bool value) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_AUTO_CREATE_LEADERS,
		std::pair<uint64_t, bool> { player_country->get_index(), value }
	);
}

void PlayerSingleton::set_auto_assign_leaders(bool value) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_AUTO_ASSIGN_LEADERS,
		std::pair<uint64_t, bool> { player_country->get_index(), value }
	);
}

void PlayerSingleton::set_mobilise(bool value) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action(
		game_action_type_t::GAME_ACTION_SET_MOBILISE,
		std::pair<uint64_t, bool> { player_country->get_index(), value }
	);
}
