#include "ModelSingleton.hpp"

#include <numbers>
#include <span>

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-simulation/map/ProvinceInstance.hpp>
#include <openvic-simulation/utility/Containers.hpp>

#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

void ModelSingleton::_bind_methods() {
	OV_BIND_METHOD(ModelSingleton::get_units);
	OV_BIND_METHOD(ModelSingleton::get_cultural_gun_model, { "culture" });
	OV_BIND_METHOD(ModelSingleton::get_cultural_helmet_model, { "culture" });
	OV_BIND_METHOD(ModelSingleton::get_flag_model, { "floating" });
	OV_BIND_METHOD(ModelSingleton::get_buildings);
}

ModelSingleton* ModelSingleton::get_singleton() {
	return singleton;
}

ModelSingleton::ModelSingleton() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}

ModelSingleton::~ModelSingleton() {
	ERR_FAIL_COND(singleton != this);
	singleton = nullptr;
}

GFX::Actor const* ModelSingleton::get_actor(std::string_view name, bool error_on_fail) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);

	GFX::Actor const* actor =
		game_singleton->get_definition_manager().get_ui_manager().get_cast_object_by_identifier<GFX::Actor>(name);

	if (error_on_fail) {
		ERR_FAIL_NULL_V_MSG(actor, nullptr, Utilities::format("Failed to find actor \"%s\"", Utilities::std_to_godot_string(name)));
	}

	return actor;
}

GFX::Actor const* ModelSingleton::get_cultural_actor(
	std::string_view culture, std::string_view name, std::string_view fallback_name
) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);

	ERR_FAIL_COND_V_MSG(
		culture.empty() || name.empty(), nullptr, Utilities::format(
			"Failed to find actor \"%s\" for culture \"%s\" - neither can be empty",
			Utilities::std_to_godot_string(name), Utilities::std_to_godot_string(culture)
		)
	);

	memory::string actor_name = StringUtils::append_string_views(culture, name);

	GFX::Actor const* actor = get_actor(actor_name, false);

	// Which should be tried first: "Generic***" or "***Infantry"?

	if (actor == nullptr) {
		/* If no Actor exists for the specified GraphicalCultureType then try the default instead. */
		GraphicalCultureType const* default_graphical_culture_type = game_singleton->get_definition_manager().get_pop_manager()
			.get_culture_manager().get_default_graphical_culture_type();

		if (default_graphical_culture_type != nullptr && default_graphical_culture_type->get_identifier() != culture) {
			actor_name = StringUtils::append_string_views(default_graphical_culture_type->get_identifier(), name);

			actor = get_actor(actor_name, false);
		}

		if (actor == nullptr && !fallback_name.empty() && fallback_name != name) {
			return get_cultural_actor(culture, fallback_name, {});
		}
	}

	ERR_FAIL_NULL_V_MSG(
		actor, nullptr, Utilities::format(
			"Failed to find actor \"%s\" for culture \"%s\"", Utilities::std_to_godot_string(name),
			Utilities::std_to_godot_string(culture)
		)
	);

	return actor;
}

Dictionary ModelSingleton::get_animation_dict(GFX::Actor::Animation const& animation) {
	const animation_map_t::const_iterator it = animation_cache.find(&animation);
	if (it != animation_cache.end()) {
		return it->second;
	}

	Dictionary dict;

	dict[OV_SNAME(file)] = Utilities::std_to_godot_string(animation.get_file());
	dict[OV_SNAME(time)] = static_cast<real_t>(animation.get_scroll_time());

	animation_cache.emplace(&animation, dict);

	return dict;
}

Dictionary ModelSingleton::get_model_dict(GFX::Actor const& actor) {
	const model_map_t::const_iterator it = model_cache.find(&actor);
	if (it != model_cache.end()) {
		return it->second;
	}

	Dictionary dict;

	dict[OV_SNAME(file)] = Utilities::std_to_godot_string(actor.get_model_file());
	dict[OV_SNAME(scale)] = static_cast<real_t>(actor.get_scale());

	const auto set_animation = [this, &dict](StringName const& key, std::optional<GFX::Actor::Animation> const& animation) {
		if (animation.has_value()) {
			dict[key] = get_animation_dict(*animation);
		}
	};

	set_animation(OV_SNAME(idle), actor.get_idle_animation());
	set_animation(OV_SNAME(move), actor.get_move_animation());
	set_animation(OV_SNAME(attack), actor.get_attack_animation());

	std::span<const GFX::Actor::Attachment> attachments = actor.get_attachments();

	if (!attachments.empty()) {
		TypedArray<Dictionary> attachments_array;

		if (attachments_array.resize(attachments.size()) == OK) {

			for (size_t idx = 0; idx < attachments_array.size(); ++idx) {

				GFX::Actor::Attachment const& attachment = attachments[idx];

				GFX::Actor const* attachment_actor = get_actor(attachment.get_actor_name());

				ERR_CONTINUE_MSG(
					attachment_actor == nullptr, Utilities::format(
						"Failed to find \"%s\" attachment actor for actor \"%s\"",
						Utilities::std_to_godot_string(attachment.get_actor_name()),
						Utilities::std_to_godot_string(actor.get_name())
					)
				);

				Dictionary attachment_dict;

				attachment_dict[OV_SNAME(node)] = Utilities::std_to_godot_string(attachment.get_attach_node());
				attachment_dict[OV_SNAME(model)] = get_model_dict(*attachment_actor);

				attachments_array[idx] = std::move(attachment_dict);

			}

			if (!attachments_array.is_empty()) {
				dict[OV_SNAME(attachments)] = std::move(attachments_array);
			}

		} else {
			UtilityFunctions::push_error(
				"Failed to resize attachments array to the correct size (", static_cast<int64_t>(attachments.size()),
				") for model for actor \"", Utilities::std_to_godot_string(actor.get_name()), "\""
			);
		}
	}

	model_cache.emplace(&actor, dict);

	return dict;
}

/* Returns false if an error occurs while trying to add a unit model for the province, true otherwise.
 * Returning true doesn't necessarily mean a unit was added, e.g. when units is empty. */
template<unit_branch_t Branch>
bool ModelSingleton::add_unit_dict(
	std::span<UnitInstanceGroupBranched<Branch>* const> units, TypedArray<Dictionary>& unit_array
) {
	using _UnitInstanceGroup = UnitInstanceGroupBranched<Branch>;

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, false);

	if (units.empty()) {
		return true;
	}

	bool ret = true;

	/* Last unit to enter the province is shown on top. */
	_UnitInstanceGroup const& unit = *units.back();
	ERR_FAIL_COND_V_MSG(unit.empty(), false, Utilities::format("Empty unit \"%s\"", Utilities::std_to_godot_string(unit.get_name())));

	CountryDefinition const& country_definition = unit.get_country()->get_country_definition();

	GraphicalCultureType const& graphical_culture_type = country_definition.get_graphical_culture();
	UnitType const* display_unit_type = unit.get_display_unit_type();
	ERR_FAIL_NULL_V_MSG(
		display_unit_type, false, Utilities::format(
			"Failed to get display unit type for unit \"%s\"", Utilities::std_to_godot_string(unit.get_name())
		)
	);

	std::string_view actor_name = display_unit_type->get_sprite();
	std::string_view mount_actor_name, mount_attach_node_name;

	if constexpr (Branch == unit_branch_t::LAND) {
		RegimentType const* regiment_type = reinterpret_cast<RegimentType const*>(display_unit_type);

		if (!regiment_type->get_sprite_override().empty()) {
			actor_name = regiment_type->get_sprite_override();
		}

		if (regiment_type->get_sprite_mount().empty() == regiment_type->get_sprite_mount_attach_node().empty()) {
			if (!regiment_type->get_sprite_mount().empty()) {
				mount_actor_name = regiment_type->get_sprite_mount();
				mount_attach_node_name = regiment_type->get_sprite_mount_attach_node();
			}
		} else {
			UtilityFunctions::push_error(
				"Mount sprite and attach node must both be set or both be empty - regiment type \"",
				Utilities::std_to_godot_string(regiment_type->get_identifier()), "\" has mount \"",
				Utilities::std_to_godot_string(regiment_type->get_sprite_mount()), "\" and attach node \"",
				Utilities::std_to_godot_string(regiment_type->get_sprite_mount_attach_node()), "\""
			);
			ret = false;
		}
	}

	// TODO - default without requiring hardcoded name
	static constexpr std::string_view default_fallback_actor_name = "Infantry";
	GFX::Actor const* actor = get_cultural_actor(
		graphical_culture_type.get_identifier(), actor_name, default_fallback_actor_name
	);

	ERR_FAIL_NULL_V_MSG(
		actor, false, Utilities::format(
			"Failed to find \"%s\" actor of graphical culture type \"%s\" for unit \"%s\"",
			Utilities::std_to_godot_string(display_unit_type->get_sprite()),
			Utilities::std_to_godot_string(graphical_culture_type.get_identifier()),
			Utilities::std_to_godot_string(unit.get_name())
		)
	);

	Dictionary dict;

	dict[OV_SNAME(culture)] = Utilities::std_to_godot_string(graphical_culture_type.get_identifier());

	dict[OV_SNAME(model)] = get_model_dict(*actor);

	if (!mount_actor_name.empty() && !mount_attach_node_name.empty()) {
		GFX::Actor const* mount_actor = get_actor(mount_actor_name);

		if (mount_actor != nullptr) {
			dict[OV_INAME("mount_model")] = get_model_dict(*mount_actor);
			dict[OV_INAME("mount_attach_node")] = Utilities::std_to_godot_string(mount_attach_node_name);
		} else {
			UtilityFunctions::push_error(Utilities::format(
				"Failed to find \"%s\" mount actor of graphical culture type \"%s\" for unit \"%s\"",
				Utilities::std_to_godot_string(mount_actor_name),
				Utilities::std_to_godot_string(graphical_culture_type.get_identifier()),
				Utilities::std_to_godot_string(unit.get_name())
			));
			ret = false;
		}
	}

	// TODO - government type based flag type
	dict[OV_INAME("flag_index")] = game_singleton->get_flag_sheet_index(country_definition.get_index(), {});

	if (display_unit_type->has_floating_flag()) {
		dict[OV_INAME("flag_floating")] = true;
	}

	dict[OV_SNAME(position)] =
		game_singleton->normalise_map_position(unit.get_position()->get_province_definition().get_unit_position());

	if (display_unit_type->get_unit_category() != UnitType::unit_category_t::INFANTRY) {
		dict[OV_SNAME(rotation)] = -0.25f * std::numbers::pi_v<float>;
	}

	dict[OV_SNAME(primary_colour)] = Utilities::to_godot_color(country_definition.get_primary_unit_colour());
	dict[OV_SNAME(secondary_colour)] = Utilities::to_godot_color(country_definition.get_secondary_unit_colour());
	dict[OV_SNAME(tertiary_colour)] = Utilities::to_godot_color(country_definition.get_tertiary_unit_colour());

	// TODO - move dict into unit_array ?
	unit_array.push_back(dict);

	return ret;
}

TypedArray<Dictionary> ModelSingleton::get_units() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	TypedArray<Dictionary> ret;

	for (ProvinceInstance const& province : instance_manager->get_map_instance().get_province_instances()) {
		if (province.get_province_definition().is_water()) {
			if (!add_unit_dict(std::span { province.get_navies() }, ret)) {
				UtilityFunctions::push_error(
					"Error adding navy to province \"", Utilities::std_to_godot_string(province.get_identifier()), "\""
				);
			}
		} else {
			if (!add_unit_dict(std::span { province.get_armies() }, ret)) {
				UtilityFunctions::push_error(
					"Error adding army to province \"", Utilities::std_to_godot_string(province.get_identifier()), "\""
				);
			}
		}

		// TODO - land units in ships
	}

	return ret;
}

Dictionary ModelSingleton::get_cultural_gun_model(String const& culture) {
	static constexpr std::string_view gun_actor_name = "Gun1";

	GFX::Actor const* actor = get_cultural_actor(Utilities::godot_to_std_string(culture), gun_actor_name, {});

	ERR_FAIL_NULL_V(actor, {});

	return get_model_dict(*actor);
}

Dictionary ModelSingleton::get_cultural_helmet_model(String const& culture) {
	static constexpr std::string_view helmet_actor_name = "Helmet1";

	GFX::Actor const* actor = get_cultural_actor(Utilities::godot_to_std_string(culture), helmet_actor_name, {});

	ERR_FAIL_NULL_V(actor, {});

	return get_model_dict(*actor);
}

Dictionary ModelSingleton::get_flag_model(bool floating) {
	static constexpr std::string_view flag_name = "Flag";
	static constexpr std::string_view flag_floating_name = "FlagFloating";

	GFX::Actor const* actor = get_actor(floating ? flag_floating_name : flag_name);

	ERR_FAIL_NULL_V(actor, {});

	return get_model_dict(*actor);
}

bool ModelSingleton::add_building_dict(
	BuildingInstance const& building, ProvinceInstance const& province, TypedArray<Dictionary>& building_array
) {
	ProvinceDefinition const& province_definition = province.get_province_definition();

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, false);

	std::string suffix;

	if (
		&building.get_building_type() ==
			game_singleton->get_definition_manager().get_economy_manager().get_building_type_manager().get_port_building_type()
	) {
		/* Port */
		if (!province_definition.has_port()) {
			return true;
		}

		if (building.get_level() > 0) {
			suffix = std::to_string(building.get_level());
		}

		if (!province.get_navies().empty()) {
			suffix += "_ships";
		}
	} else if (building.get_identifier() == "fort") {
		/* Fort */
		if (building.get_level() < 1) {
			return true;
		}

		if (building.get_level() > 1) {
			suffix = std::to_string(building.get_level());
		}
	} else {
		// TODO - railroad (trainstations)
		return true;
	}

	fvec2_t const* position_ptr = province_definition.get_building_position(&building.get_building_type());
	const float rotation = static_cast<float>(province_definition.get_building_rotation(&building.get_building_type()));

	const memory::string actor_name = StringUtils::append_string_views("building_", building.get_identifier(), suffix);

	GFX::Actor const* actor = get_actor(actor_name);
	ERR_FAIL_NULL_V_MSG(
		actor, false, Utilities::format(
			"Failed to find \"%s\" actor for building \"%s\" in province \"%s\"",
			Utilities::std_to_godot_string(actor_name), Utilities::std_to_godot_string(building.get_identifier()),
			Utilities::std_to_godot_string(province.get_identifier())
		)
	);

	Dictionary dict;

	dict[OV_SNAME(model)] = get_model_dict(*actor);

	dict[OV_SNAME(position)] = game_singleton->normalise_map_position(
		position_ptr != nullptr ? *position_ptr : province_definition.get_centre()
	);

	if (rotation != 0.0f) {
		dict[OV_SNAME(rotation)] = rotation;
	}

	// TODO - move dict into unit_array ?
	building_array.push_back(dict);

	return true;
}

TypedArray<Dictionary> ModelSingleton::get_buildings() {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	TypedArray<Dictionary> ret;

	for (ProvinceInstance const& province : instance_manager->get_map_instance().get_province_instances()) {
		if (!province.get_province_definition().is_water()) {
			for (BuildingInstance const& building : province.get_buildings()) {
				if (!add_building_dict(building, province, ret)) {
					UtilityFunctions::push_error(
						"Error adding building \"", Utilities::std_to_godot_string(building.get_identifier()),
						"\" to province \"", Utilities::std_to_godot_string(province.get_identifier()), "\""
					);
				}
			}
		}
	}

	return ret;
}
