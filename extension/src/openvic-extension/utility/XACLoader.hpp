/*#include <cstdint>
#include <vector>
#include "XACUtilities.hpp"
//#include "godot_cpp/classes/animation.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/packed_vector4_array.hpp"
//#include "openvic-simulation/utility/Logger.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

//#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

//using OpenVic::Utilities::std_view_to_godot_string;

static constexpr uint32_t XSM_FORMAT_SPECIFIER = ' CAX'; /* Order reversed due to litte endian */
/*static constexpr uint8_t XSM_VERSION_MAJOR = 1, XSM_VERSION_MINOR = 0;

#pragma pack(push)
#pragma pack(1)

struct xac_header_t {
	uint32_t format_identifier;
	uint8_t version_major;
	uint8_t version_minor;
	uint8_t big_endian;
	uint8_t multiply_order;
};

struct xac_metadata_v2_pack {
	uint32_t reposition_mask; //1=position, 2=rotation, 4=scale
	int32_t repositioning_node;
	uint8_t exporter_major_version;
	uint8_t exporter_minor_version;
	uint16_t pad;
	float retarget_root_offset;
};

struct node_hierarchy_v1_pack {
	int32_t node_count;
	int32_t root_node_count; //nodes with parent_id == -1
};

struct node_data_pack { //v1
	quat_v1_t rotation;
	quat_v1_t scale_rotation;
	vec3d_t position;
	vec3d_t scale;
	float unused[3];
	int32_t unknown[2];
	int32_t parent_node_id;
	int32_t child_nodes_count;
	int32_t include_in_bounds_calculation; //bool
	matrix44_t transform;
	float importance_factor;
};

struct material_totals { //v1
	int32_t total_materials_count;
	int32_t standard_materials_count;
	int32_t fx_materials_count;
};

struct material_definition_pack {
	vec4d_t ambient_color;
	vec4d_t diffuse_color;
	vec4d_t specular_color;
	vec4d_t emissive_color;
	float shine;
	float shine_strength;
	float opacity;
	float ior; //index of refraction
	uint8_t double_sided; //bool
	uint8_t wireframe; //bool
	uint8_t unused;
	uint8_t layers_count;
};

struct material_layer_pack_v2 {
	float amount;
	vec2d_t uv_offset;
	vec2d_t uv_tiling;
	float rotation_in_radians;
	int16_t material_id;
	uint8_t map_type;
	uint8_t unused;
};

struct material_layer_pack_v1 {
	int32_t unknown[3]; //could be a vec3?
	float amount;
	vec2d_t uv_offset;
	vec2d_t uv_tiling;
	float rotation_in_radians;
	int16_t material_id;
	uint8_t map_type;
	uint8_t unused;
};

struct mesh_pack {
	int32_t node_id;
	int32_t influence_ranges_count;
	int32_t vertices_count;
	int32_t indices_count;
	int32_t submeshes_count;
	int32_t attribute_layers_count;
	uint8_t is_collision_mesh; //bool
	uint8_t pad[3];
};

struct vertices_attribute_pack {
	int32_t type; //0-6
	int32_t attribute_size;
	uint8_t keep_originals; //bool
	uint8_t is_scale_factor; //bool
	uint16_t pad;
};

struct submesh_pack {
	int32_t indices_count;
	int32_t vertices_count;
	int32_t material_id;
	int32_t bones_count;
};

struct skinning_v3_pack {
	int32_t node_id;
	int32_t local_bones_count;
	int32_t influences_count;
	uint8_t is_for_collision; //bool
	uint8_t pad[3];
};

struct skinning_v2_pack {
	int32_t node_id;
	int32_t influences_count;
	uint8_t is_for_collision; //bool
	uint8_t pad[3];
};

struct influence_data {
	float weight;
	int16_t bone_id;
	int16_t pad;
};

struct influence_range {
	int32_t first_influence_index;
	int32_t influences_count;
};

// 0x4, 0x6, 0xA chunk types appear in vic2, but what they do
// is unknown
// 0x8 is junk data
// 0x0 is the older node/bone chunk

//0x0
struct node_chunk_pack {
	quat_v1_t rotation;
	quat_v1_t scale_rotation;
	vec3d_t position;
	vec3d_t scale;
	vec3d_t unused;
	int32_t unknown; //uncertain
	int32_t parent_node_id;
	matrix44_t possible_matrix; //uncertain
	float possible_importance_factor; //uncertain
};

#pragma pack(pop)

struct xac_metadata_v2 {
	xac_metadata_v2_pack packed;
	String source_app;
	String original_file_name;
	String export_date;
	String actor_name;
};

struct node_data { //v1
	node_data_pack packed;
	String name;
};

struct material_layer_v2 {
	material_layer_pack_v2 packed;
	String texture;
};

struct material_definition_v2 {
	material_definition_pack packed;
	String name;
	std::vector<material_layer_v2> layers;
};

struct material_layer_v1 {
	material_layer_pack_v1 packed;
	String texture;
};

struct material_definition_v1 {
	material_definition_pack packed;
	String name;
	std::vector<material_layer_v1> layers;
};

struct vertices_attribute {
	vertices_attribute_pack packed;
	PackedByteArray data;
};

struct submesh {
	submesh_pack packed;
	std::vector<int32_t> relative_indices;
	std::vector<int32_t> bone_ids;
};

struct mesh {
	mesh_pack packed;
	std::vector<vertices_attribute> vertices_attributes;
	std::vector<submesh> submeshes;
};

struct skinning_v3 {
	skinning_v3_pack packed;
	std::vector<influence_data> influence_data;
	std::vector<influence_range> influence_ranges;
};

struct skinning_v2 {
	skinning_v2_pack packed;
	std::vector<influence_data> influence_data;
	std::vector<influence_range> influence_ranges;
};

struct node_chunk {
	node_chunk_pack packed;
	String name;
};


/*
static bool read_header(Ref<FileAccess> const& file) {
	xac_header_t header;
	ERR_FAIL_COND_V(!read_struct(file, header), false);

	ERR_FAIL_COND_V_MSG(
		header.format_identifier != XAC_FORMAT_SPECIFIER, false, vformat(
			"Invalid XAC format identifier: %x (should be %x)", header.format_identifier, XAC_FORMAT_SPECIFIER
		)
	);

	ERR_FAIL_COND_V_MSG(
		header.version_major != XAC_VERSION_MAJOR || header.version_minor != XAC_VERSION_MINOR, false, vformat(
			"Invalid XAC version: %d.%d (should be %d.%d)",
			header.version_major, header.version_minor, XAC_VERSION_MAJOR, XAC_VERSION_MINOR
		)
	);

	ERR_FAIL_COND_V_MSG(
		header.big_endian != 0, false, "Invalid XAC endianness: big endian (only little endian is supported)"
	);

	ERR_FAIL_COND_V_MSG(
		header.multiply_order != 0, false, "Invalid XAC multiply order: ???"
	);

	return true;
}

Skeleton3D* ModelLoader::load_xac_model(godot::String const& xac_path) {
	ERR_FAIL_COND_V(xac_path.is_empty(), nullptr);

	const Ref<FileAccess> file = FileAccess::open(xac_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	ERR_FAIL_COND_V_MSG(
		err != OK  || file.is_null(), nullptr, vformat("Failed to open XAC model file: %s", xac_path)
	);

	ERR_FAIL_COND_V_MSG(
		!read_header(file), nullptr, vformat("Failed to read and validate XAC header for model file: %s", xac_path)
	);

	Skeleton3D* skeleton = memnew(Skeleton3D);
	ERR_FAIL_NULL_V(skeleton, nullptr);

	while (!file->eof_reached()) {
		ERR_FAIL_COND_V(!read_chunk(file), nullptr);
	}

	// build skeleton and child nodes...

	return skeleton;
}
*/