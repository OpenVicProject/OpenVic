#include "Utilities.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-simulation/politics/Government.hpp"
#include <gli/convert.hpp>
#include <gli/load_dds.hpp>

#include <openvic-simulation/country/CountryInstance.hpp>
#include <openvic-simulation/map/ProvinceInstance.hpp>
#include <openvic-simulation/map/Region.hpp>
#include <openvic-simulation/map/State.hpp>

#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/core/StaticString.hpp"

using namespace godot;
using namespace OpenVic;

godot::String Utilities::get_short_value_placeholder() {
	return "$VAL$";
}
godot::String Utilities::get_long_value_placeholder() {
	return "$VALUE$";
}
godot::String Utilities::get_percentage_value_placeholder() {
	return "$PERC$";
}
godot::String Utilities::get_colour_and_sign(const fixed_point_t value) {
	return value > 0 ? "G+" : value < 0 ? "R" : "Y";
}

/* Int to 2 decimal place string in terms of the largest suffix less than or equal to it,
 * or normal integer string if less than the smallest suffix. */
String Utilities::int_to_string_suffixed(int64_t val) {
	static const std::vector<std::pair<int64_t, String>> suffixes {
		{ 1'000'000'000'000, "T" },
		{ 1'000'000'000, "B" },
		{ 1'000'000, "M" },
		{ 1'000, "k" }
	};
	static constexpr int64_t decimal_places_multiplier = 100;
	const bool negative = val < 0;
	if (negative) {
		val = -val;
	}
	for (auto const& [suffix_val, suffix_str] : suffixes) {
		if (val >= suffix_val) {
			const int64_t whole = val / suffix_val;
			const int64_t frac = (val * decimal_places_multiplier / suffix_val) % decimal_places_multiplier;
			return (negative ? "-" : "") + String::num_int64(whole) + "." +
				(frac < 10 ? "0" : "") + String::num_int64(frac) + suffix_str;
		}
	}
	return (negative ? "-" : "") + String::num_int64(val);
}

String Utilities::int_to_string_commas(int64_t val) {
	const bool negative = val < 0;
	if (negative) {
		val = -val;
	}

	const String string_val = String::num_int64(val);

	String result;
	int64_t length_remaining = string_val.length();

	static constexpr int64_t digits_per_comma = 3;
	static const String comma = ",";

	while (length_remaining > digits_per_comma) {
		result = comma + string_val.substr(length_remaining -= digits_per_comma, digits_per_comma) + result;
	}

	result = string_val.substr(0, length_remaining) + result;

	if (negative) {
		result = "-" + result;
	}

	return result;
}

String Utilities::float_to_string_suffixed(float val) {
	const float abs_val = std::abs(val);

	if (abs_val < 10'000.0f) {
		return float_to_string_dp(val, 1);
	}

	if (abs_val < 1'000'000.0f) {
		return float_to_string_dp(val / 1'000.0f, 2) + "k";
	}

	if (abs_val < 1'000'000'000.0f) {
		return float_to_string_dp(val / 1'000'000.0f, 2) + "M";
	}

	if (abs_val < 1'000'000'000'000.0f) {
		return float_to_string_dp(val / 1'000'000'000.0f, 2) + "B";
	}

	return float_to_string_dp(val / 1'000'000'000'000.0f, 2) + "T";
}

/* Float to string formatted with the specified number of decimal places. */
String Utilities::float_to_string_dp(float val, int32_t decimal_places) {
	String result = String::num(val, decimal_places);
	if (decimal_places >= 0) {
		return result.pad_decimals(decimal_places);
	} else {
		return result;
	}
}

String Utilities::fixed_point_to_string_dp(fixed_point_t val, int32_t decimal_places) {
	// We could use fixed point's own to_string method, but that allocates an intermediate string so better to go via float
	// return Utilities::std_to_godot_string(val.to_string(decimal_places));
	return Utilities::float_to_string_dp(static_cast<float>(val), decimal_places);
}

String Utilities::percentage_to_string_dp(fixed_point_t val, int32_t decimal_places) {
	return Utilities::format(
		"%s%%",
		Utilities::float_to_string_dp(
			static_cast<float>(100 * val),
			decimal_places
		)
	);
}

String Utilities::float_to_string_dp_dynamic(float val) {
	const float abs_val = std::abs(val);
	return float_to_string_dp(val, abs_val < 2.0f ? 3 : abs_val < 10.0f ? 2 : 1);
}

String Utilities::cash_to_string_dp_dynamic(fixed_point_t val) {
	return format_with_currency(
		Utilities::float_to_string_dp_dynamic(
			static_cast<float>(val)
		)
	);
}

String Utilities::format_with_currency(godot::String const& text) {
	static const String currency_format = String::utf8("%s¤");
	return Utilities::format(currency_format, text);
}

String Utilities::date_to_string(Date date) {
	static const String date_template_string = String { "%d" } + Date::SEPARATOR_CHARACTER + "%d" +
		Date::SEPARATOR_CHARACTER + "%d";

	return Utilities::format(date_template_string, date.get_year(), date.get_month(), date.get_day());
}

/* Date formatted like one of these, with the month localised if possible:
 *  - "1 January, 1836" (if month_first is false)
 *  - "January 1, 1836" (if month_first is true) */
String Utilities::date_to_formatted_string(Date date, bool month_first) {
	String day = String::num_int64(date.get_day());
	String month = Utilities::std_to_godot_string(date.get_month_name());
	TranslationServer const* server = TranslationServer::get_singleton();
	if (server != nullptr) {
		month = server->translate(month);
	}
	String year = String::num_int64(date.get_year());

	if (month_first) {
		return month + " " + day + ", " + year;
	} else {
		return day + " " + month + ", " + year;
	}
}

Ref<Resource> Utilities::load_resource(String const& path, String const& type_hint) {
	ResourceLoader* loader = ResourceLoader::get_singleton();
	ERR_FAIL_NULL_V(loader, nullptr);
	return loader->load(path, type_hint);
}

static Ref<Image> load_dds_image(String const& path) {
	const gli::texture dds_file = gli::load_dds(Utilities::godot_to_std_string(path));
	ERR_FAIL_COND_V_MSG(dds_file.empty(), nullptr, Utilities::format("Failed to load DDS file: %s", path));
	gli::texture2d texture { dds_file };
	ERR_FAIL_COND_V_MSG(texture.empty(), nullptr, Utilities::format("Failed to create DDS texture: %s", path));

	static constexpr gli::format expected_format = gli::FORMAT_BGRA8_UNORM_PACK8;
	const bool needs_bgr_to_rgb = texture.format() == expected_format;
	if (!needs_bgr_to_rgb) {
		texture = gli::convert(texture, expected_format);
		ERR_FAIL_COND_V_MSG(texture.empty(), nullptr, Utilities::format("Failed to convert DDS file: %s", path));
	}

	const gli::texture2d::extent_type extent { texture.extent() };
	const int32_t width = extent.x, height = extent.y, size = width * height * 4;

	/* Only fail if there aren't enough bytes, everything seems to work fine if there are extra bytes and we ignore them */
	ERR_FAIL_COND_V_MSG(
		size > texture.size(), nullptr,
		Utilities::format("Texture size %d mismatched with dims-based size %d for %s", static_cast<int64_t>(texture.size()), size, path)
	);

	PackedByteArray pixels;
	ERR_FAIL_COND_V(pixels.resize(size) != OK, nullptr);

	/* Index offset used to control whether we are reading */
	const size_t rb_idx = 2 * needs_bgr_to_rgb;
	uint8_t const* ptr = static_cast<uint8_t const*>(texture.data());
	for (size_t i = 0; i < size; i += 4) {
		pixels[i + 0] = ptr[i + rb_idx];
		pixels[i + 1] = ptr[i + 1];
		pixels[i + 2] = ptr[i + 2 - rb_idx];
		pixels[i + 3] = ptr[i + 3];
	}

	return Image::create_from_data(width, height, false, Image::FORMAT_RGBA8, pixels);
}

Ref<Image> Utilities::load_godot_image(String const& path) {
	if (path.ends_with(".dds")) {
		return load_dds_image(path);
	}
	return Image::load_from_file(path);
}

Ref<FontFile> Utilities::load_godot_font(String const& fnt_path, Ref<Image> const& image) {
	ERR_FAIL_NULL_V(image, nullptr);
	Ref<FontFile> font;
	font.instantiate();
	ERR_FAIL_COND_V_MSG(font->load_bitmap_font(fnt_path) != OK, nullptr, Utilities::format("Failed to load font: %s", fnt_path));
	font->set_texture_image(0, { font->get_fixed_size(), 0 }, 0, image);
	return font;
}

Ref<Image> Utilities::make_solid_colour_image(Color const& colour, int32_t width, int32_t height, Image::Format format) {
	const Ref<Image> result = Image::create(width, height, false, format);
	ERR_FAIL_NULL_V(result, nullptr);
	result->fill(colour);
	return result;
}

Ref<ImageTexture> Utilities::make_solid_colour_texture(Color const& colour, int32_t width, int32_t height, Image::Format format) {
	const Ref<Image> image = make_solid_colour_image(colour, width, height, format);
	ERR_FAIL_NULL_V(image, nullptr);
	const Ref<ImageTexture> result = ImageTexture::create_from_image(image);
	ERR_FAIL_NULL_V(result, nullptr);
	return result;
}

Variant Utilities::get_project_setting(godot::StringName const& p_path, godot::Variant const& p_default_value) {
	if (!ProjectSettings::get_singleton()->has_setting(p_path)) {
		ProjectSettings::get_singleton()->set(p_path, p_default_value);
	}
	Variant result = ProjectSettings::get_singleton()->get_setting_with_override(p_path);

	ProjectSettings::get_singleton()->set_initial_value(p_path, p_default_value);
	return result;
}

godot::String Utilities::get_state_name(godot::Object const& translation_object, State const& state) {
	StateSet const& state_set = state.get_state_set();

	const String region_identifier = Utilities::std_to_godot_string(state_set.get_region().get_identifier());

	String name = translation_object.tr(region_identifier);

	const bool named = name != region_identifier;
	const bool owned = state.get_owner() != nullptr;
	const bool split = state_set.get_state_count() > 1;

	if (!named) {
		// Capital province name
		// TODO - confirm capital is never null?
		name = translation_object.tr(GUINode::format_province_name(Utilities::std_to_godot_string(state.get_capital()->get_identifier())));

		if (!owned) {
			const StringName region_key = OV_SNAME(REGION_NAME);
			static const String name_key = "$NAME$";

			String region = translation_object.tr(region_key);

			if (region != region_key) {
				// CAPITAL Region
				return region.replace(name_key, name);
			}
		}
	}

	if (owned && split) {
		// COUNTRY STATE/CAPITAL
		return get_country_adjective(translation_object, *state.get_owner()) + " " + name;
	}

	// STATE/CAPITAL
	return name;
}
godot::String Utilities::get_country_name(godot::Object const& translation_object, CountryInstance const& country) {
	GovernmentType const* government_type = country.get_government_type_untracked();
	if (government_type != nullptr) {
		const String government_name_key = Utilities::std_to_godot_string(StringUtils::append_string_views(
			country.get_identifier(), "_", government_type->get_identifier()
		));

		String government_name = translation_object.tr(government_name_key);

		if (government_name != government_name_key) {
			return government_name;
		}
	}

	return translation_object.tr(Utilities::std_to_godot_string(country.get_identifier()));
}
godot::String Utilities::get_country_adjective(godot::Object const& translation_object, CountryInstance const& country) {
	static constexpr std::string_view adjective = "_ADJ";

	GovernmentType const* government_type = country.get_government_type_untracked();
	if (government_type != nullptr) {
		const String government_adjective_key = Utilities::std_to_godot_string(StringUtils::append_string_views(
			country.get_identifier(), "_", government_type->get_identifier(), adjective
		));

		String government_adjective = translation_object.tr(government_adjective_key);

		if (government_adjective != government_adjective_key) {
			return government_adjective;
		}
	}

	return translation_object.tr(Utilities::std_to_godot_string(StringUtils::append_string_views(country.get_identifier(), adjective)));
}

godot::String Utilities::make_modifier_effect_value(
	godot::Object const& translation_object,
	ModifierEffect const& format_effect,
	fixed_point_t value,
	bool plus_for_non_negative
) {
	godot::String result;

	if (plus_for_non_negative && value >= 0) {
		result = "+";
	}

	const uint8_t format = static_cast<uint8_t>(format_effect.get_format());

	// Apply multiplier format part
	{
		uint8_t multiplier_power = (format >> ModifierEffect::FORMAT_MULTIPLIER_BIT_OFFSET) &
			((1 << ModifierEffect::FORMAT_MULTIPLIER_BIT_COUNT) - 1);
		while (multiplier_power-- > 0) {
			value *= 10;
		}
	}

	// Apply decimal places format part
	const uint8_t decimal_places = (format >> ModifierEffect::FORMAT_DECIMAL_PLACES_BIT_OFFSET) &
		((1 << ModifierEffect::FORMAT_DECIMAL_PLACES_BIT_COUNT) - 1);

	result += Utilities::fixed_point_to_string_dp(value, decimal_places);

	// Apply suffix format part
	const ModifierEffect::suffix_t suffix = static_cast<ModifierEffect::suffix_t>(
		(format >> ModifierEffect::FORMAT_SUFFIX_BIT_OFFSET) & ((1 << ModifierEffect::FORMAT_SUFFIX_BIT_COUNT) - 1)
	);

	static const String normal_suffix_text = GUILabel::get_colour_marker() + String { "!" };
	static const String special_suffix_text = " " + normal_suffix_text;

	switch (suffix) {
		using enum ModifierEffect::suffix_t;

	case PERCENT: {
		result += "%" + normal_suffix_text;
		break;
	}

	// DAYS AND SPEED MUST BE DEFAULT COLOUR!
	case DAYS: {
		result += special_suffix_text + translation_object.tr(OV_SNAME(DAYS));
		break;
	}

	case SPEED: {
		result += special_suffix_text + translation_object.tr(OV_SNAME(KPH));
		break;
	}

	default: {
		result += normal_suffix_text;
		break;
	}
	}

	return result;
}
godot::String Utilities::make_modifier_effect_value_coloured(
	godot::Object const& translation_object,
	ModifierEffect const& format_effect,
	fixed_point_t value,
	bool plus_for_non_negative
) {
	godot::String result = GUILabel::get_colour_marker();

	const bool is_positive_green = (
		static_cast<uint8_t>(format_effect.get_format()) & (1 << ModifierEffect::FORMAT_POS_NEG_BIT_OFFSET)
	) == static_cast<uint8_t>(ModifierEffect::format_t::FORMAT_PART_POS);

	if (value == 0) {
		result += "Y";
	} else if (is_positive_green == (value > 0)) {
		result += "G";
	} else {
		result += "R";
	}

	result += make_modifier_effect_value(translation_object, format_effect, value, plus_for_non_negative);

	return result;
}

namespace OpenVic::Utilities {
	thread_local memory::vector<godot::Array> _formatting_array_pool_1;
	thread_local memory::vector<godot::Array> _formatting_array_pool_2;
	thread_local memory::vector<godot::Array> _formatting_array_pool_3;
	thread_local memory::vector<godot::Array> _formatting_array_pool_4;
}

fmt::format_context::iterator fmt::formatter<String>::format(String const& str, format_context& ctx) const {
	CharString cs = str.utf8();
	string_view view { cs.get_data(), static_cast<size_t>(cs.length()) };

	return fmt::formatter<string_view>::format(view, ctx);
}

fmt::format_context::iterator fmt::formatter<StringName>::format(StringName const& str, format_context& ctx) const {
	return fmt::formatter<String>::format(static_cast<String>(str), ctx);
}