#include "PlayerSingleton.hpp"

#include <openvic-simulation/core/FormatValidate.hpp>
#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/map/ProvinceInstance.hpp>
#include <openvic-simulation/misc/GameAction.hpp>
#include <openvic-simulation/types/TypedIndices.hpp>

#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/MenuSingleton.hpp"
#include "openvic-extension/core/Bind.hpp"

using namespace OpenVic;
using namespace godot;

/* StringNames cannot be constructed until Godot has called StringName::setup(),
 * so we must use these wrapper functions to delay their initialisation. */
StringName const& PlayerSingleton::_signal_province_selected() {
	return OV_SNAME(province_selected);
}

void PlayerSingleton::_bind_methods() {
	OV_BIND_METHOD(PlayerSingleton::reset_player_singleton);

	// Player country
	OV_BIND_METHOD(PlayerSingleton::set_player_country_by_province_number, { "province_number" });
	OV_BIND_METHOD(PlayerSingleton::get_player_country_capital_position);

	// Selected province
	OV_BIND_METHOD(PlayerSingleton::set_selected_province_by_number, { "province_number" });
	OV_BIND_METHOD(PlayerSingleton::unset_selected_province);
	OV_BIND_METHOD(PlayerSingleton::get_selected_province_number);

	// Core
	OV_BIND_METHOD(PlayerSingleton::toggle_paused);
	OV_BIND_METHOD(PlayerSingleton::increase_speed);
	OV_BIND_METHOD(PlayerSingleton::decrease_speed);

	// Production
	OV_BIND_METHOD(PlayerSingleton::expand_selected_province_building, { "building_index" });

	// Budget

	// Technology

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

void PlayerSingleton::reset_player_singleton() {
	// This function is called when a game session ends, so we don't use functions like set_player_country or
	// unset_selected_province here as they can dereference leftover pointers from the previous game session which
	// are either already or soon to be invalid.
	player_country = nullptr;
	selected_province = nullptr;
}

// Player country
void PlayerSingleton::set_player_country(CountryInstance* new_player_country) {
	if (OV_unlikely(player_country == new_player_country)) {
		return;
	}

	GameSingleton& game_singleton = *GameSingleton::get_singleton();
	InstanceManager* instance_manager = game_singleton.get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	if (player_country != nullptr) {
		instance_manager->queue_game_action<set_ai_argument_t>(
			player_country->index, true
		);
	}

	player_country = new_player_country;

	if (player_country != nullptr) {
		instance_manager->queue_game_action<set_ai_argument_t>(
			player_country->index, false
		);
	}

	SPDLOG_INFO("Set player country to: {}", ovfmt::validate(player_country));

	game_singleton._on_gamestate_updated();
}

void PlayerSingleton::set_player_country_by_province_number(int32_t province_number) {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	ProvinceInstance* province_instance = instance_manager->get_map_instance().get_province_instance_from_number(province_number);
	ERR_FAIL_NULL(province_instance);

	set_player_country(province_instance->get_owner());
}

Vector2 PlayerSingleton::get_player_country_capital_position() const {
	if (player_country != nullptr) {
		ProvinceInstance const* capital = player_country->get_capital();

		if (capital != nullptr) {
			return GameSingleton::get_singleton()->get_billboard_pos(capital->province_definition);
		}
	}

	return {};
}

// Selected province
void PlayerSingleton::set_selected_province(ProvinceInstance const* new_selected_province) {
	if (selected_province != new_selected_province) {
		selected_province = new_selected_province;

		GameSingleton::get_singleton()->_update_colour_image();

		emit_signal(_signal_province_selected(), get_selected_province_number());
	}
}

void PlayerSingleton::set_selected_province_by_number(int32_t province_number) {
	if (province_number == ProvinceDefinition::NULL_PROVINCE_NUMBER) {
		unset_selected_province();
	} else {
		InstanceManager const* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
		ERR_FAIL_NULL(instance_manager);

		MapInstance const& map_instance = instance_manager->get_map_instance();

		set_selected_province(map_instance.get_province_instance_from_number(province_number));

		if (selected_province == nullptr) {
			spdlog::error_s(
				"Trying to set selected province to an invalid number {} (max number is {})",
				map_instance.get_province_instance_by_definition().get_count(), province_number
			);
		}
	}
}

void PlayerSingleton::unset_selected_province() {
	set_selected_province(nullptr);
}

int32_t PlayerSingleton::get_selected_province_number() const {
	return selected_province == nullptr
		? ProvinceDefinition::NULL_PROVINCE_NUMBER
		: selected_province->province_definition.get_province_number();
}

// Core
void PlayerSingleton::toggle_paused() const {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_pause_argument_t>(
		!instance_manager->get_simulation_clock().is_paused()
	);
}

void PlayerSingleton::increase_speed() const {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_speed_argument_t>(
		instance_manager->get_simulation_clock().get_simulation_speed() + 1
	);
}

void PlayerSingleton::decrease_speed() const {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_speed_argument_t>(
		instance_manager->get_simulation_clock().get_simulation_speed() - 1
	);
}

// Production
void PlayerSingleton::expand_selected_province_building(int32_t building_index) const {
	ERR_FAIL_NULL(selected_province);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<expand_province_building_argument_t>(
		selected_province->index, building_instance_index_t(building_index)
	);
}

// Budget
#define SET_SLIDER_GAME_ACTION_EXPLICIT(value_name, argument_name) \
void PlayerSingleton::set_##value_name##_slider_value(fixed_point_t const value) const { \
	if (player_country == nullptr) { \
		return; \
	} \
	GameSingleton::get_singleton()->get_instance_manager()->queue_game_action<set_##argument_name##_argument_t>( \
		player_country->index, value \
	); \
}
#define SET_SLIDER_GAME_ACTION(value_name) SET_SLIDER_GAME_ACTION_EXPLICIT(value_name, value_name)

SET_SLIDER_GAME_ACTION(administration_spending)
SET_SLIDER_GAME_ACTION(education_spending)
SET_SLIDER_GAME_ACTION(military_spending)
SET_SLIDER_GAME_ACTION(social_spending)
SET_SLIDER_GAME_ACTION_EXPLICIT(national_stockpile_army_spending, army_spending)
SET_SLIDER_GAME_ACTION_EXPLICIT(national_stockpile_navy_spending, navy_spending)
SET_SLIDER_GAME_ACTION_EXPLICIT(national_stockpile_construction_spending, construction_spending)
SET_SLIDER_GAME_ACTION(tariff_rate)

#undef SET_SLIDER_GAME_ACTION

void PlayerSingleton::set_strata_tax_rate_slider_value(Strata const& strata, fixed_point_t const value) const {
	if (player_country == nullptr) {
		return;
	}
	GameSingleton::get_singleton()->get_instance_manager()->queue_game_action<set_strata_tax_argument_t>(
		player_country->index, strata.index, value
	);
}

// Technology

// Politics

// Population

// Trade
void PlayerSingleton::set_good_automated(int32_t good_index, bool is_automated) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_good_automated_argument_t>(
		player_country->index, good_index_t(good_index), is_automated
	);
}

void PlayerSingleton::set_good_trade_order(int32_t good_index, bool is_selling, GUIScrollbar const* amount_slider) const {
	ERR_FAIL_NULL(amount_slider);
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_good_trade_order_argument_t>(
		player_country->index, good_index_t(good_index), is_selling,
		MenuSingleton::calculate_trade_menu_stockpile_cutoff_amount_fp(amount_slider->get_value_scaled_fp())
	);
}

// Diplomacy

// Military
void PlayerSingleton::create_leader(bool is_general) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<create_leader_argument_t>(
		player_country->index, is_general ? unit_branch_t::LAND : unit_branch_t::NAVAL
	);
}

void PlayerSingleton::set_can_use_leader(uint64_t leader_id, bool can_use) const {
	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_use_leader_argument_t>(
		unique_id_t(leader_id), can_use
	);
}

void PlayerSingleton::set_auto_create_leaders(bool value) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_auto_create_leaders_argument_t>(
		player_country->index, value
	);
}

void PlayerSingleton::set_auto_assign_leaders(bool value) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_auto_assign_leaders_argument_t>(
		player_country->index, value
	);
}

void PlayerSingleton::set_mobilise(bool value) const {
	ERR_FAIL_NULL(player_country);

	InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL(instance_manager);

	instance_manager->queue_game_action<set_mobilise_argument_t>(
		player_country->index, value
	);
}
