#include "GUINode.hpp"

#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

#define APPLY_TO_CHILD_TYPES(F) \
	F(Button, button) \
	F(CheckBox, check_box) \
	F(Label, label) \
	F(Panel, panel) \
	F(TextureProgressBar, progress_bar) \
	F(TextureRect, texture_rect) \
	F(GUIOverlappingElementsBox, gui_overlapping_elements_box) \
	F(GUIScrollbar, gui_scrollbar) \
	F(GUIListBox, gui_listbox)

#define APPLY_TO_TEXTURE_TYPES(F) \
	F(GFXSpriteTexture, gfx_sprite_texture) \
	F(GFXMaskedFlagTexture, gfx_masked_flag_texture) \
	F(GFXPieChartTexture, gfx_pie_chart_texture)

void GUINode::_bind_methods() {
	OV_BIND_SMETHOD(generate_gui_element, { "gui_scene", "gui_element", "name" }, DEFVAL(String {}));
	OV_BIND_METHOD(GUINode::add_gui_element, { "gui_scene", "gui_element", "name" }, DEFVAL(String {}));
	OV_BIND_SMETHOD(get_gui_position, { "gui_scene", "gui_position" });

#define GET_BINDINGS(type, name) \
	OV_BIND_SMETHOD(get_##name##_from_node, { "node" }); \
	OV_BIND_METHOD(GUINode::get_##name##_from_nodepath, { "path" });

	APPLY_TO_CHILD_TYPES(GET_BINDINGS)

	OV_BIND_SMETHOD(get_texture_from_node, { "node" });
	OV_BIND_METHOD(GUINode::get_texture_from_nodepath, { "path" });

	APPLY_TO_TEXTURE_TYPES(GET_BINDINGS)

#undef GET_BINDINGS

	OV_BIND_METHOD(GUINode::hide_node, { "path" });
	OV_BIND_METHOD(GUINode::hide_nodes, { "paths" });

	OV_BIND_SMETHOD(int_to_formatted_string, { "val" });
	OV_BIND_SMETHOD(float_to_formatted_string, { "val", "decimal_places" });
	OV_BIND_SMETHOD(format_province_name, { "province_identifier" });
}

GUINode::GUINode() {
	set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	set_h_grow_direction(GROW_DIRECTION_BOTH);
	set_v_grow_direction(GROW_DIRECTION_BOTH);
	set_mouse_filter(MOUSE_FILTER_IGNORE);
}

Control* GUINode::generate_gui_element(String const& gui_scene, String const& gui_element, String const& name) {
	Control* result = nullptr;
	if (!UITools::generate_gui_element(gui_scene, gui_element, name, result)) {
		UtilityFunctions::push_error("Error generating GUI element ", gui_element, " from GUI scene ", gui_scene);
	}
	return result;
}

Error GUINode::add_gui_element(String const& gui_scene, String const& gui_element, String const& name) {
	Error err = OK;
	Control* result = nullptr;
	if (!UITools::generate_gui_element(gui_scene, gui_element, name, result)) {
		UtilityFunctions::push_error("Error generating GUI element ", gui_element, " from GUI scene ", gui_scene);
		err = FAILED;
	}
	if (result != nullptr) {
		add_child(result);
	}
	return err;
}

Vector2 GUINode::get_gui_position(String const& gui_scene, String const& gui_position) {
	GUI::Position const* position = UITools::get_gui_position(gui_scene, gui_position);
	ERR_FAIL_NULL_V(position, {});
	return Utilities::to_godot_fvec2(position->get_position());
}

template<std::derived_from<godot::Node> T>
static T* _cast_node(Node* node) {
	ERR_FAIL_NULL_V(node, nullptr);
	T* result = Object::cast_to<T>(node);
	ERR_FAIL_NULL_V_MSG(
		result, nullptr,
		vformat("Failed to cast node %s from type %s to %s", node->get_name(), node->get_class(), T::get_class_static())
	);
	return result;
}

#define CHILD_GET_FUNCTIONS(type, name) \
	type* GUINode::get_##name##_from_node(Node* node) { \
		return _cast_node<type>(node); \
	} \
	type* GUINode::get_##name##_from_nodepath(NodePath const& path) const { \
		return _cast_node<type>(get_node_internal(path)); \
	}

APPLY_TO_CHILD_TYPES(CHILD_GET_FUNCTIONS)

#undef CHILD_GET_FUNCTIONS

Ref<Texture2D> GUINode::get_texture_from_node(Node* node) {
	ERR_FAIL_NULL_V(node, nullptr);
	if (TextureRect const* texture_rect = Object::cast_to<TextureRect>(node); texture_rect != nullptr) {
		const Ref<Texture2D> texture = texture_rect->get_texture();
		ERR_FAIL_NULL_V_MSG(texture, nullptr, vformat("Failed to get Texture2D from TextureRect %s", node->get_name()));
		return texture;
	} else if (Button const* button = Object::cast_to<Button>(node); button != nullptr) {
		static const StringName theme_name_normal = "normal";
		const Ref<StyleBox> stylebox = button->get_theme_stylebox(theme_name_normal);
		ERR_FAIL_NULL_V_MSG(
			stylebox, nullptr, vformat("Failed to get StyleBox %s from Button %s", theme_name_normal, node->get_name())
		);
		const Ref<StyleBoxTexture> stylebox_texture = stylebox;
		ERR_FAIL_NULL_V_MSG(
			stylebox_texture, nullptr, vformat(
				"Failed to cast StyleBox %s from Button %s to type StyleBoxTexture", theme_name_normal, node->get_name()
			)
		);
		const Ref<Texture2D> result = stylebox_texture->get_texture();
		ERR_FAIL_NULL_V_MSG(
			result, nullptr,
			vformat("Failed to get Texture2D from StyleBoxTexture %s from Button %s", theme_name_normal, node->get_name())
		);
		return result;
	}
	ERR_FAIL_V_MSG(
		nullptr, vformat("Failed to cast node %s from type %s to TextureRect or Button", node->get_name(), node->get_class())
	);
}

Ref<Texture2D> GUINode::get_texture_from_nodepath(NodePath const& path) const {
	return get_texture_from_node(get_node_internal(path));
}

template<std::derived_from<Texture2D> T>
static Ref<T> _cast_texture(Ref<Texture2D> const& texture) {
	ERR_FAIL_NULL_V(texture, nullptr);
	const Ref<T> result = texture;
	ERR_FAIL_NULL_V_MSG(
		result, nullptr, vformat("Failed to cast Texture2D from type %s to %s", texture->get_class(), T::get_class_static())
	);
	return result;
}

#define TEXTURE_GET_FUNCTIONS(type, name) \
	Ref<type> GUINode::get_##name##_from_node(Node* node) { \
		return _cast_texture<type>(get_texture_from_node(node)); \
	} \
	Ref<type> GUINode::get_##name##_from_nodepath(NodePath const& path) const { \
		return _cast_texture<type>(get_texture_from_nodepath(path)); \
	}

APPLY_TO_TEXTURE_TYPES(TEXTURE_GET_FUNCTIONS)

#undef TEXTURE_GET_FUNCTIONS

#undef APPLY_TO_CHILD_TYPES

Error GUINode::hide_node(NodePath const& path) const {
	CanvasItem* node = _cast_node<CanvasItem>(get_node_internal(path));
	ERR_FAIL_NULL_V(node, FAILED);
	node->hide();
	return OK;
}

Error GUINode::hide_nodes(TypedArray<NodePath> const& paths) const {
	Error ret = OK;
	for (int32_t i = 0; i < paths.size(); ++i) {
		if (hide_node(paths[i]) != OK) {
			ret = FAILED;
		}
	}
	return ret;
}

String GUINode::int_to_formatted_string(int64_t val) {
	return Utilities::int_to_formatted_string(val);
}

String GUINode::float_to_formatted_string(float val, int32_t decimal_places) {
	return Utilities::float_to_formatted_string(val, decimal_places);
}

String GUINode::format_province_name(String const& province_identifier) {
	static const String province_prefix = "PROV";
	return province_prefix + province_identifier;
}
