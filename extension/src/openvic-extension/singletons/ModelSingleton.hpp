#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/interface/GFXObject.hpp>
#include <openvic-simulation/military/UnitInstance.hpp>

namespace OpenVic {
	struct BuildingInstance;

	class ModelSingleton : public godot::Object {
		GDCLASS(ModelSingleton, godot::Object)

		static inline ModelSingleton* singleton = nullptr;

	protected:
		static void _bind_methods();

	public:
		static ModelSingleton* get_singleton();

		ModelSingleton();
		~ModelSingleton();

	private:
		GFX::Actor const* get_actor(std::string_view name, bool error_on_fail = true) const;
		GFX::Actor const* get_cultural_actor(
			std::string_view culture, std::string_view name, std::string_view fallback_name
		) const;

		godot::Dictionary make_animation_dict(GFX::Actor::Animation const& animation) const;
		godot::Dictionary make_model_dict(GFX::Actor const& actor) const;

		template<utility::is_derived_from_specialization_of<UnitInstanceGroup> T>
		bool add_unit_dict(ordered_set<T*> const& units, godot::TypedArray<godot::Dictionary>& unit_array) const;

		bool add_building_dict(
			BuildingInstance const& building, ProvinceInstance const& province,
			godot::TypedArray<godot::Dictionary>& building_array
		) const;

	public:
		godot::TypedArray<godot::Dictionary> get_units() const;
		godot::Dictionary get_cultural_gun_model(godot::String const& culture) const;
		godot::Dictionary get_cultural_helmet_model(godot::String const& culture) const;

		godot::Dictionary get_flag_model(bool floating) const;

		godot::TypedArray<godot::Dictionary> get_buildings() const;
	};
}
