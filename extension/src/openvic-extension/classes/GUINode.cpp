#include "GUINode.hpp"

#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/UIAdapter.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

using OpenVic::Utilities::godot_to_std_string;
using OpenVic::Utilities::std_view_to_godot_string;

void GUINode::_bind_methods() {
	OV_BIND_METHOD(GUINode::add_gui_element, { "gui_file", "gui_element", "name" }, DEFVAL(String {}));

	OV_BIND_METHOD(GUINode::get_button_node, { "path" });
	OV_BIND_METHOD(GUINode::get_check_box_node, { "path" });
	OV_BIND_METHOD(GUINode::get_label_node, { "path" });
	OV_BIND_METHOD(GUINode::get_panel_node, { "path" });
	OV_BIND_METHOD(GUINode::get_progress_bar_node, { "path" });
	OV_BIND_METHOD(GUINode::get_texture_rect_node, { "path" });

	OV_BIND_METHOD(GUINode::get_texture_from_node, { "path" });
	OV_BIND_METHOD(GUINode::get_gfx_icon_texture_from_node, { "path" });
	OV_BIND_METHOD(GUINode::get_gfx_masked_flag_texture_from_node, { "path" });
	OV_BIND_METHOD(GUINode::get_gfx_pie_chart_texture_from_node, { "path" });

	OV_BIND_METHOD(GUINode::hide_node, { "path" });
	OV_BIND_METHOD(GUINode::hide_nodes, { "paths" });
}

Error GUINode::_add_gui_element(GUI::Element const* element, String const& name) {
	ERR_FAIL_NULL_V(element, FAILED);
	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);
	Error err = OK;
	Control* result = nullptr;
	if (!GodotGUIBuilder::generate_element(element, name, *asset_manager, result)) {
		UtilityFunctions::push_error("Failed to generate GUI element ", std_view_to_godot_string(element->get_name()));
		err = FAILED;
	}
	if (result != nullptr) {
		add_child(result);
	}
	return err;
}

Error GUINode::add_gui_element(String const& gui_file, String const& gui_element, String const& name) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, FAILED);
	GUI::Scene const* scene =
		game_singleton->get_game_manager().get_ui_manager().get_scene_by_identifier(godot_to_std_string(gui_file));
	ERR_FAIL_NULL_V_MSG(scene, FAILED, vformat("Failed to find GUI file %s", gui_file));
	GUI::Element const* element = scene->get_scene_element_by_identifier(godot_to_std_string(gui_element));
	ERR_FAIL_NULL_V_MSG(element, FAILED, vformat("Failed to find GUI element %s in GUI file %s", gui_element, gui_file));
	return _add_gui_element(element, name);
}

template<std::derived_from<godot::Node> T>
T* GUINode::_get_cast_node(NodePath const& path) const {
	Node* node = get_node_or_null(path);
	ERR_FAIL_NULL_V_MSG(node, nullptr, vformat("Failed to find node %s", path));
	T* result = Object::cast_to<T>(node);
	ERR_FAIL_NULL_V_MSG(result, nullptr, vformat("Failed to cast node %s to type %s", path, T::get_class_static()));
	return result;
}

Button* GUINode::get_button_node(NodePath const& path) const {
	return _get_cast_node<Button>(path);
}

CheckBox* GUINode::get_check_box_node(NodePath const& path) const {
	return _get_cast_node<CheckBox>(path);
}

Label* GUINode::get_label_node(NodePath const& path) const {
	return _get_cast_node<Label>(path);
}

Panel* GUINode::get_panel_node(NodePath const& path) const {
	return _get_cast_node<Panel>(path);
}

TextureProgressBar* GUINode::get_progress_bar_node(NodePath const& path) const {
	return _get_cast_node<TextureProgressBar>(path);
}

TextureRect* GUINode::get_texture_rect_node(NodePath const& path) const {
	return _get_cast_node<TextureRect>(path);
}

Ref<Texture2D> GUINode::get_texture_from_node(NodePath const& path) const {
	Node* node = get_node_or_null(path);
	ERR_FAIL_NULL_V_MSG(node, nullptr, vformat("Failed to find node %s", path));
	if (TextureRect const* texture_rect = Object::cast_to<TextureRect>(node); texture_rect != nullptr) {
		const Ref<Texture2D> texture = texture_rect->get_texture();
		ERR_FAIL_NULL_V_MSG(texture, nullptr, vformat("Failed to get Texture2D from TextureRect %s", path));
		return texture;
	} else if (Button const* button = Object::cast_to<Button>(node); button != nullptr) {
		static const StringName theme_name_normal = "normal";
		const Ref<StyleBox> stylebox = button->get_theme_stylebox(theme_name_normal);
		ERR_FAIL_NULL_V_MSG(stylebox, nullptr, vformat("Failed to get StyleBox %s from Button %s", theme_name_normal, path));
		const Ref<StyleBoxTexture> stylebox_texture = stylebox;
		ERR_FAIL_NULL_V_MSG(
			stylebox_texture, nullptr, vformat(
				"Failed to cast StyleBox %s from Button %s to type StyleBoxTexture", theme_name_normal, path
			)
		);
		const Ref<Texture2D> result = stylebox_texture->get_texture();
		ERR_FAIL_NULL_V_MSG(
			result, nullptr, vformat("Failed to get Texture2D from StyleBoxTexture %s from Button %s", theme_name_normal, path)
		);
		return result;
	}
	ERR_FAIL_V_MSG(nullptr, vformat("Failed to cast node %s to type TextureRect or Button", path));
}

template<std::derived_from<godot::Texture2D> T>
Ref<T> GUINode::_get_cast_texture_from_node(NodePath const& path) const {
	const Ref<Texture2D> texture = get_texture_from_node(path);
	ERR_FAIL_NULL_V(texture, nullptr);
	const Ref<T> result = texture;
	ERR_FAIL_NULL_V_MSG(result, nullptr, vformat("Failed to cast Texture2D from %s to type %s", path, T::get_class_static()));
	return result;
}

Ref<GFXIconTexture> GUINode::get_gfx_icon_texture_from_node(NodePath const& path) const {
	return _get_cast_texture_from_node<GFXIconTexture>(path);
}

Ref<GFXMaskedFlagTexture> GUINode::get_gfx_masked_flag_texture_from_node(NodePath const& path) const {
	return _get_cast_texture_from_node<GFXMaskedFlagTexture>(path);
}

Ref<GFXPieChartTexture> GUINode::get_gfx_pie_chart_texture_from_node(NodePath const& path) const {
	return _get_cast_texture_from_node<GFXPieChartTexture>(path);
}

Error GUINode::hide_node(NodePath const& path) const {
	CanvasItem* node = _get_cast_node<CanvasItem>(path);
	if (node == nullptr) {
		return FAILED;
	}
	node->hide();
	return OK;
}

Error GUINode::hide_nodes(Array const& paths) const {
	Error ret = OK;
	for (int32_t i = 0; i < paths.size(); ++i) {
		if (hide_node(paths[i]) != OK) {
			ret = FAILED;
		}
	}
	return ret;
}
