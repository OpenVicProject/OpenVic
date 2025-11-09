#include "MapItemSingleton.hpp"

#include <openvic-simulation/utility/Containers.hpp>

#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/utility/Utilities.hpp"
#include "openvic-simulation/country/CountryDefinition.hpp"
#include "openvic-simulation/country/CountryInstance.hpp"
#include "openvic-simulation/DefinitionManager.hpp"
#include "openvic-simulation/economy/BuildingType.hpp"
#include "openvic-simulation/interface/GFXObject.hpp"
#include "openvic-simulation/map/ProvinceDefinition.hpp"
#include "openvic-simulation/map/ProvinceInstance.hpp"
#include "openvic-simulation/map/State.hpp"
#include "openvic-simulation/types/Vector.hpp"

using namespace godot;
using namespace OpenVic;
using namespace OpenVic::Utilities::literals;

void MapItemSingleton::_bind_methods() {
	OV_BIND_METHOD(MapItemSingleton::get_billboards);
	OV_BIND_METHOD(MapItemSingleton::get_province_positions);
	OV_BIND_METHOD(MapItemSingleton::get_max_capital_count);
	OV_BIND_METHOD(MapItemSingleton::get_capital_positions);
	OV_BIND_METHOD(MapItemSingleton::get_crime_icons);
	OV_BIND_METHOD(MapItemSingleton::get_rgo_icons);
	OV_BIND_METHOD(MapItemSingleton::get_national_focus_icons);
	OV_BIND_METHOD(MapItemSingleton::get_projections);
	OV_BIND_METHOD(MapItemSingleton::get_unit_position_by_province_number,{"province_number"});
	OV_BIND_METHOD(MapItemSingleton::get_port_position_by_province_number,{"province_number"});
	OV_BIND_METHOD(MapItemSingleton::get_clicked_port_province_number, {"position"});
	
}

MapItemSingleton* MapItemSingleton::get_singleton() {
	return singleton;
}

MapItemSingleton::MapItemSingleton() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

MapItemSingleton::~MapItemSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

// repackage the billboard object into a godot dictionary for the Billboard manager to work with
void MapItemSingleton::add_billboard_dict(GFX::Billboard const& billboard, TypedArray<Dictionary>& billboard_dict_array) const {
	Dictionary dict;

	dict[OV_SNAME(name)] = Utilities::std_to_godot_string(billboard.get_name());
	dict[OV_SNAME(texture)] = Utilities::std_to_godot_string(billboard.get_texture_file());
	dict[OV_SNAME(scale)] = static_cast<real_t>(billboard.get_scale());
	dict[OV_INAME("noFrames")] = billboard.get_no_of_frames();

	billboard_dict_array.push_back(dict);
}

//get an array of all the billboard dictionaries
TypedArray<Dictionary> MapItemSingleton::get_billboards() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();


	TypedArray<Dictionary> ret;

	for (memory::unique_base_ptr<GFX::Object> const& obj : game_singleton->get_definition_manager().get_ui_manager().get_objects()) {
		GFX::Billboard const* billboard = obj->cast_to<GFX::Billboard>();
		if (billboard != nullptr) {
			add_billboard_dict(*billboard, ret);
		}
	}

	return ret;
}

void MapItemSingleton::add_projection_dict(GFX::Projection const& projection, TypedArray<Dictionary>& projection_dict_array) const {
	Dictionary dict;

	dict[OV_SNAME(name)] = Utilities::std_to_godot_string(projection.get_name());
	dict[OV_SNAME(texture)] = Utilities::std_to_godot_string(projection.get_texture_file());
	dict[OV_SNAME(size)] = static_cast<real_t>(projection.get_size());
	dict[OV_INAME("spin")] = static_cast<real_t>(projection.get_spin());
	dict[OV_INAME("expanding")] = static_cast<real_t>(projection.get_expanding());
	dict[OV_INAME("duration")] = static_cast<real_t>(projection.get_duration());
	dict[OV_INAME("additative")] = projection.get_additative();

	projection_dict_array.push_back(dict);
}

TypedArray<Dictionary> MapItemSingleton::get_projections() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();

	TypedArray<Dictionary> ret;

	for (memory::unique_base_ptr<GFX::Object> const& obj : game_singleton->get_definition_manager().get_ui_manager().get_objects()) {
		GFX::Projection const* projection = obj->cast_to<GFX::Projection>();
		if (projection != nullptr) {
			add_projection_dict(*projection, ret);
		}
	}

	return ret;
}

PackedVector2Array MapItemSingleton::get_province_positions() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	MapDefinition const& map_definition = game_singleton->get_definition_manager().get_map_definition();

	PackedVector2Array billboard_pos {};

	billboard_pos.resize(map_definition.get_land_province_count());

	int64_t index = 0;

	for (ProvinceDefinition const& prov : map_definition.get_province_definitions()) {
		if (prov.is_water()) {
			// billboards dont appear over water, skip
			continue;
		}

		billboard_pos[index++] = game_singleton->get_billboard_pos(prov);
	}

	return billboard_pos;
}

//includes non-existent countries, used for setting the billboard buffer size
int32_t MapItemSingleton::get_max_capital_count() const {
	return GameSingleton::get_singleton()->get_definition_manager().get_country_definition_manager().get_country_definition_count();
}

PackedVector2Array MapItemSingleton::get_capital_positions() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	CountryInstanceManager const& country_instance_manager = instance_manager->get_country_instance_manager();

	PackedVector2Array billboard_pos {};

	billboard_pos.resize(country_instance_manager.get_country_instance_by_definition().get_count());

	int64_t index = 0;

	for (CountryInstance const& country : country_instance_manager.get_country_instances()) {
		if (!country.exists() || country.get_capital() == nullptr) {
			//skip non-existent or capital-less countries
			continue;
		}

		billboard_pos[index++] = game_singleton->get_billboard_pos(country.get_capital()->get_province_definition());
	}

	billboard_pos.resize(index);

	return billboard_pos;
}

PackedByteArray MapItemSingleton::get_crime_icons() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	MapInstance const& map_instance = instance_manager->get_map_instance();

	PackedByteArray icons {};

	icons.resize(map_instance.get_map_definition().get_land_province_count());

	int64_t index = 0;

	for (ProvinceInstance const& prov_inst : map_instance.get_province_instances()) {
		if (prov_inst.get_province_definition().is_water()) {
			// billboards dont appear over water, skip
			continue;
		}

		Crime const* crime = prov_inst.get_crime();
		icons[index++] = crime != nullptr ? crime->get_icon() : 0; // 0 if no crime in the province
	}

	return icons;
}

PackedByteArray MapItemSingleton::get_rgo_icons() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	MapInstance const& map_instance = instance_manager->get_map_instance();

	PackedByteArray icons {};

	icons.resize(map_instance.get_map_definition().get_land_province_count());

	int64_t index = 0;

	for (ProvinceInstance const& prov_inst : map_instance.get_province_instances()) {
		if (prov_inst.get_province_definition().is_water()) {
			// billboards dont appear over water, skip
			continue;
		}

		GoodDefinition const* rgo_good = prov_inst.get_rgo_good();
		icons[index++] = rgo_good != nullptr ? rgo_good->get_index() + 1 : 0; // 0 if no rgo good in the province
	}

	return icons;

}

/*
TODO: National focus isn't implemented yet. It could be done at the country instance, or the province instance
 So this function just returns dummy data.
 So in the future...
 - Return the icon of the current national focus of the state
 if there is a focus on that state, else return 0 to indicate no focus.
*/

PackedByteArray MapItemSingleton::get_national_focus_icons() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	MapInstance const& map_instance = instance_manager->get_map_instance();

	PackedByteArray icons {};

	icons.resize(map_instance.get_map_definition().get_land_province_count());

	int64_t index = 0;

	for (ProvinceInstance const& prov_inst : map_instance.get_province_instances()) {
		if (prov_inst.get_province_definition().is_water()) {
			// billboards dont appear over water, skip
			continue;
		}

		State const* state = prov_inst.get_state();
		icons[index++] = state != nullptr && &prov_inst == state->get_capital() ? 1 : 0;
	}

	return icons;
}


Vector2 MapItemSingleton::get_unit_position_by_province_number(int32_t province_number) const { 
	GameSingleton const* game_singleton = GameSingleton::get_singleton();

	ProvinceDefinition const* province = game_singleton->get_definition_manager().get_map_definition()
		.get_province_definition_from_number(province_number);
	ERR_FAIL_NULL_V_MSG(province, {}, Utilities::format("Cannot get unit position - invalid province number: %d", province_number));

	return game_singleton->normalise_map_position(province->get_unit_position());
}

Vector2 MapItemSingleton::get_port_position_by_province_number(int32_t province_number) const { 
	GameSingleton const* game_singleton = GameSingleton::get_singleton();

	ProvinceDefinition const* province = game_singleton->get_definition_manager().get_map_definition()
		.get_province_definition_from_number(province_number);
	ERR_FAIL_NULL_V_MSG(province, {}, Utilities::format("Cannot get port position - invalid province number: %d", province_number));
	ERR_FAIL_COND_V_MSG(!province->has_port(), {},Utilities::format("Cannot get port position, province has no port, number: %d", province_number) ); 

	BuildingType const* port_building_type = game_singleton->get_definition_manager().get_economy_manager().get_building_type_manager().get_port_building_type();
	fvec2_t const* port_position = province->get_building_position(port_building_type);

	//no null check because province->has_port() condition passed, already verifying that this isn't null
	return game_singleton->normalise_map_position(*port_position);
}

static constexpr real_t port_radius = 0.0006_real; //how close we have to click for a detection

//Searches provinces near the one clicked and attempts to find a port within the port_radius of the click position
int32_t MapItemSingleton::get_clicked_port_province_number(Vector2 click_position) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();

	int32_t initial_province_number = game_singleton->get_province_number_from_uv_coords(click_position);

	ProvinceDefinition const* province = game_singleton->get_definition_manager().get_map_definition()
		.get_province_definition_from_number(initial_province_number);
	ERR_FAIL_NULL_V_MSG(province, {}, Utilities::format("Cannot get port position - invalid province number: %d", initial_province_number));

	BuildingType const* port_building_type = game_singleton->get_definition_manager().get_economy_manager().get_building_type_manager().get_port_building_type();
	
	if(province->has_port()){
		Vector2 port_position = game_singleton->normalise_map_position(*province->get_building_position(port_building_type));
		if(click_position.distance_to(port_position) <= port_radius){
			return province->get_province_number();
		}
	}
	else if(province->is_water()){
		// search the adjacent provinces for ones with ports
		for(ProvinceDefinition::adjacency_t const& adjacency : province->get_adjacencies()) {
			ProvinceDefinition const* adjacent_province = adjacency.get_to();
			if(!adjacent_province->has_port()) { 
				continue; // skip provinces without ports (ie. other water provinces)
			}
			Vector2 port_position = game_singleton->normalise_map_position(*adjacent_province->get_building_position(port_building_type));
			if(click_position.distance_to(port_position) <= port_radius){
				return adjacent_province->get_province_number();
			}
		}
	}

	return 0;
}