#include "GUIPieChart.hpp"

#include <godot_cpp/classes/input_event_mouse_motion.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/singletons/MenuSingleton.hpp"
#include "openvic-extension/utility/ClassBindings.hpp"

using namespace godot;
using namespace OpenVic;
using namespace OpenVic::Utilities::literals;

void GUIPieChart::_update_tooltip() {
	MenuSingleton* menu_singleton = MenuSingleton::get_singleton();
	ERR_FAIL_NULL(menu_singleton);

	if (gfx_pie_chart_texture.is_valid()) {
		GFXPieChartTexture::slice_t const* slice = gfx_pie_chart_texture->get_slice(tooltip_position);

		if (slice != nullptr) {
			static const String tooltip_identifier_key = "ID";
			static const String tooltip_percent_key = "PC";
			// "§Y$ID$§!: $PC$%"
			static const String tooltip_string =
				GUILabel::get_colour_marker() + String { "Y" } + GUILabel::get_substitution_marker() + tooltip_identifier_key
				+ GUILabel::get_substitution_marker() + GUILabel::get_colour_marker() + "!: "
				+ GUILabel::get_substitution_marker() + tooltip_percent_key + GUILabel::get_substitution_marker() + "%";

			Dictionary substitution_dict;
			substitution_dict[tooltip_identifier_key] = slice->name;

			float percent = slice->weight * 100.0f;
			if (gfx_pie_chart_texture->get_total_weight() > 0.0f) {
				percent /= gfx_pie_chart_texture->get_total_weight();
			}
			substitution_dict[tooltip_percent_key] = Utilities::float_to_string_dp(percent, 2);

			menu_singleton->show_control_tooltip(tooltip_string, substitution_dict, this);

			tooltip_active = true;
			return;
		}
	}

	menu_singleton->hide_tooltip();
	tooltip_active = false;
}

void GUIPieChart::_bind_methods() {
	OV_BIND_METHOD(GUIPieChart::get_gfx_pie_chart_texture);
	OV_BIND_METHOD(GUIPieChart::set_gfx_pie_chart_name, { "gfx_pie_chart_name" });
	OV_BIND_METHOD(GUIPieChart::get_gfx_pie_chart_name);
	OV_BIND_METHOD(GUIPieChart::set_slices_array, { "new_slices" });
}

static const Vector2 disabled_tooltip_position { -1.0_real, -1.0_real };

void GUIPieChart::_notification(int what) {
	if (what == NOTIFICATION_MOUSE_EXIT_SELF) {
		tooltip_position = disabled_tooltip_position;

		_update_tooltip();
	}
}

void GUIPieChart::_gui_input(Ref<InputEvent> const& event) {
	Ref<InputEventMouseMotion> mm = event;

	if (mm.is_valid()) {
		tooltip_position = mm->get_position() * 2.0_real / get_size() - Vector2 { 1.0_real, 1.0_real };

		_update_tooltip();
	}
}

GUIPieChart::GUIPieChart() : tooltip_position { disabled_tooltip_position } {}

Error GUIPieChart::set_gfx_pie_chart(GFX::PieChart const* gfx_pie_chart) {
	const bool needs_setting = gfx_pie_chart_texture.is_null();

	if (needs_setting) {
		gfx_pie_chart_texture.instantiate();
		ERR_FAIL_NULL_V(gfx_pie_chart_texture, FAILED);
	}

	const Error err = gfx_pie_chart_texture->set_gfx_pie_chart(gfx_pie_chart);

	if (needs_setting) {
		set_texture(gfx_pie_chart_texture);
	}

	if (tooltip_active) {
		_update_tooltip();
	}

	return err;
}

Ref<GFXPieChartTexture> GUIPieChart::get_gfx_pie_chart_texture() const {
	ERR_FAIL_NULL_V(gfx_pie_chart_texture, nullptr);

	return gfx_pie_chart_texture;
}

Error GUIPieChart::set_gfx_pie_chart_name(String const& gfx_pie_chart_name) {
	const bool needs_setting = gfx_pie_chart_texture.is_null();

	if (needs_setting) {
		gfx_pie_chart_texture.instantiate();
		ERR_FAIL_NULL_V(gfx_pie_chart_texture, FAILED);
	}

	const Error err = gfx_pie_chart_texture->set_gfx_pie_chart_name(gfx_pie_chart_name);

	if (needs_setting) {
		set_texture(gfx_pie_chart_texture);
	}

	if (tooltip_active) {
		_update_tooltip();
	}

	return err;
}

String GUIPieChart::get_gfx_pie_chart_name() const {
	ERR_FAIL_NULL_V(gfx_pie_chart_texture, {});

	return gfx_pie_chart_texture->get_gfx_pie_chart_name();
}

Error GUIPieChart::set_slices_array(GFXPieChartTexture::godot_pie_chart_data_t const& new_slices) {
	ERR_FAIL_NULL_V(gfx_pie_chart_texture, FAILED);

	const Error err = gfx_pie_chart_texture->set_slices_array(new_slices);

	if (tooltip_active) {
		_update_tooltip();
	}

	return err;
}
