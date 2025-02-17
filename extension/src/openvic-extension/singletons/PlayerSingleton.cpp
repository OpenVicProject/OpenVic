#include "PlayerSingleton.hpp"

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/map/ProvinceInstance.hpp>

#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"

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
void PlayerSingleton::set_player_country(CountryInstance* new_player_country) {
	if (player_country != new_player_country) {
		GameSingleton& game_singleton = *GameSingleton::get_singleton();
		InstanceManager* instance_manager = game_singleton.get_instance_manager();
		ERR_FAIL_NULL(instance_manager);

		player_country = new_player_country;

		Logger::info("Set player country to: ", player_country != nullptr ? player_country->get_identifier() : "<NULL>");

		game_singleton._on_gamestate_updated();
	}
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
void PlayerSingleton::set_selected_province(ProvinceInstance* new_selected_province) {
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
		InstanceManager* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
		ERR_FAIL_NULL(instance_manager);

		MapInstance& map_instance = instance_manager->get_map_instance();

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
