#include "MapItemSingleton.hpp"
#include <string_view>

#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/variant/vector2.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"
#include "openvic-simulation/DefinitionManager.hpp"
#include "openvic-simulation/country/CountryDefinition.hpp"
#include "openvic-simulation/country/CountryInstance.hpp"
#include "openvic-simulation/interface/GFXObject.hpp"
#include "openvic-simulation/map/ProvinceDefinition.hpp"
#include "openvic-simulation/map/ProvinceInstance.hpp"
#include "openvic-simulation/map/State.hpp"
#include "openvic-simulation/types/Vector.hpp"

using namespace godot;
using namespace OpenVic;

void MapItemSingleton::_bind_methods() {
	OV_BIND_METHOD(MapItemSingleton::get_billboards);
	OV_BIND_METHOD(MapItemSingleton::get_province_positions);
	OV_BIND_METHOD(MapItemSingleton::get_capital_count);
	OV_BIND_METHOD(MapItemSingleton::get_capital_positions);
	OV_BIND_METHOD(MapItemSingleton::get_crime_icons);
	OV_BIND_METHOD(MapItemSingleton::get_rgo_icons);
	OV_BIND_METHOD(MapItemSingleton::get_national_focus_icons);
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

// Get the billboard object from the loaded objects

GFX::Billboard const* MapItemSingleton::get_billboard(std::string_view name, bool error_on_fail) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);

	GFX::Billboard const* billboard =
		game_singleton->get_definition_manager().get_ui_manager().get_cast_object_by_identifier<GFX::Billboard>(name);

	if (error_on_fail) {
		ERR_FAIL_NULL_V_MSG(billboard, nullptr, vformat("Failed to find billboard \"%s\"", Utilities::std_to_godot_string(name)));
	}

	return billboard;
}

// repackage the billboard object into a godot dictionnary for the Billboard manager to work with
bool MapItemSingleton::add_billboard_dict(std::string_view name, TypedArray<Dictionary>& billboard_dict_array) const {

	static const StringName name_key = "name";
	static const StringName texture_key = "texture";
	static const StringName scale_key = "scale";
	static const StringName noOfFrames_key = "noFrames";

	GFX::Billboard const* billboard = get_billboard(name,false);

	ERR_FAIL_NULL_V_MSG(
		billboard, false, "Failed to find billboard"
	);

	Dictionary dict;

	dict[name_key] = Utilities::std_to_godot_string(billboard->get_name());
	dict[texture_key] = Utilities::std_to_godot_string(billboard->get_texture_file());
	dict[scale_key] = billboard->get_scale().to_float();
	dict[noOfFrames_key] = billboard->get_no_of_frames();

	billboard_dict_array.push_back(dict);

	return true;
}


//get an array of all the billboard dictionnaries
TypedArray<Dictionary> MapItemSingleton::get_billboards() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});

	TypedArray<Dictionary> ret;

	for (std::unique_ptr<GFX::Object> const& obj : game_singleton->get_definition_manager().get_ui_manager().get_objects()) {
		if (obj->is_type<GFX::Billboard>()) {
			add_billboard_dict(obj->get_name(), ret);
		}
	}
	
	return ret;
}

PackedVector2Array MapItemSingleton::get_province_positions() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, PackedVector2Array());

	PackedVector2Array billboard_pos {};
	
	for(ProvinceDefinition const& prov : game_singleton->get_definition_manager().get_map_definition().get_province_definitions()){
		if(prov.is_water()) continue; //billboards dont appear over water, skip

		fvec2_t city_pos = prov.get_city_position();
		Vector2 pos = Utilities::to_godot_fvec2(city_pos) / game_singleton->get_map_dims();
		billboard_pos.push_back(pos);

	}
	
	return billboard_pos;
}

//includes non-existent countries, used for setting the billboard buffer size
int32_t MapItemSingleton::get_capital_count() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, 0);

	return game_singleton->get_definition_manager().get_country_definition_manager().get_country_definition_count();
}

PackedVector2Array MapItemSingleton::get_capital_positions() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, PackedVector2Array());

	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, PackedVector2Array());

	PackedVector2Array billboard_pos {};

	for(CountryInstance const& country : instance_manager->get_country_instance_manager().get_country_instances()){
		if(!country.exists()) continue; //skip non-existant countries

		fvec2_t city_pos = country.get_capital()->get_province_definition().get_city_position();
		Vector2 pos = Utilities::to_godot_fvec2(city_pos) / game_singleton->get_map_dims();
		billboard_pos.push_back(pos);

	}

	return billboard_pos;
}

PackedByteArray MapItemSingleton::get_crime_icons() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, PackedByteArray());

	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, PackedByteArray());

	PackedByteArray icons {};

	for(ProvinceInstance const& prov_inst : instance_manager->get_map_instance().get_province_instances()){
		if (prov_inst.get_province_definition().is_water()) continue; //billboards dont appear over water, skip

		if(prov_inst.get_crime() == nullptr){
			icons.push_back(0); //no crime on the province
		}
		else {
			icons.push_back(prov_inst.get_crime()->get_icon());
		}
		
	}

	return icons;

}

PackedByteArray MapItemSingleton::get_rgo_icons() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, PackedByteArray());

	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, PackedByteArray());

	PackedByteArray icons {};

	for(ProvinceInstance const& prov_inst : instance_manager->get_map_instance().get_province_instances()){
		if (prov_inst.get_province_definition().is_water()) continue; //billboards dont appear over water, skip

		if(prov_inst.get_rgo_good() == nullptr){
			icons.push_back(0); //no good on the province
		}
		else{
			icons.push_back(prov_inst.get_rgo_good()->get_index()+1);
		}
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
	ERR_FAIL_NULL_V(game_singleton, PackedByteArray());

	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, PackedByteArray());

	PackedByteArray icons {};

	for(ProvinceInstance const& prov_inst : instance_manager->get_map_instance().get_province_instances()){
		if (prov_inst.get_province_definition().is_water()) continue; //billboards dont appear over water, skip

		State const* state = prov_inst.get_state();
		if (state == nullptr) {
			icons.push_back(0);
			UtilityFunctions::push_warning(
				"State for province ", Utilities::std_to_godot_string(prov_inst.get_identifier()), " was null"
			);
		} else if (&prov_inst == state->get_capital()) {
			icons.push_back(1);
		} else {
			icons.push_back(0);
		}
	}

	return icons;
}