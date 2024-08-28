#include "GUIPieChart.hpp"

#include "openvic-extension/utility/ClassBindings.hpp"

using namespace godot;
using namespace OpenVic;
using namespace OpenVic::Utilities::literals;

void GUIPieChart::_bind_methods() {
	OV_BIND_METHOD(GUIPieChart::get_gfx_pie_chart_texture);
	OV_BIND_METHOD(GUIPieChart::set_gfx_pie_chart_name, { "gfx_pie_chart_name" });
	OV_BIND_METHOD(GUIPieChart::get_gfx_pie_chart_name);
	OV_BIND_METHOD(GUIPieChart::set_slices_array, { "new_slices" });
}

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

	return err;
}

String GUIPieChart::get_gfx_pie_chart_name() const {
	ERR_FAIL_NULL_V(gfx_pie_chart_texture, {});

	return gfx_pie_chart_texture->get_gfx_pie_chart_name();
}

Error GUIPieChart::set_slices_array(GFXPieChartTexture::godot_pie_chart_data_t const& new_slices) const {
	ERR_FAIL_NULL_V(gfx_pie_chart_texture, FAILED);

	return gfx_pie_chart_texture->set_slices_array(new_slices);
}
