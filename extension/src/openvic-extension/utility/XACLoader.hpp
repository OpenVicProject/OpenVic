#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>
#include "Utilities.hpp"
#include "XACUtilities.hpp"
//#include "godot_cpp/classes/animation.hpp"
#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/material.hpp"
//#include "godot_cpp/classes/node.hpp"
#include <godot_cpp/classes/node3d.hpp>
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/resource_preloader.hpp"
#include "godot_cpp/classes/shader_material.hpp"
#include "godot_cpp/classes/skeleton3d.hpp"
//#include "godot_cpp/variant/basis.hpp"
//#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_int64_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/packed_vector4_array.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include "openvic-simulation/utility/Logger.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <openvic-extension/singletons/AssetManager.hpp>
#include <openvic-extension/singletons/GameSingleton.hpp>

//#include "openvic-extension/utility/Utilities.hpp"

//using namespace godot;
//using namespace OpenVic;
//using godot::Node3D;

namespace OpenVic {

	//using OpenVic::Utilities::std_view_to_godot_string;

	static constexpr uint32_t XAC_FORMAT_SPECIFIER = ' CAX'; // Order reversed due to litte endian
	static constexpr uint8_t XAC_VERSION_MAJOR = 1, XAC_VERSION_MINOR = 0;

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

	struct node_hierarchy_pack {
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

	//comes as the first 3xint32s when version == 1
	struct material_layer_pack_v1_unk {
		int32_t unknown[3]; //could be a vec3?
	};

	struct material_layer_pack {
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

	enum ATTRIBUTE {
		POSITION,
		NORMAL,
		TANGENT,
		UV,
		COL_32,
		INFLUENCE_RANGE,
		COL_128
	};

	struct vertices_attribute_pack {
		int32_t type; //0-6 (enum ATTRIBUTE)
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

	struct skinning_pack {
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

	struct node_hierarchy { //v1
		node_hierarchy_pack packed;
		std::vector<node_data> node_data;
	};

	struct material_layer {
		material_layer_pack_v1_unk unk; //optional
		material_layer_pack packed;
		String texture;
	};

	struct material_definition {
		material_definition_pack packed;
		String name;
		std::vector<material_layer> layers;
	};

	struct vertices_attribute {
		vertices_attribute_pack packed;
		std::vector<uint32_t> data; // sometimes int32, other times uint32
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

	struct skinning {
		int32_t node_id;
		int32_t local_bones_count; //v3 only
		skinning_pack packed;
		std::vector<influence_data> influence_data;
		std::vector<influence_range> influence_ranges;
	};

	struct node_chunk {
		node_chunk_pack packed;
		String name;
	};

	static bool read_xac_header(Ref<FileAccess> const& file) {
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

	static bool read_xac_metadata(Ref<FileAccess> const& file, xac_metadata_v2& metadata) {
		bool ret = read_struct(file, metadata.packed);
		ret &= read_string(file, metadata.source_app, false);
		ret &= read_string(file, metadata.original_file_name, false);
		ret &= read_string(file, metadata.export_date, false);
		ret &= read_string(file, metadata.actor_name, false);
		return ret;
	}

	static bool read_node_data(Ref<FileAccess> const& file, node_data& node_data) {
		bool ret = read_struct(file, node_data);
		ret &= read_string(file, node_data.name);
		return ret;
	}

	static bool read_node_hierarchy(Ref<FileAccess> const& file, node_hierarchy& hierarchy) {
		bool ret = read_struct(file, hierarchy.packed);
		for(int i=0; i<hierarchy.packed.node_count; i++){
			node_data node;
			ret &= read_node_data(file, node);
			hierarchy.node_data.push_back(node);
		}
		return ret;
	}

	static bool read_material_totals(Ref<FileAccess> const& file, material_totals& totals) {
		return read_struct(file, totals);
	}

	static bool read_layer(Ref<FileAccess> const& file, material_layer& layer, int32_t version) {
		bool ret = true;
		if(version == 1){
			ret &= read_struct(file, layer.unk);
		}
		ret &= read_struct(file, layer.packed);
		ret &= read_string(file, layer.texture, false);
		return ret;
	}

	static bool read_material_definition(Ref<FileAccess> const& file, material_definition& def, int32_t version) {
		bool ret = read_struct(file, def.packed);
		ret &= read_string(file, def.name, false);
		for(int i=0; i<def.packed.layers_count; i++){
			material_layer layer;
			ret &= read_layer(file, layer, version);
			def.layers.push_back(layer);
		}
		return ret;
	}

	static bool read_vertices_attribute(Ref<FileAccess> const& file, vertices_attribute& attribute, int32_t vertices_count) {
		bool ret = read_struct(file, attribute.packed);
		//ret &= read_buffer(file, attribute.data, vertices_count*attribute.packed.attribute_size);
		ret &= read_struct_array(file, attribute.data, vertices_count*attribute.packed.attribute_size/4);
		return ret;
	}

	static bool read_submesh(Ref<FileAccess> const& file, submesh& submesh) {
		bool ret = read_struct(file, submesh.packed);
		ret &= read_struct_array(file, submesh.relative_indices, submesh.packed.indices_count);
		ret &= read_struct_array(file, submesh.bone_ids, submesh.packed.bones_count);
		return ret;
	}

	static bool read_mesh(Ref<FileAccess> const& file, mesh& mesh) {
		bool ret = read_struct(file, mesh.packed);
		for(int i=0; i<mesh.packed.attribute_layers_count; i++){
			vertices_attribute attribute;
			ret &= read_vertices_attribute(file, attribute, mesh.packed.vertices_count);
			mesh.vertices_attributes.push_back(attribute);
		}
		for(int i=0; i<mesh.packed.submeshes_count; i++){
			submesh submesh;
			ret &= read_submesh(file, submesh);
			mesh.submeshes.push_back(submesh);
		}
		return ret;
	}

	static bool read_skinning(Ref<FileAccess> const& file, skinning& skin, std::vector<mesh> const* meshes, int32_t version) {
		bool ret = read_struct(file, skin.node_id);
		if(version == 3) ret &= read_struct(file, skin.local_bones_count);
		ret &= read_struct(file, skin.packed);
		ret &= read_struct_array(file, skin.influence_data, skin.packed.influences_count);
		bool found = false;
		for(mesh mesh : *meshes) {
			if(mesh.packed.is_collision_mesh == skin.packed.is_for_collision && mesh.packed.node_id == skin.node_id){
				ret &= read_struct_array(file, skin.influence_ranges, mesh.packed.influence_ranges_count);
				found = true;
				break;
			}
		}
		ret &= found;
		return ret;
	}

	static bool read_node_chunk(Ref<FileAccess> const& file, node_chunk& node) {
		bool ret = read_struct(file, node.packed);
		ret &= read_string(file, node.name);
		return ret;
	}

	/*
	====================================
	Skeleton helper functions

	====================================
	*/

	//TODO: Verify
	static Transform3D make_transform(vec3d_t position, quat_v1_t quaternion, vec3d_t scale) {
		Transform3D transform = Transform3D();

		Basis basis = Basis();
		basis.set_quaternion(quat_v1_to_godot(quaternion));
		basis.scale(vec3d_to_godot(scale));

		transform.set_basis(basis);
		transform.translate_local(vec3d_to_godot(position, true));

		return transform;
	}

	//TODO: do we want to use node's bone id instead of current_id?
	static Skeleton3D* build_armature_hierarchy(node_hierarchy& hierarchy) {
		static const StringName skeleton_name = "skeleton";
		Skeleton3D* skeleton = memnew(Skeleton3D);
		skeleton->set_name(skeleton_name);

		uint32_t current_id = 0;

		for(node_data const node : hierarchy.node_data) {
			skeleton->add_bone(node.name);
			skeleton->set_bone_parent(current_id, node.packed.parent_node_id);
			
			Transform3D transform = make_transform(node.packed.position, node.packed.rotation, node.packed.scale);
			skeleton->set_bone_rest(current_id, transform);
			skeleton->set_bone_pose(current_id, transform);

			current_id += 1;
		}

		return skeleton;
	}

	static Skeleton3D* build_armature_nodes(std::vector<node_chunk> const* nodes) {
		static const StringName skeleton_name = "skeleton";
		Skeleton3D* skeleton = memnew(Skeleton3D);
		skeleton->set_name(skeleton_name);

		uint32_t current_id = 0;

		for(node_chunk const node : *nodes) {
			skeleton->add_bone(node.name);
			skeleton->set_bone_parent(current_id, node.packed.parent_node_id);
			
			Transform3D transform = make_transform(node.packed.position, node.packed.rotation, node.packed.scale);
			skeleton->set_bone_rest(current_id, transform);
			skeleton->set_bone_pose(current_id, transform);

			current_id += 1;
		}

		return skeleton;
	}

	/*
	====================================
	Material helper functions

	====================================
	*/

	struct material_mapping {
		//-1 means unused
		Ref<Material> godot_material;
		int32_t diffuse_texture_index = -1;
		int32_t specular_texture_index = -1;
		int32_t scroll_index = -1;
	};

	static Error setup_flag_shader() {
		Error result = OK;
		GameSingleton const* game_singleton = GameSingleton::get_singleton();
		ERR_FAIL_NULL_V(game_singleton, {});
		
		static const StringName Param_flag_dimensions = "flag_dims";
		static const StringName Param_flag_texture_sheet = "texture_flag_sheet_diffuse";
		static const Ref<ShaderMaterial> flag_shader = ResourcePreloader().get_resource("res://src/Game/Model/flag_mat.tres");

		flag_shader->set_shader_parameter(Param_flag_dimensions, game_singleton->get_flag_dims());
		flag_shader->set_shader_parameter(Param_flag_texture_sheet, game_singleton->get_flag_sheet_texture());
		return result;
	}

	static std::vector<material_mapping> build_materials(std::vector<material_definition>* materials) {
		
		static const StringName Textures_path = "gfx/anims/%s.dds";

		// Parameters for the default model shader
		static const StringName Param_texture_diffuse = "texture_diffuse";
		//red channel is specular, green and blue are nation colours
		static const StringName Param_texture_nation_colors_mask = "texture_nation_colors_mask";

		// Scrolling textures (smoke, tank tracks)
		static const StringName Param_Scroll_texture_diffuse = "scroll_texture_diffuse";
		static const StringName Param_Scroll_factor = "scroll_factor";
		static Dictionary SCROLLING_MATERIAL_FACTORS;
		SCROLLING_MATERIAL_FACTORS["TexAnim"] = 2.5;
		SCROLLING_MATERIAL_FACTORS["Smoke"] = 0.3;

		static PackedStringArray Scrolling_textures_diffuse;
		static PackedStringArray unit_textures_diffuse;
		static PackedStringArray unit_textures_specular;

		// Flag textures

		static const StringName Param_texture_normal = "texture_normal";

		//General
		static const uint32_t MAX_UNIT_TEXTURES = 32;

		static const StringName Texture_skip_nospec = "nospec";
		static const StringName Texture_skip_flag = "unionjacksquare";
		static const StringName Texture_skip_diff = "test256texture";

		static const Ref<ShaderMaterial> unit_shader = ResourcePreloader().get_resource("res://src/Game/Model/unit_colours_mat.tres");
		static const Ref<ShaderMaterial> scrolling_shader = ResourcePreloader().get_resource("res://src/Game/Model/scrolling_mat.tres");
		static const Ref<ShaderMaterial> flag_shader = ResourcePreloader().get_resource("res://src/Game/Model/flag_mat.tres");
		
		
		std::vector<material_mapping> mappings;

		AssetManager* asset_manager = AssetManager::get_singleton();
		ERR_FAIL_NULL_V(asset_manager, {});

		for(material_definition mat : *materials) {
			String diffuse_name;
			String specular_name;
			String normal_name;

			for(material_layer layer : mat.layers) {
				if(layer.texture == Texture_skip_diff || layer.texture == Texture_skip_flag || layer.texture == Texture_skip_nospec) {
					continue;
				}
				//Get the texture names
				switch(layer.packed.map_type){
					case 2: //diffuse
						if(diffuse_name.is_empty()) diffuse_name = layer.texture;
						else Logger::error("Multiple diffuse layers in material: ", Utilities::godot_to_std_string(diffuse_name), " and ", Utilities::godot_to_std_string(layer.texture));
						break;
					case 3: //specular
						if(specular_name.is_empty()) specular_name = layer.texture;
						else Logger::error("Multiple specular layers in material: ", Utilities::godot_to_std_string(specular_name), " and ", Utilities::godot_to_std_string(layer.texture));
						break;
					case 4: //?
						break;
					case 5: //normal
						if(normal_name.is_empty()) normal_name = layer.texture;
						else Logger::error("Multiple normal layers in material: ", Utilities::godot_to_std_string(normal_name), " and ", Utilities::godot_to_std_string(layer.texture));
						break;
					default:
						Logger::warning("Unknown layer type: ", layer.packed.map_type);
				}
			}

			Ref<ImageTexture> diffuse_texture;
			Ref<ImageTexture> specular_texture;
			Ref<ImageTexture> normal_texture;
			material_mapping mapping;

			if(!diffuse_name.is_empty()) diffuse_texture = asset_manager->get_texture(vformat(Textures_path, diffuse_name));
			if(!specular_name.is_empty()) specular_texture = asset_manager->get_texture(vformat(Textures_path, specular_name));
			if(!normal_name.is_empty()) normal_texture = asset_manager->get_texture(vformat(Textures_path, normal_name));

			//flag
			if(!normal_texture.is_null() && diffuse_texture.is_null()) {
				//There shouldn't be a specular texture
				flag_shader->set_shader_parameter(Param_texture_normal, normal_texture);
				mapping.godot_material = flag_shader;
			}
			//Scrolling texture
			else if(!diffuse_texture.is_null() && SCROLLING_MATERIAL_FACTORS.has(mat.name)) {
				int32_t scroll_textures_index_diffuse = Scrolling_textures_diffuse.find(diffuse_name);
				if(scroll_textures_index_diffuse < 0){
					Scrolling_textures_diffuse.push_back(diffuse_name);
					//err check
					TypedArray<ImageTexture> scroll_diffuse_textures = scrolling_shader->get_shader_parameter(Param_Scroll_texture_diffuse);
					scroll_diffuse_textures.push_back(diffuse_texture);
					scrolling_shader->set_shader_parameter(Param_Scroll_texture_diffuse, scroll_diffuse_textures);

					PackedFloat32Array scroll_factors = scrolling_shader->get_shader_parameter(Param_Scroll_factor);

					scroll_factors.push_back(SCROLLING_MATERIAL_FACTORS[mat.name]);
					scrolling_shader->set_shader_parameter(Param_Scroll_factor, scroll_factors);
				}
				mapping.godot_material = scrolling_shader;
				mapping.scroll_index = scroll_textures_index_diffuse;
				
			}
			//standard material (diffuse optionally with a specular/unit colours)
			else {
				int32_t textures_index_diffuse = unit_textures_diffuse.find(diffuse_name);
				int32_t textures_index_specular = unit_textures_specular.find(specular_name);
				
				if(textures_index_diffuse < 0 && !diffuse_name.is_empty()) {
					unit_textures_diffuse.push_back(diffuse_name);
					if(unit_textures_diffuse.size() >= MAX_UNIT_TEXTURES) {
						Logger::error("Number of diffuse textures exceeded max supported by a shader!");
					}
					TypedArray<ImageTexture> diffuse_textures = unit_shader->get_shader_parameter(Param_texture_diffuse);
					diffuse_textures.push_back(diffuse_texture);
					textures_index_diffuse = diffuse_textures.size() - 1;
					unit_shader->set_shader_parameter(Param_texture_diffuse, diffuse_textures);
				}
				if(textures_index_specular < 0 && !specular_name.is_empty()) {
					unit_textures_specular.push_back(specular_name);
					if(unit_textures_specular.size() >= MAX_UNIT_TEXTURES) {
						Logger::error("Number of diffuse textures exceeded max supported by a shader!");
					}
					TypedArray<ImageTexture> specular_textures = unit_shader->get_shader_parameter(Param_texture_nation_colors_mask);
					specular_textures.push_back(specular_texture);
					textures_index_specular = specular_textures.size() - 1;
					unit_shader->set_shader_parameter(Param_texture_nation_colors_mask, specular_textures);
				}

				mapping.godot_material = unit_shader;
				mapping.diffuse_texture_index = textures_index_diffuse;
				mapping.specular_texture_index = textures_index_specular;
			}

			mappings.push_back(mapping);
		}

		return mappings;
	}

	/*
	====================================
	Mesh helper functions

	====================================
	*/
	//
	
	//TODO: before, we used surface tool to generate the mesh
	// this time around, it might be faster/easier to use the array tools

	//TODO: last time we loaded collision mesh chunks, since we can likely get
	// away with a radius around a unit's position, why not skip them
	//TODO: last time, we hardcoded lists of other chunk names not to load
	// why not try this:

	// if skeleton but mesh chunk has no corresponding skinning chunk:
	//	skip
	// if collision mesh chunk
	//  skip

	// alternatively, see if a mesh name corresponds to a name in the hierarchy
	//  if yes, keep the mesh

	//this should get rid of mesh chunks that aren't attached to anything

	static MeshInstance3D* build_mesh(mesh& mesh_chunk, skinning* skin, std::vector<material_mapping>& materials) {
		static const uint32_t EXTRA_CULL_MARGIN = 2;

		MeshInstance3D* mesh_inst = memnew(MeshInstance3D);
		mesh_inst->set_extra_cull_margin(EXTRA_CULL_MARGIN);

		Ref<ArrayMesh> mesh = Ref<ArrayMesh>();
		mesh.instantiate();

		std::vector<vec3d_t>* verts;
		std::vector<vec3d_t>* normals;
		std::vector<vec4d_t>* tangents;
		int uvs_read = 0;
		std::vector<vec2d_t>* uv1;
		std::vector<vec2d_t>* uv2;
		std::vector<uint32_t>* influence_range_indices;

		for(vertices_attribute attribute : mesh_chunk.vertices_attributes) {
			switch(attribute.packed.type){
				case ATTRIBUTE::POSITION:
					verts = (std::vector<vec3d_t>*)attribute.data.data();
					break;
				case ATTRIBUTE::NORMAL:
					normals = (std::vector<vec3d_t>*)attribute.data.data();
					break;
				case ATTRIBUTE::TANGENT:
					tangents = (std::vector<vec4d_t>*)attribute.data.data();
					break;
				case ATTRIBUTE::UV:
					if(uvs_read == 0) uv1 = (std::vector<vec2d_t>*)&attribute.data;
					else if(uvs_read == 1) uv2 = (std::vector<vec2d_t>*)&attribute.data;
					uvs_read += 1;
					break;
				case ATTRIBUTE::INFLUENCE_RANGE:
					if(skin == nullptr){
						Logger::warning("Mesh chunk has influence attribute but no corresponding skinning chunk");
						break;
					}
					influence_range_indices = (std::vector<uint32_t>*)&attribute.data;
					break;
				default: //for now, ignore color data
					break;
			}
		}

		uint32_t vert_total = 0;
		static const StringName key_diffuse = "tex_index_diffuse";
		static const StringName key_specular = "tex_index_specular";
		static const StringName key_scroll = "scroll_tex_index_diffuse";

		//for now we treat a submesh as a godot mesh surface
		for(submesh submesh : mesh_chunk.submeshes) {
			Array arrs; //surface vertex data arrays
			PackedVector3Array verts_submesh = {};
			PackedVector3Array normals_submesh = {};
			PackedVector4Array tangents_submesh = {};
			PackedVector2Array uv1_submesh = {};
			PackedVector2Array uv2_submesh = {};
			PackedInt32Array bone_ids = {};
			PackedFloat32Array weights = {};
			//godot uses a fixed 4 bones influencing a vertex, so size the array accordingly
			bone_ids.resize(submesh.relative_indices.size()*4);
			bone_ids.fill(0);

			//1 vertex is in the surface per relative index
			for(uint32_t rel_ind : submesh.relative_indices) {
				uint32_t index = rel_ind + vert_total;
				
				if((*verts).size() > index) verts_submesh.push_back(vec3d_to_godot((*verts)[index], true));//verts[index]);
				if((*normals).size() > index) normals_submesh.push_back( vec3d_to_godot((*normals)[index]));
				
				//TODO: verify if we need to invert the x for each tangent
				if((*tangents).size() > index) {
					tangents_submesh.push_back(vec4d_to_godot((*tangents)[index]));
					tangents_submesh[tangents_submesh.size()-1].x *= -1;
				}
				
				if((*uv1).size() > index) uv1_submesh.push_back(vec2d_to_godot((*uv1)[index]));
				if((*uv2).size() > index) uv2_submesh.push_back(vec2d_to_godot((*uv2)[index]));

				uint32_t influence_range_ind = (*influence_range_indices)[index];
				uint32_t influences_ind = skin->influence_ranges[influence_range_ind].first_influence_index;
				uint32_t influences_count = Math::min(skin->influence_ranges[influence_range_ind].influences_count, 4);

				for(int i = 0; i < influences_count; i++) {
					bone_ids[rel_ind + i] = skin->influence_data[influences_ind + i].bone_id;
					weights[rel_ind + i] = skin->influence_data[influences_ind + i].weight;
				}

			}

			arrs[Mesh::ARRAY_VERTEX] = verts_submesh;
			arrs[Mesh::ARRAY_NORMAL] = normals_submesh;
			arrs[Mesh::ARRAY_TANGENT] = tangents_submesh;
			arrs[Mesh::ARRAY_TEX_UV] = uv1_submesh;
			arrs[Mesh::ARRAY_TEX_UV2] = uv2_submesh; //can enable this later if needed
			arrs[Mesh::ARRAY_BONES] = bone_ids;
			arrs[Mesh::ARRAY_WEIGHTS] = weights;

			mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrs);
			
			//setup materials for the surface
			material_mapping material_mapping = materials[submesh.packed.material_id];
			mesh->surface_set_material(mesh->get_surface_count()-1, material_mapping.godot_material);
			if(material_mapping.diffuse_texture_index != -1) {
				mesh_inst->set_instance_shader_parameter(key_diffuse, material_mapping.diffuse_texture_index);
			}
			if(material_mapping.specular_texture_index != -1) {
				mesh_inst->set_instance_shader_parameter(key_specular, material_mapping.diffuse_texture_index);
			}
			if(material_mapping.scroll_index != -1) {
				mesh_inst->set_instance_shader_parameter(key_scroll, material_mapping.diffuse_texture_index);
			}

			vert_total += submesh.relative_indices.size();
		}

		mesh_inst->set_mesh(mesh);

		return mesh_inst;
	} 



	/*
	====================================
	Main loading function

	====================================
	*/
	static Node3D* _load_xac_model(Ref<FileAccess> const& file) {
		bool res = read_xac_header(file);
		
		Node3D* base = memnew(Node3D);

		if(!res) return base;

		xac_metadata_v2 metadata;
		
		bool hierarchy_read = false;
		node_hierarchy hierarchy;
		std::vector<node_chunk> nodes; //other way of making a bone hierarchy
		
		material_totals material_totals;
		std::vector<material_definition> materials;

		std::vector<mesh> meshes;
		std::vector<skinning> skinnings;

		Logger::info("Read...", Utilities::godot_to_std_string(file->get_path()));
		//0xA unknown
		//0x4 unknown
		//0x6 unknown
		//0x8 junk data (ex. old versions of the file duplicated in append mode)

		while(!file->eof_reached()){
			chunk_header_t header;
			if(!read_chunk_header(file, header)) break;

			if(header.type == 0x7 && header.version == 2) read_xac_metadata(file, metadata);
			
			else if(header.type == 0xB && header.version == 1) {
				hierarchy_read = read_node_hierarchy(file, hierarchy);
			}
			else if(header.type == 0x0 && header.version == 1) {
				node_chunk node;
				read_node_chunk(file, node);
				nodes.push_back(node);
			}
			else if(header.type == 0xD && header.version == 1) {
				read_material_totals(file, material_totals);
			}
			else if(header.type == 0x3) {
				material_definition mat;
				read_material_definition(file, mat, header.version);
				materials.push_back(mat);
			}
			else if(header.type == 0x1 && header.version == 1) {
				mesh mesh;
				read_mesh(file, mesh);
				meshes.push_back(mesh);
			}
			else if(header.type == 0x2) {
				skinning skin;
				read_skinning(file, skin, &meshes, header.version);
				skinnings.push_back(skin);
			}
			else {
				UtilityFunctions::print(
				vformat("Unsupported XAC chunk: type = %x, length = %x, version = %d", header.type, header.length, header.version)
				);
				// Skip unsupported chunks by using get_buffer, make sure this doesn't break anything
				if(header.length + file->get_position() < file->get_length()) file->get_buffer(header.length);
				else {
					res = false;
					break;
				} 
			}
		}

		Logger::info("Setup skeleton...");

		//Setup skeleton
		Skeleton3D* skeleton;
		if(hierarchy_read) {
			skeleton = build_armature_hierarchy(hierarchy);
			base->add_child(skeleton);
		}
		else if(!nodes.empty()) {
			skeleton = build_armature_nodes(&nodes);
			base->add_child(skeleton);
		}

		Logger::info("Setup Skeleton...");

		//Setup materials
		std::vector<material_mapping> material_mappings = build_materials(&materials);
		
		Logger::info("Setup Mesh...");

		//setup mesh
		for(mesh mesh : meshes) {
			if(mesh.packed.is_collision_mesh) continue; //we'll use our own collision meshes where needed

			skinning* mesh_skin = nullptr;
			for(skinning skin : skinnings){
				if(skin.node_id == mesh.packed.node_id && skin.packed.is_for_collision == mesh.packed.is_collision_mesh){
					mesh_skin = &skin;
					break;
				}
			}

			if(skeleton && mesh_skin == nullptr) continue;

			MeshInstance3D* mesh_inst = build_mesh(mesh, mesh_skin, material_mappings);
			base->add_child(mesh_inst);
			mesh_inst->set_owner(base);
			
			if(skeleton) {
				mesh_inst->set_skeleton_path(mesh_inst->get_path_to(skeleton));
				int32_t node_id = mesh.packed.node_id;
				if(hierarchy_read && node_id < hierarchy.node_data.size()) {
					mesh_inst->set_name(hierarchy.node_data[node_id].name);
				}
				else if(!nodes.empty() && node_id < nodes.size()) {
					mesh_inst->set_name(nodes[node_id].name);
				}
			} 
		}

		return base;
	}
}