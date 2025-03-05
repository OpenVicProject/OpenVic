#pragma once
#include <cassert>
#include <cstdint>
#include <vector>

#include "XACUtilities.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/node3d.hpp>

#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/skeleton3d.hpp"
#include <openvic-extension/singletons/ModelSingleton.hpp>
#include <openvic-extension/singletons/AssetManager.hpp>
#include <openvic-extension/singletons/GameSingleton.hpp>


namespace OpenVic {

	//needed for material loading functions, which use modelSingleton to track state between calls
	struct ModelSingleton;
	
	/*enum struct MAP_TYPE {
		DIFFUSE = 2,
		SPECULAR,
		SHADOW,
		NORMAL
	};*/
	/*struct MAP_TYPE {
		enum Values : int32_t {
			DIFFUSE = 2,
			SPECULAR,
			SHADOW,
			NORMAL
		};
	};*/

	class XacLoader {
		static constexpr uint32_t XAC_FORMAT_SPECIFIER = ' CAX'; // Order reversed due to little endian
		static constexpr uint8_t XAC_VERSION_MAJOR = 1, XAC_VERSION_MINOR = 0;

		//TODO: How do we get this enum to work both here and in modelSingleton?
		//public:
		enum class MAP_TYPE {
			DIFFUSE = 2,
			SPECULAR,
			SHADOW,
			NORMAL
		};
		/*struct MAP_TYPE {
			enum Values : int32_t {
				DIFFUSE = 2,
				SPECULAR,
				SHADOW,
				NORMAL
			};
		};
		private:*/

		#pragma pack(push)
		#pragma pack(1)

		struct xac_header_t {
			uint32_t format_identifier;
			uint8_t version_major;
			uint8_t version_minor;
			uint8_t big_endian;
			uint8_t multiply_order;
		};

		struct xac_metadata_v2_pack_t {
			uint32_t reposition_mask; //1=position, 2=rotation, 4=scale
			int32_t repositioning_node;
			uint8_t exporter_major_version;
			uint8_t exporter_minor_version;
			uint16_t pad;
			float retarget_root_offset;
		};

		struct node_hierarchy_pack_t { //v1
			int32_t node_count;
			int32_t root_node_count; //nodes with parent_id == -1
		};

		struct node_data_pack_t { //v1
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

		struct material_totals_t { //v1
			int32_t total_materials_count;
			int32_t standard_materials_count;
			int32_t fx_materials_count;
		};

		struct material_definition_pack_t {
			vec4d_t ambient_color;
			vec4d_t diffuse_color;
			vec4d_t specular_color;
			vec4d_t emissive_color;
			float shine;
			float shine_strength;
			float opacity;
			float ior; 				//index of refraction
			uint8_t double_sided; 	//bool
			uint8_t wireframe; 		//bool
			uint8_t unused;			//in v1, unknown, but used
			uint8_t layers_count; 	//in v1, unknown, but used
		};

		struct material_layer_pack_t { //also chunk 0x4, v2
			float amount;
			vec2d_t uv_offset;
			vec2d_t uv_tiling;
			float rotation_in_radians;
			int16_t material_id;
			uint8_t map_type; // (1-5) enum MAP_TYPE
			uint8_t unused;
		};

		struct mesh_pack_t {
			int32_t node_id;
			int32_t influence_ranges_count;
			int32_t vertices_count;
			int32_t indices_count;
			int32_t submeshes_count;
			int32_t attribute_layers_count;
			uint8_t is_collision_mesh; //bool
			uint8_t pad[3];
		};

		struct ATTRIBUTE {
			enum Values : int32_t {
				POSITION,
				NORMAL,
				TANGENT,
				UV,
				COL_32,
				INFLUENCE_RANGE,
				COL_128
			};
		};

		struct vertices_attribute_pack_t {
			int32_t type; //0-6 (enum ATTRIBUTE)
			int32_t attribute_size;
			uint8_t keep_originals; //bool
			uint8_t is_scale_factor; //bool
			uint16_t pad;
		};

		struct submesh_pack_t {
			int32_t indices_count;
			int32_t vertices_count;
			int32_t material_id;
			int32_t bones_count;
		};

		struct skinning_pack_t {
			int32_t influences_count;
			uint8_t is_for_collision; //bool
			uint8_t pad[3];
		};

		struct influence_data_t {
			float weight;
			int16_t bone_id;
			int16_t pad;
		};

		struct influence_range_t {
			int32_t first_influence_index;
			int32_t influences_count;
		};

		// 0x4, 0x6, 0xA chunk types appear in vic2, but what they do
		// is unknown
		// 0x8 is junk data
		// 0x0 is the older node/bone chunk

		//0x0, v3
		struct node_chunk_pack_t {
			quat_v1_t rotation;
			quat_v1_t scale_rotation;
			vec3d_t position;
			vec3d_t scale;
			vec3d_t unused; //-1, -1, -1
			int32_t unknown; //-1 (0xFFFF)
			int32_t parent_node_id;
			float uncertain[17]; //likely a matrix44 + fimportancefactor
		};

		/*
		0x6, v? unknown
			12x int32?
			09x float?
		*/

		#pragma pack(pop)

		struct xac_metadata_v2_t {
			xac_metadata_v2_pack_t packed = {};
			godot::String source_app;
			godot::String original_file_name;
			godot::String export_date;
			godot::String actor_name;
		};

		struct node_data_t { //v1
			node_data_pack_t packed = {};
			godot::String name;
		};

		struct node_hierarchy_t { //v1
			node_hierarchy_pack_t packed = {};
			std::vector<node_data_t> node_data;
		};

		struct material_layer_t {
			material_layer_pack_t packed = {};
			godot::String texture;
		};

		struct material_definition_t {
			material_definition_pack_t packed = {};
			godot::String name;
			std::vector<material_layer_t> layers;
		};

		struct vertices_attribute_t {
			vertices_attribute_pack_t packed = {};
			std::vector<uint8_t> data;
		};

		struct submesh_t {
			submesh_pack_t packed = {};
			std::vector<int32_t> relative_indices;
			std::vector<int32_t> bone_ids;
		};

		struct mesh_t {
			mesh_pack_t packed = {};
			std::vector<vertices_attribute_t> vertices_attributes;
			std::vector<submesh_t> submeshes;
		};

		struct skinning_t {
			int32_t node_id = 0;
			int32_t local_bones_count = -1; //v3 only
			skinning_pack_t packed = {};
			std::vector<influence_data_t> influence_data;
			std::vector<influence_range_t> influence_ranges;
		};

		struct node_chunk_t {
			node_chunk_pack_t packed = {};
			godot::String name;
		};

		//Dataloading functions
		bool read_xac_header(godot::Ref<godot::FileAccess> const& file);
		bool read_xac_metadata(godot::Ref<godot::FileAccess> const& file, xac_metadata_v2_t& metadata);
		bool read_node_data(godot::Ref<godot::FileAccess> const& file, node_data_t& node_data);
		bool read_node_hierarchy(godot::Ref<godot::FileAccess> const& file, node_hierarchy_t& hierarchy);
		bool read_material_totals(godot::Ref<godot::FileAccess> const& file, material_totals_t& totals);
		bool read_layer(godot::Ref<godot::FileAccess> const& file, material_layer_t& layer);
		bool read_material_definition(godot::Ref<godot::FileAccess> const& file, material_definition_t& def, int32_t version);
		bool read_vertices_attribute(godot::Ref<godot::FileAccess> const& file, vertices_attribute_t& attribute, int32_t vertices_count);
		bool read_submesh(godot::Ref<godot::FileAccess> const& file, submesh_t& submesh);
		bool read_mesh(godot::Ref<godot::FileAccess> const& file, mesh_t& mesh);
		bool read_skinning(godot::Ref<godot::FileAccess> const& file, skinning_t& skin, std::vector<mesh_t> const& meshes, int32_t version);
		bool read_node_chunk(godot::Ref<godot::FileAccess> const& file, node_chunk_t& node);

		//Xac -> godot conversion functions
		struct material_mapping {
			//-1 means unused
			godot::Ref<godot::Material> godot_material;
			int32_t diffuse_texture_index = -1;
			int32_t specular_texture_index = -1;
			//int32_t shadow_texture_index = -1;
			int32_t scroll_index = -1;
		};

		struct model_texture_set {
			godot::String diffuse_name;
			godot::String specular_name;
			godot::String normal_name;
			//godot::String shadow_name;
		};

		godot::Skeleton3D* build_armature_hierarchy(node_hierarchy_t const& hierarchy);
		godot::Skeleton3D* build_armature_nodes(std::vector<node_chunk_t> const& nodes);

		model_texture_set get_model_textures(material_definition_t const& material);
		std::vector<material_mapping> build_materials(std::vector<material_definition_t> const& materials);
		godot::MeshInstance3D* build_mesh(mesh_t const& mesh_chunk, skinning_t* skin, std::vector<material_mapping> const& materials);

	public:
		//godot::Ref<godot::ImageTexture> get_texture(godot::String name);
		static godot::Ref<godot::ImageTexture> get_model_texture(godot::String name) {
			/*godot::UtilityFunctions::print(
				godot::vformat("texture: %s", name)
			);*/
			AssetManager* asset_manager = AssetManager::get_singleton();
			//ERR_FAIL_NULL_V(asset_manager, {});
			static const godot::StringName Textures_path = "gfx/anims/%s.dds";
			return asset_manager->get_texture(godot::vformat(Textures_path, name));
		}
		godot::Node3D* _load_xac_model(godot::Ref<godot::FileAccess> const& file);
	};
}