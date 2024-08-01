#pragma once

#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/interface/GFXObject.hpp>
#include <openvic-simulation/military/UnitInstanceGroup.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>

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

		using animation_map_t = deque_ordered_map<GFX::Actor::Animation const*, godot::Dictionary>;
		using model_map_t = deque_ordered_map<GFX::Actor const*, godot::Dictionary>;

		animation_map_t animation_cache;
		model_map_t model_cache;

		godot::Dictionary get_animation_dict(GFX::Actor::Animation const& animation);
		godot::Dictionary get_model_dict(GFX::Actor const& actor);

		template<UnitType::branch_t Branch>
		bool add_unit_dict(
			ordered_set<UnitInstanceGroupBranched<Branch>*> const& units, godot::TypedArray<godot::Dictionary>& unit_array
		);

		bool add_building_dict(
			BuildingInstance const& building, ProvinceInstance const& province,
			godot::TypedArray<godot::Dictionary>& building_array
		);

	public:
		godot::TypedArray<godot::Dictionary> get_units();
		godot::Dictionary get_cultural_gun_model(godot::String const& culture);
		godot::Dictionary get_cultural_helmet_model(godot::String const& culture);

		godot::Dictionary get_flag_model(bool floating);

		godot::TypedArray<godot::Dictionary> get_buildings();
	};
}
