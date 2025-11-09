#include "GUINode.hpp"

#include <limits>

#include <godot_cpp/classes/bit_map.hpp>
#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/texture_progress_bar.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include "openvic-extension/core/Bind.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/utility/UITools.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

#define APPLY_TO_CHILD_TYPES(F) \
	F(GUIIconButton, gui_icon_button) \
	F(GUIMaskedFlagButton, gui_masked_flag_button) \
	F(GUILabel, gui_label) \
	F(Panel, panel) \
	F(GUIProgressBar, gui_progress_bar) \
	F(GUIIcon, gui_icon) \
	F(GUIMaskedFlag, gui_masked_flag) \
	F(GUIPieChart, gui_pie_chart) \
	F(GUIOverlappingElementsBox, gui_overlapping_elements_box) \
	F(GUIScrollbar, gui_scrollbar) \
	F(GUIListBox, gui_listbox) \
	F(LineEdit, line_edit) \
	F(GUILineChart, gui_line_chart)

void GUINode::_bind_methods() {
	OV_BIND_SMETHOD(generate_gui_element, { "gui_scene", "gui_element", "name" }, DEFVAL(String {}));
	OV_BIND_METHOD(GUINode::add_gui_element, { "gui_scene", "gui_element", "name" }, DEFVAL(String {}));
	OV_BIND_SMETHOD(get_gui_position, { "gui_scene", "gui_position" });

	OV_BIND_METHOD(GUINode::get_click_mask);
	OV_BIND_METHOD(GUINode::set_click_mask, { "mask" });
	ADD_PROPERTY(
		PropertyInfo(Variant::OBJECT, "click_mask", PROPERTY_HINT_RESOURCE_TYPE, "BitMap"), "set_click_mask", "get_click_mask"
	);

	OV_BIND_METHOD(GUINode::set_click_mask_from_nodepaths, { "paths" });
	OV_BIND_METHOD(GUINode::update_click_mask);

#define GET_BINDINGS(type, name) \
	OV_BIND_SMETHOD(get_##name##_from_node, { "node" }); \
	OV_BIND_SMETHOD(get_##name##_from_node_and_path, { "node", "path" }); \
	OV_BIND_METHOD(GUINode::get_##name##_from_nodepath, { "path" });

	APPLY_TO_CHILD_TYPES(GET_BINDINGS)

#undef GET_BINDINGS

	OV_BIND_SMETHOD(get_texture_from_node, { "node" });
	OV_BIND_METHOD(GUINode::get_texture_from_nodepath, { "path" });

	OV_BIND_METHOD(GUINode::hide_node, { "path" });
	OV_BIND_METHOD(GUINode::hide_nodes, { "paths" });

	OV_BIND_METHOD(GUINode::remove_node, { "path" });
	OV_BIND_METHOD(GUINode::remove_nodes, { "paths" });

	OV_BIND_SMETHOD(int_to_string_suffixed, { "val" });
	OV_BIND_SMETHOD(int_to_string_commas, { "val" });
	OV_BIND_SMETHOD(float_to_string_suffixed, { "val" });
	OV_BIND_SMETHOD(float_to_string_dp, { "val", "decimal_places" });
	OV_BIND_SMETHOD(float_to_string_dp_dynamic, { "val" });
	OV_BIND_SMETHOD(format_province_name, { "province_identifier", "ignore_empty" }, DEFVAL(false));
}

GUINode::GUINode() {
	set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	set_h_grow_direction(GROW_DIRECTION_BOTH);
	set_v_grow_direction(GROW_DIRECTION_BOTH);
	set_mouse_filter(MOUSE_FILTER_STOP);
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
		Utilities::format("Failed to cast node %s from type %s to %s", node->get_name(), node->get_class(), T::get_class_static())
	);
	return result;
}

#define CHILD_GET_FUNCTIONS(type, name) \
	type* GUINode::get_##name##_from_node(Node* node) { \
		return _cast_node<type>(node); \
	} \
	type* GUINode::get_##name##_from_node_and_path(Node* node, NodePath const& path) { \
		return _cast_node<type>(node->get_node_internal(path)); \
	} \
	type* GUINode::get_##name##_from_nodepath(NodePath const& path) const { \
		return _cast_node<type>(get_node_internal(path)); \
	}

APPLY_TO_CHILD_TYPES(CHILD_GET_FUNCTIONS)

#undef CHILD_GET_FUNCTIONS

#undef APPLY_TO_CHILD_TYPES

Ref<Texture2D> GUINode::get_texture_from_node(Node* node) {
	ERR_FAIL_NULL_V(node, nullptr);
	if (TextureRect const* texture_rect = Object::cast_to<TextureRect>(node); texture_rect != nullptr) {
		const Ref<Texture2D> texture = texture_rect->get_texture();
		ERR_FAIL_NULL_V_MSG(texture, nullptr, Utilities::format("Failed to get Texture2D from TextureRect %s", node->get_name()));
		return texture;
	} else if (GUIButton const* button = Object::cast_to<GUIButton>(node); button != nullptr) {
		const Ref<StyleBox> stylebox = button->get_theme_stylebox(OV_SNAME(normal));
		ERR_FAIL_NULL_V_MSG(
			stylebox, nullptr, Utilities::format("Failed to get StyleBox %s from GUIButton %s", OV_SNAME(normal), node->get_name())
		);
		const Ref<StyleBoxTexture> stylebox_texture = stylebox;
		ERR_FAIL_NULL_V_MSG(
			stylebox_texture, nullptr, Utilities::format(
				"Failed to cast StyleBox %s from GUIButton %s to type StyleBoxTexture", OV_SNAME(normal), node->get_name()
			)
		);
		const Ref<Texture2D> result = stylebox_texture->get_texture();
		ERR_FAIL_NULL_V_MSG(
			result, nullptr,
			Utilities::format("Failed to get Texture2D from StyleBoxTexture %s from GUIButton %s", OV_SNAME(normal), node->get_name())
		);
		return result;
	}
	ERR_FAIL_V_MSG(
		nullptr, Utilities::format(
			"Failed to cast node %s from type %s to TextureRect or GUIButton", node->get_name(), node->get_class()
		)
	);
}

Ref<Texture2D> GUINode::get_texture_from_nodepath(NodePath const& path) const {
	return get_texture_from_node(get_node_internal(path));
}

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

Error GUINode::remove_node(NodePath const& path) const {
	Node* node = get_node_internal(path);
	ERR_FAIL_NULL_V(node, FAILED);

	Node* parent = node->get_parent();
	ERR_FAIL_NULL_V(parent, FAILED);

	parent->remove_child(node);
	node->queue_free();

	return OK;
}

Error GUINode::remove_nodes(TypedArray<NodePath> const& paths) const {
	Error ret = OK;

	for (int32_t i = 0; i < paths.size(); ++i) {
		if (remove_node(paths[i]) != OK) {
			ret = FAILED;
		}
	}

	return ret;
}

String GUINode::int_to_string_suffixed(int64_t val) {
	return Utilities::int_to_string_suffixed(val);
}

String GUINode::int_to_string_commas(int64_t val) {
	return Utilities::int_to_string_commas(val);
}

String GUINode::float_to_string_suffixed(float val) {
	return Utilities::float_to_string_suffixed(val);
}

String GUINode::float_to_string_dp(float val, int32_t decimal_places) {
	return Utilities::float_to_string_dp(val, decimal_places);
}

String GUINode::float_to_string_dp_dynamic(float val) {
	return Utilities::float_to_string_dp_dynamic(val);
}

String GUINode::format_province_name(String const& province_identifier, bool ignore_empty) {
	if (!province_identifier.is_empty()) {
		static const String province_prefix = "PROV";
		return province_prefix + province_identifier;
	} else if (!ignore_empty) {
		static const String no_province = "NO PROVINCE";
		return no_province;
	} else {
		return {};
	}
}

Ref<BitMap> GUINode::get_click_mask() const {
	return _click_mask;
}

void GUINode::set_click_mask(Ref<BitMap> const& mask) {
	if (_click_mask == mask) {
		return;
	}
	_mask_controls.clear();
	_click_mask = mask;
	queue_redraw();
	update_minimum_size();
}

bool GUINode::_update_click_mask_for(Ref<Image> const& img, int index) {
	ERR_FAIL_INDEX_V(index, _mask_controls.size(), false);
	Control* control = _mask_controls[index];
	if (!ObjectDB::get_instance(control->get_instance_id()) && !control->is_inside_tree()) {
		_mask_controls.remove_at(index);
		return false;
	}
	ERR_FAIL_COND_V(img.is_null(), false);
	Ref<Texture2D> texture = get_texture_from_node(control);
	ERR_FAIL_COND_V(texture.is_null(), false);
	Ref<Image> texture_img = texture->get_image();
	if (img->is_empty()) {
		img->copy_from(texture_img);
	} else {
		if (img->get_format() != texture_img->get_format()) {
			img->convert(texture_img->get_format());
		}
		Vector2i img_size = img->get_size();
		Vector2i total_size = control->get_screen_position() + texture_img->get_size();
		Vector2i new_img_size = img_size.max(total_size);
		if (new_img_size != img_size) {
			img->crop(new_img_size.x, new_img_size.y);
		}
		img->blend_rect(texture_img, texture_img->get_used_rect(), control->get_position());
	}
	ERR_FAIL_COND_V(img->is_empty(), false);
	return true;
}

void GUINode::update_click_mask() {
	static constexpr real_t max_real = std::numeric_limits<real_t>::max();
	static const Point2 max_point { max_real, max_real };
	if (_mask_controls.size() == 0) {
		return;
	}

	if (_click_mask.is_null()) {
		_click_mask.instantiate();
	}
	Ref<Image> img;
	img.instantiate();
	Vector2 size = get_size();
	img->create(size.x, size.y, false, Image::Format::FORMAT_RGBA8);
	Point2 highest_position = { max_real, max_real };
	for (int index = 0; index < _mask_controls.size(); index++) {
		if (!_update_click_mask_for(img, index)) {
			continue;
		}
		Vector2 screen_pos = _mask_controls[index]->get_screen_position();
		highest_position = highest_position.min(screen_pos);
	}
	ERR_FAIL_COND(img.is_null());
	ERR_FAIL_COND(highest_position == max_point);
	_texture_region = Rect2(Point2(), img->get_size());
	_position_rect = Rect2(highest_position, _texture_region.get_size());
	_click_mask->create_from_image_alpha(img, 0.9);
	queue_redraw();
	update_minimum_size();
}

void GUINode::set_click_mask_from_nodepaths(TypedArray<NodePath> const& paths) {
	// TODO: Update to use https://github.com/godotengine/godot/pull/90916
	// for(godot::Control* control : _mask_controls) {
	// 	control->set_mouse_filter(Control::MouseFilter::MOUSE_FILTER_STOP);
	// }
	_mask_controls.clear();
	for (int index = 0; index < paths.size(); index++) {
		Control* control = _cast_node<Control>(get_node_internal(paths[index]));
		ERR_CONTINUE(control == nullptr);
		control->set_mouse_filter(Control::MouseFilter::MOUSE_FILTER_IGNORE);
		_mask_controls.push_back(control);
	}
	update_click_mask();
}

bool GUINode::_has_point(Vector2 const& p_point) const {
	if (!_click_mask.is_valid()) {
		return Control::_has_point(p_point);
	}

	Point2 point = p_point;
	Rect2 rect;
	Size2 mask_size = _click_mask->get_size();

	if (!_position_rect.has_area()) {
		rect.size = mask_size;
	} else {
		// we need to transform the point from our scaled / translated image back to our mask image
		Point2 ofs = _position_rect.position;
		Size2 scale = mask_size / _position_rect.size;

		// offset and scale the new point position to adjust it to the bitmask size
		point -= ofs;
		point *= scale;

		// finally, we need to check if the point is inside a rectangle with a position >= 0,0 and a size <= mask_size
		rect.position = _texture_region.position.min(Point2 {});
		rect.size = mask_size.min(_texture_region.size);
	}

	if (!rect.has_point(point)) {
		return false;
	}

	return _click_mask->get_bitv(point);
}
