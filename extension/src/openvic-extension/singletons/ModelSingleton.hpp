#pragma once

#include <string_view>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/object.hpp>

#include <openvic-simulation/interface/GFXObject.hpp>
#include <openvic-simulation/military/UnitInstanceGroup.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>
#include "../utility/XSMLoader.hpp"
#include "../utility/XACLoader.hpp"
#include "godot_cpp/classes/node3d.hpp"

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
		using xsm_map_t = deque_ordered_map<godot::StringName, godot::Ref<godot::Animation>>;
		using xac_map_t = deque_ordered_map<godot::StringName, godot::Node3D*>;

		animation_map_t animation_cache;
		model_map_t model_cache;
		xsm_map_t xsm_cache;
		xac_map_t xac_cache;

		godot::Dictionary get_animation_dict(GFX::Actor::Animation const& animation);
		godot::Dictionary get_model_dict(GFX::Actor const& actor);

		template<UnitType::branch_t Branch>
		bool add_unit_dict(
			std::vector<UnitInstanceGroupBranched<Branch>*> const& units, godot::TypedArray<godot::Dictionary>& unit_array
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

		godot::Ref<godot::Animation> get_xsm_animation(godot::String source_file);
		godot::Node3D* get_xac_model(godot::String source_file);
		godot::Error setup_flag_shader();
	};
}
