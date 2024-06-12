#include "ModelSingleton.hpp"

#include <numbers>

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_to_godot_string;
using OpenVic::Utilities::std_view_to_godot_string;

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
		ERR_FAIL_NULL_V_MSG(actor, nullptr, vformat("Failed to find actor \"%s\"", std_view_to_godot_string(name)));
	}

	return actor;
}

GFX::Actor const* ModelSingleton::get_cultural_actor(
	std::string_view culture, std::string_view name, std::string_view fallback_name
) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);

	ERR_FAIL_COND_V_MSG(
		culture.empty() || name.empty(), nullptr, vformat(
			"Failed to find actor \"%s\" for culture \"%s\" - neither can be empty",
			std_view_to_godot_string(name), std_view_to_godot_string(culture)
		)
	);

	std::string actor_name = StringUtils::append_string_views(culture, name);

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
		actor, nullptr, vformat(
			"Failed to find actor \"%s\" for culture \"%s\"", std_view_to_godot_string(name),
			std_view_to_godot_string(culture)
		)
	);

	return actor;
}

Dictionary ModelSingleton::make_animation_dict(GFX::Actor::Animation const& animation) const {
	static const StringName file_key = "file";
	static const StringName time_key = "time";

	Dictionary dict;

	dict[file_key] = std_view_to_godot_string(animation.get_file());
	dict[time_key] = animation.get_scroll_time().to_float();

	return dict;
}

Dictionary ModelSingleton::make_model_dict(GFX::Actor const& actor) const {
	static const StringName file_key = "file";
	static const StringName scale_key = "scale";
	static const StringName idle_key = "idle";
	static const StringName move_key = "move";
	static const StringName attack_key = "attack";
	static const StringName attachments_key = "attachments";

	Dictionary dict;

	dict[file_key] = std_view_to_godot_string(actor.get_model_file());
	dict[scale_key] = actor.get_scale().to_float();

	const auto set_animation = [this, &dict](StringName const& key, std::optional<GFX::Actor::Animation> const& animation) {
		if (animation.has_value()) {
			dict[key] = make_animation_dict(*animation);
		}
	};

	set_animation(idle_key, actor.get_idle_animation());
	set_animation(move_key, actor.get_move_animation());
	set_animation(attack_key, actor.get_attack_animation());

	std::vector<GFX::Actor::Attachment> const& attachments = actor.get_attachments();

	if (!attachments.empty()) {
		static const StringName attachment_node_key = "node";
		static const StringName attachment_model_key = "model";

		TypedArray<Dictionary> attachments_array;

		if (attachments_array.resize(attachments.size()) == OK) {

			for (size_t idx = 0; idx < attachments_array.size(); ++idx) {

				GFX::Actor::Attachment const& attachment = attachments[idx];

				GFX::Actor const* attachment_actor = get_actor(attachment.get_actor_name());

				ERR_CONTINUE_MSG(
					attachment_actor == nullptr, vformat(
						"Failed to find \"%s\" attachment actor for actor \"%s\"",
						std_view_to_godot_string(attachment.get_actor_name()), std_view_to_godot_string(actor.get_name())
					)
				);

				Dictionary attachment_dict;

				attachment_dict[attachment_node_key] = std_view_to_godot_string(attachment.get_attach_node());
				attachment_dict[attachment_model_key] = make_model_dict(*attachment_actor);

				attachments_array[idx] = std::move(attachment_dict);

			}

			if (!attachments_array.is_empty()) {
				dict[attachments_key] = std::move(attachments_array);
			}

		} else {
			UtilityFunctions::push_error(
				"Failed to resize attachments array to the correct size (", static_cast<int64_t>(attachments.size()),
				") for model for actor \"", std_view_to_godot_string(actor.get_name()), "\""
			);
		}
	}

	return dict;
}

/* Returns false if an error occurs while trying to add a unit model for the province, true otherwise.
 * Returning true doesn't necessarily mean a unit was added, e.g. when units is empty. */
template<utility::is_derived_from_specialization_of<UnitInstanceGroup> T>
bool ModelSingleton::add_unit_dict(ordered_set<T*> const& units, TypedArray<Dictionary>& unit_array) const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, false);

	static const StringName culture_key = "culture";
	static const StringName model_key = "model";
	static const StringName mount_model_key = "mount_model";
	static const StringName mount_attach_node_key = "mount_attach_node";
	static const StringName flag_index_key = "flag_index";
	static const StringName flag_floating_key = "flag_floating";
	static const StringName position_key = "position";
	static const StringName rotation_key = "rotation";
	static const StringName primary_colour_key = "primary_colour";
	static const StringName secondary_colour_key = "secondary_colour";
	static const StringName tertiary_colour_key = "tertiary_colour";

	if (units.empty()) {
		return true;
	}

	bool ret = true;

	/* Last unit to enter the province is shown on top. */
	T const& unit = *units.back();
	ERR_FAIL_COND_V_MSG(unit.empty(), false, vformat("Empty unit \"%s\"", std_view_to_godot_string(unit.get_name())));

	Country const* country = unit.get_country()->get_base_country();

	GraphicalCultureType const& graphical_culture_type = country->get_graphical_culture();
	UnitType const* display_unit_type = unit.get_display_unit_type();
	ERR_FAIL_NULL_V_MSG(
		display_unit_type, false, vformat(
			"Failed to get display unit type for unit \"%s\"", std_view_to_godot_string(unit.get_name())
		)
	);

	std::string_view actor_name = display_unit_type->get_sprite();
	std::string_view mount_actor_name, mount_attach_node_name;

	if constexpr (std::same_as<T, ArmyInstance>) {
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
				std_view_to_godot_string(regiment_type->get_identifier()), "\" has mount \"",
				std_view_to_godot_string(regiment_type->get_sprite_mount()), "\" and attach node \"",
				std_view_to_godot_string(regiment_type->get_sprite_mount_attach_node()), "\""
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
		actor, false, vformat(
			"Failed to find \"%s\" actor of graphical culture type \"%s\" for unit \"%s\"",
			std_view_to_godot_string(display_unit_type->get_sprite()),
			std_view_to_godot_string(graphical_culture_type.get_identifier()),
			std_view_to_godot_string(unit.get_name())
		)
	);

	Dictionary dict;

	dict[culture_key] = std_view_to_godot_string(graphical_culture_type.get_identifier());

	dict[model_key] = make_model_dict(*actor);

	if (!mount_actor_name.empty() && !mount_attach_node_name.empty()) {
		GFX::Actor const* mount_actor = get_actor(mount_actor_name);

		if (mount_actor != nullptr) {
			dict[mount_model_key] = make_model_dict(*mount_actor);
			dict[mount_attach_node_key] = std_view_to_godot_string(mount_attach_node_name);
		} else {
			UtilityFunctions::push_error(vformat(
				"Failed to find \"%s\" mount actor of graphical culture type \"%s\" for unit \"%s\"",
				std_view_to_godot_string(mount_actor_name),
				std_view_to_godot_string(graphical_culture_type.get_identifier()),
				std_view_to_godot_string(unit.get_name())
			));
			ret = false;
		}
	}

	// TODO - government type based flag type
	dict[flag_index_key] = game_singleton->get_flag_sheet_index(country->get_index(), {});

	if (display_unit_type->has_floating_flag()) {
		dict[flag_floating_key] = true;
	}

	dict[position_key] =
		game_singleton->map_position_to_world_coords(unit.get_position()->get_province_definition().get_unit_position());

	if (display_unit_type->get_unit_category() != UnitType::unit_category_t::INFANTRY) {
		dict[rotation_key] = -0.25f * std::numbers::pi_v<float>;
	}

	dict[primary_colour_key] = Utilities::to_godot_color(country->get_primary_unit_colour());
	dict[secondary_colour_key] = Utilities::to_godot_color(country->get_secondary_unit_colour());
	dict[tertiary_colour_key] = Utilities::to_godot_color(country->get_tertiary_unit_colour());

	// TODO - move dict into unit_array ?
	unit_array.push_back(dict);

	return ret;
}

TypedArray<Dictionary> ModelSingleton::get_units() const {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, {});
	InstanceManager const* instance_manager = game_singleton->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	TypedArray<Dictionary> ret;

	for (ProvinceInstance const& province : instance_manager->get_map_instance().get_province_instances()) {
		if (province.get_province_definition().is_water()) {
			if (!add_unit_dict(province.get_navies(), ret)) {
				UtilityFunctions::push_error(
					"Error adding navy to province \"", std_view_to_godot_string(province.get_identifier()), "\""
				);
			}
		} else {
			if (!add_unit_dict(province.get_armies(), ret)) {
				UtilityFunctions::push_error(
					"Error adding army to province \"", std_view_to_godot_string(province.get_identifier()), "\""
				);
			}
		}

		// TODO - land units in ships
	}

	return ret;
}

Dictionary ModelSingleton::get_cultural_gun_model(String const& culture) const {
	static constexpr std::string_view gun_actor_name = "Gun1";

	GFX::Actor const* actor = get_cultural_actor(godot_to_std_string(culture), gun_actor_name, {});

	ERR_FAIL_NULL_V(actor, {});

	return make_model_dict(*actor);
}

Dictionary ModelSingleton::get_cultural_helmet_model(String const& culture) const {
	static constexpr std::string_view helmet_actor_name = "Helmet1";

	GFX::Actor const* actor = get_cultural_actor(godot_to_std_string(culture), helmet_actor_name, {});

	ERR_FAIL_NULL_V(actor, {});

	return make_model_dict(*actor);
}

Dictionary ModelSingleton::get_flag_model(bool floating) const {
	static constexpr std::string_view flag_name = "Flag";
	static constexpr std::string_view flag_floating_name = "FlagFloating";

	GFX::Actor const* actor = get_actor(floating ? flag_floating_name : flag_name);

	ERR_FAIL_NULL_V(actor, {});

	return make_model_dict(*actor);
}

bool ModelSingleton::add_building_dict(
	BuildingInstance const& building, ProvinceInstance const& province, TypedArray<Dictionary>& building_array
) const {
	ProvinceDefinition const& province_definition = province.get_province_definition();

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, false);

	static const StringName model_key = "model";
	static const StringName position_key = "position";
	static const StringName rotation_key = "rotation";

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
	const float rotation = province_definition.get_building_rotation(&building.get_building_type());

	const std::string actor_name = StringUtils::append_string_views("building_", building.get_identifier(), suffix);

	GFX::Actor const* actor = get_actor(actor_name);
	ERR_FAIL_NULL_V_MSG(
		actor, false, vformat(
			"Failed to find \"%s\" actor for building \"%s\" in province \"%s\"",
			std_to_godot_string(actor_name), std_view_to_godot_string(building.get_identifier()),
			std_view_to_godot_string(province.get_identifier())
		)
	);

	Dictionary dict;

	dict[model_key] = make_model_dict(*actor);

	dict[position_key] = game_singleton->map_position_to_world_coords(
		position_ptr != nullptr ? *position_ptr : province_definition.get_centre()
	);

	if (rotation != 0.0f) {
		dict[rotation_key] = rotation;
	}

	// TODO - move dict into unit_array ?
	building_array.push_back(dict);

	return true;
}

TypedArray<Dictionary> ModelSingleton::get_buildings() const {
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
						"Error adding building \"", std_view_to_godot_string(building.get_identifier()), "\" to province \"",
						std_view_to_godot_string(province.get_identifier()), "\""
					);
				}
			}
		}
	}

	return ret;
}
