#include "OVLexyVDF.hpp"

#include <span>
#include <string_view>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

#include "openvic-extension/core/Convert.hpp"

#include <lexy-vdf/gen/author_info.gen.hpp>
#include <lexy-vdf/gen/commit_info.gen.hpp>
#include <lexy-vdf/gen/license_info.gen.hpp>

using namespace OpenVic;
using namespace godot;

inline static PackedStringArray _generate_author_array(std::span<const std::string_view> span) {
	PackedStringArray result;
	result.resize(span.size());
	for (size_t index = 0; std::string_view author : span) {
		result[index] = convert_to<String>(author);
		++index;
	}
	return result;
}

TypedDictionary<String, PackedStringArray> OVLexyVDF::get_author_info() {
	TypedDictionary<String, PackedStringArray> result;
	result["senior_developers"] = _generate_author_array(LVDF_AUTHORS_SENIOR_DEVELOPERS);
	result["consultants"] = _generate_author_array(LVDF_AUTHORS_CONSULTANTS);
	return result;
}

inline static Dictionary _generate_copyright_dictionary(LvdfComponentCopyright const& component) {
	Dictionary result;
	result["name"] = convert_to<String>(component.name);
	TypedArray<Dictionary> parts;
	parts.resize(component.parts.size());
	for (size_t index = 0; LvdfComponentCopyrightPart const& part : component.parts) {
		Dictionary part_result;
		part_result["files"] = convert_to<PackedStringArray>(part.files);
		part_result["copyright"] = convert_to<PackedStringArray>(part.copyright_statements);
		part_result["license"] = convert_to<String>(part.license);
		parts[index] = part_result;
		++index;
	}
	result["parts"] = parts;
	return result;
}

TypedArray<Dictionary> OVLexyVDF::get_copyright_info() {
	TypedArray<Dictionary> result;
	result.resize(LVDF_COPYRIGHT_INFO.size());
	for (size_t index = 0; LvdfComponentCopyright const& component : LVDF_COPYRIGHT_INFO) {
		result[index] = _generate_copyright_dictionary(component);
		++index;
	}
	return result;
}

TypedDictionary<String, String> OVLexyVDF::get_license_info() {
	TypedDictionary<String, String> result;
	for (LvdfLicense const& license : LVDF_LICENSES) {
		result[convert_to<String>(license.license_name)] = convert_to<String>(license.license_body);
	}
	return result;
}

String OVLexyVDF::get_license_text() {
	return convert_to<String>(LVDF_LICENSE_TEXT);
}

Dictionary OVLexyVDF::get_version_info() {
	Dictionary result;
	String tag = convert_to<String>(LVDF_TAG);

	int64_t major, minor, patch, prerelease_count;
	String prerelease_type, prerelease_hash;
	if (tag.begins_with("v") && !tag.contains("_")) {
		PackedStringArray split = tag.erase(0).split("-");

		PackedStringArray core = split[0].split(".");
		major = core[0].to_int();
		minor = core[1].to_int();
		if (core.size() == 2) {
			patch = 0;
		} else {
			patch = core[2].to_int();
		}

		if (split.size() > 1) {
			split = split[1].split("+");
			if (split.size() >= 1) {
				PackedStringArray prerelease;
				if (split[0].begins_with("snapshot.dev")) {
					prerelease = split[0].erase(0, sizeof("snapshot.dev")).split(".");
					prerelease.insert(0, "snapshot.dev");
				} else {
					prerelease = split[0].split(".");
				}
				prerelease_type = prerelease[0];
				prerelease_count = prerelease[1].to_int();
				if (split.size() > 1) {
					prerelease_hash = split[1];
				} else {
					prerelease_hash = "";
				}
			} else {
				prerelease_type = "";
				prerelease_count = 0;
				prerelease_hash = "";
			}
		}
	} else {
		major = 0;
		minor = 2;
		patch = 0;
		prerelease_type = "unknown";
		prerelease_count = 0;
		prerelease_hash = "unknown";
	}

	result["tag"] = tag;
	result["major"] = major;
	result["minor"] = minor;
	result["patch"] = patch;
	result["prerelease_type"] = prerelease_type;
	result["prerelease_count"] = prerelease_count;
	result["prerelease_hash"] = prerelease_hash;
	result["hex"] = 0x10000 * major + 0x100 * minor + patch;
	result["release"] = convert_to<String>(LVDF_RELEASE);
	result["hash"] = convert_to<String>(LVDF_COMMIT_HASH);
	result["timestamp"] = LVDF_COMMIT_TIMESTAMP;

	return result;
}
