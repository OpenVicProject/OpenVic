#include "XACLoader.hpp"
#include <cstdint>
#include <vector>

#include "XACUtilities.hpp"
#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/resource_preloader.hpp"
#include "godot_cpp/classes/script.hpp"
#include "godot_cpp/classes/shader.hpp"
#include "godot_cpp/classes/shader_material.hpp"
#include "godot_cpp/classes/skeleton3d.hpp"
#include "godot_cpp/classes/surface_tool.hpp"

#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_int64_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/packed_vector4_array.hpp"
#include "godot_cpp/variant/plane.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/variant.hpp"

#include "openvic-simulation/utility/Logger.hpp"

using namespace::OpenVic;
using namespace::godot;

bool XacLoader::_read_xac_header(Ref<FileAccess> const& file) {
	xac_header_t header = {};
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

	/*ERR_FAIL_COND_V_MSG(
		header.multiply_order != 0, false, "Invalid XAC multiply order: ???"
	);*/

	return true;
}

bool XacLoader::_read_xac_metadata(Ref<FileAccess> const& file, xac_metadata_v2_t& metadata) {
	bool ret = read_struct(file, metadata.packed);
	ret &= read_string(file, metadata.source_app, false);
	ret &= read_string(file, metadata.original_file_name, false);
	ret &= read_string(file, metadata.export_date, false);
	ret &= read_string(file, metadata.actor_name, true);
	return ret;
}

bool XacLoader::_read_node_data(Ref<FileAccess> const& file, node_data_t& node_data) {
	bool ret = read_struct(file, node_data.packed);
	ret &= read_string(file, node_data.name);
	return ret;
}

bool XacLoader::_read_node_hierarchy(Ref<FileAccess> const& file, node_hierarchy_t& hierarchy) {
	bool ret = read_struct(file, hierarchy.packed);
	for(int32_t i=0; i<hierarchy.packed.node_count; i++) {
		node_data_t node;
		ret &= _read_node_data(file, node);
		hierarchy.node_data.push_back(node);
	}
	return ret;
}

bool XacLoader::_read_material_totals(Ref<FileAccess> const& file, material_totals_t& totals) {
	return read_struct(file, totals);
}

bool XacLoader::_read_layer(Ref<FileAccess> const& file, material_layer_t& layer) {
	bool ret = true;
	ret &= read_struct(file, layer.packed);
	ret &= read_string(file, layer.texture, false);
	return ret;
}

bool XacLoader::_read_material_definition(Ref<FileAccess> const& file, material_definition_t& def, int32_t version) {
	bool ret = read_struct(file, def.packed);
	ret &= read_string(file, def.name, false);
	if (version > 1) { //in version 1, the layers are defined in separate chunks of type 0x4
		for(uint32_t i=0; i<def.packed.layers_count; i++) {
			material_layer_t layer;
			ret &= _read_layer(file, layer);
			def.layers.push_back(layer);
		}
	}
	return ret;
}

bool XacLoader::_read_vertices_attribute(Ref<FileAccess> const& file, vertices_attribute_t& attribute, int32_t vertices_count) {
	bool ret = read_struct(file, attribute.packed);
	ret &= read_struct_array(file, attribute.data, vertices_count*attribute.packed.attribute_size);
	return ret;
}

bool XacLoader::_read_submesh(Ref<FileAccess> const& file, submesh_t& submesh) {
	bool ret = read_struct(file, submesh.packed);
	ret &= read_struct_array(file, submesh.relative_indices, submesh.packed.indices_count);
	ret &= read_struct_array(file, submesh.bone_ids, submesh.packed.bones_count);
	return ret;
}

bool XacLoader::_read_mesh(Ref<FileAccess> const& file, mesh_t& mesh) {
	bool ret = read_struct(file, mesh.packed);
	for(uint32_t i=0; i<mesh.packed.attribute_layers_count; i++) {
		vertices_attribute_t attribute;
		ret &= _read_vertices_attribute(file, attribute, mesh.packed.vertices_count);
		mesh.vertices_attributes.push_back(attribute);
	}
	for(uint32_t i=0; i<mesh.packed.submeshes_count; i++) {
		submesh_t submesh;
		ret &= _read_submesh(file, submesh);
		mesh.submeshes.push_back(submesh);
	}
	return ret;
}

bool XacLoader::_read_skinning(Ref<FileAccess> const& file, skinning_t& skin, std::vector<mesh_t> const& meshes, int32_t version) {
	bool ret = read_struct(file, skin.node_id);
	if (version == 3) {
		ret &= read_struct(file, skin.local_bones_count);
	}
	ret &= read_struct(file, skin.packed);
	ret &= read_struct_array(file, skin.influence_data, skin.packed.influences_count);
	bool found = false;
	for(mesh_t const& mesh : meshes) {
		if (mesh.packed.is_collision_mesh == skin.packed.is_for_collision && mesh.packed.node_id == skin.node_id) {
			ret &= read_struct_array(file, skin.influence_ranges, mesh.packed.influence_ranges_count);
			found = true;
			break;
		}
	}
	ret &= found;
	return ret;
}

bool XacLoader::_read_node_chunk(Ref<FileAccess> const& file, node_chunk_t& node) {
	bool ret = read_struct(file, node.packed);
	ret &= read_string(file, node.name);
	return ret;
}

/*
====================================
Skeleton helper functions

====================================
*/

static Transform3D _make_transform(vec3d_t const& position, quat_v1_t const& quaternion, vec3d_t const& scale) {
	Basis basis = Basis();
	basis.set_quaternion(quat_v1_to_godot(quaternion));
	basis.scale(vec3d_to_godot(scale));

	return Transform3D(basis,vec3d_to_godot(position, true));
}

//TODO: do we want to use node's bone id instead of current_id?
Skeleton3D* XacLoader::_build_armature_hierarchy(node_hierarchy_t const& hierarchy) {
	static const StringName skeleton_name = "skeleton";
	Skeleton3D* skeleton = memnew(Skeleton3D);
	skeleton->set_name(skeleton_name);

	uint32_t current_id = 0;

	for(node_data_t const& node : hierarchy.node_data) {
		skeleton->add_bone(node.name);
		skeleton->set_bone_parent(current_id, node.packed.parent_node_id);
		
		Transform3D transform = _make_transform(node.packed.position, node.packed.rotation, node.packed.scale);
		skeleton->set_bone_rest(current_id, transform);
		skeleton->set_bone_pose(current_id, transform);

		current_id += 1;
	}

	return skeleton;
}

Skeleton3D* XacLoader::_build_armature_nodes(std::vector<node_chunk_t> const& nodes) {
	static const StringName skeleton_name = "skeleton";
	Skeleton3D* skeleton = memnew(Skeleton3D);
	skeleton->set_name(skeleton_name);

	uint32_t current_id = 0;

	for(node_chunk_t const& node : nodes) {
		skeleton->add_bone(node.name);
		skeleton->set_bone_parent(current_id, node.packed.parent_node_id);
		
		Transform3D transform = _make_transform(node.packed.position, node.packed.rotation, node.packed.scale);
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

XacLoader::model_texture_set XacLoader::_get_model_textures(material_definition_t const& material) {
	static const StringName Texture_skip_nospec = "nospec";
	static const StringName Texture_skip_flag = "unionjacksquare";
	static const StringName Texture_skip_diff = "test256texture";

	model_texture_set texture_set;

	for(material_layer_t const& layer : material.layers) {
		if (layer.texture == Texture_skip_diff || layer.texture == Texture_skip_flag) { //|| layer.texture == Texture_skip_nospec
			continue;
		}
		//Get the texture names
		switch(static_cast<MAP_TYPE>(layer.packed.map_type)) {
			case MAP_TYPE::DIFFUSE:
				texture_set.diffuse_name = layer.texture;
				break;
			case MAP_TYPE::SPECULAR:
				texture_set.specular_name = layer.texture;
				break;
			case MAP_TYPE::NORMAL:
				texture_set.normal_name = layer.texture;
				break;
			case MAP_TYPE::SHADOW:
				/*UtilityFunctions::print(
					vformat("Shadow layer: %s", layer.texture)
				);*/
				break;
			default:
				UtilityFunctions::print(
					vformat("Unknown layer type: %x, texture: %s", layer.packed.map_type, layer.texture)
				);
				break;
		}
	}
	if(texture_set.specular_name.is_empty()) { texture_set.specular_name = Texture_skip_nospec; }
	return texture_set;
}

//unit colours not working, likely due to improper setup of the unit script
std::vector<XacLoader::material_mapping> XacLoader::_build_materials(std::vector<material_definition_t> const& materials) {

	// Scrolling textures (smoke, tank tracks)
	static const StringName tracks = "TexAnim";
	static const StringName smoke = "Smoke";

	//General
	ResourceLoader* loader = ResourceLoader::get_singleton();		
	std::vector<material_mapping> mappings;
	ModelSingleton* model_singleton = ModelSingleton::get_singleton();


	for(material_definition_t const& mat : materials) {
		model_texture_set texture_names = _get_model_textures(mat);
		material_mapping mapping = {};

		// *** Determine the correct material to use, and set it up ***
		// Note all 3 of these materials correspond to different compile targets of avatar.fx
		//  it has techniques for TexAnim (tracks), Smoke, Shadow, Flag, Standard (we combine TexAnim+smoke, shadow+standard),
		// in the original shader, shadow = just sample the color, smoke and flag share speed properties

		//flag TODO: perhaps flag should be determined by hard-coding so that other models can have a normal texture?
		//There shouldn't be a specular texture
		if (!texture_names.normal_name.is_empty() && texture_names.diffuse_name.is_empty()) {
			static const StringName Param_texture_normal = "texture_normal";
			static const Ref<ShaderMaterial> flag_shader = loader->load("res://src/Game/Model/flag_mat.tres");
			flag_shader->set_shader_parameter(Param_texture_normal, get_model_texture(texture_names.normal_name));
			mapping.godot_material = flag_shader;
		}
		//Scrolling texture
		else if (!texture_names.diffuse_name.is_empty() && (mat.name == tracks || mat.name == smoke)) {
			mapping.scroll_index = model_singleton->set_scroll_material_texture(texture_names.diffuse_name);
			mapping.godot_material = model_singleton->get_scroll_shader();
		}
		//standard material (diffuse optionally with a specular/unit colours)
		else if(!texture_names.diffuse_name.is_empty()){
			mapping.diffuse_texture_index = model_singleton->set_unit_material_texture(2, texture_names.diffuse_name); //MAP_TYPE::DIFFUSE
			mapping.specular_texture_index = model_singleton->set_unit_material_texture(3, texture_names.specular_name); //MAP_TYPE::SPECULAR
			mapping.godot_material = model_singleton->get_unit_shader();
		}

		else {
			UtilityFunctions::push_warning(
				vformat("Material %s did not have a diffuse texture! Skipping", mat.name)
			);
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

template<typename T>
struct byte_array_wrapper : std::span<const T> {
	byte_array_wrapper() = default;
	byte_array_wrapper(std::vector<uint8_t> const& source) :
		std::span<const T>(reinterpret_cast<T const* const>(source.data()), source.size() / sizeof(T)) {}
};

std::vector<MeshInstance3D*> XacLoader::_build_mesh(mesh_t const& mesh_chunk, skinning_t* skin, std::vector<material_mapping> const& materials) {
	static const uint32_t EXTRA_CULL_MARGIN = 2;

	std::vector<MeshInstance3D*> meshes = {}; //return value 

	byte_array_wrapper<vec3d_t> verts;
	byte_array_wrapper<vec3d_t> normals;
	byte_array_wrapper<vec4d_t> tangents;
	uint32_t uvs_read = 0;
	byte_array_wrapper<vec2d_t> uv1;
	byte_array_wrapper<vec2d_t> uv2;
	byte_array_wrapper<uint32_t> influence_range_indices;

	for(vertices_attribute_t const& attribute : mesh_chunk.vertices_attributes) {
		switch(attribute.packed.type) {
			case ATTRIBUTE::POSITION:
				verts = attribute.data;
				break;
			case ATTRIBUTE::NORMAL:
				normals = attribute.data;
				break;
			case ATTRIBUTE::TANGENT:
				tangents = attribute.data;
				break;
			case ATTRIBUTE::UV:
				if (uvs_read == 0) {
					uv1 = attribute.data;
				} 
				else if (uvs_read == 1) {
					uv2 = attribute.data;
				}
				uvs_read += 1;
				break;
			case ATTRIBUTE::INFLUENCE_RANGE:
				if (skin == nullptr) {
					Logger::warning("Mesh chunk has influence attribute but no corresponding skinning chunk");
					break;
				}
				influence_range_indices = attribute.data;
				break;
			//for now, ignore color data
			case ATTRIBUTE::COL_32:
			case ATTRIBUTE::COL_128:
			default:
				break;
		}
	}

	uint32_t vert_total = 0;
	static const StringName key_diffuse = "tex_index_diffuse";
	static const StringName key_specular = "tex_index_specular";
	static const StringName key_scroll = "scroll_tex_index_diffuse";
	
	Ref<SurfaceTool> st = Ref<SurfaceTool>();
	st.instantiate();

	//for now we treat a submesh as a godot mesh surface
	for(submesh_t const& submesh : mesh_chunk.submeshes) {
		// one mesh per submesh so that they can use different instance uniform parameters on the same material
		Ref<ArrayMesh> mesh = Ref<ArrayMesh>();
		mesh.instantiate();
	
		MeshInstance3D* mesh_inst = memnew(MeshInstance3D);
		mesh_inst->set_extra_cull_margin(EXTRA_CULL_MARGIN);
		mesh_inst->set_mesh(mesh);


		st->begin(Mesh::PRIMITIVE_TRIANGLES);

		for(int j=0; j<submesh.relative_indices.size(); j++) {
			int32_t rel_index = submesh.relative_indices[j] + vert_total;
			if(!normals.empty()) {
				st->set_normal(vec3d_to_godot(normals[rel_index],true));
			}
			if(!tangents.empty()) {
				vec4d_t const& v4 = tangents[rel_index];
				st->set_tangent(Plane(-v4.x,v4.y,v4.z,v4.w));
			}
			if(!uv1.empty()) {
				st->set_uv(vec2d_to_godot(uv1[rel_index]));
			}
			if(!uv2.empty()) {
				st->set_uv2(vec2d_to_godot(uv2[rel_index]));
			}
			if(skin != nullptr && !influence_range_indices.empty()) {
				PackedInt32Array boneIds = {0,0,0,0};
				PackedFloat32Array weights = {0,0,0,0};

				influence_range_t const& infl_range = skin->influence_ranges[influence_range_indices[rel_index]];
				int32_t first = infl_range.first_influence_index;
				int32_t count = Math::min(infl_range.influences_count,4);

				for(int32_t i=0; i<count; i++){
					influence_data_t const& infl_data = skin->influence_data[first + i];
					boneIds[i] = infl_data.bone_id;
					weights[i] = infl_data.weight;
				}

				st->set_bones(boneIds);
				st->set_weights(weights);
			}
			st->add_vertex(vec3d_to_godot(verts[rel_index],true));
		}

		material_mapping const& material_mapping = materials[submesh.packed.material_id];
		st->set_material(material_mapping.godot_material);

		mesh = st->commit(mesh); // add a new surface to the mesh
		st->clear();

		vert_total += submesh.packed.vertices_count;

		//setup materials for the surface
		if (material_mapping.diffuse_texture_index != -1) {
			mesh_inst->set_instance_shader_parameter(key_diffuse, material_mapping.diffuse_texture_index);
		}
		if (material_mapping.specular_texture_index != -1) {
			mesh_inst->set_instance_shader_parameter(key_specular, material_mapping.specular_texture_index);
		}
		if (material_mapping.scroll_index != -1) {
			mesh_inst->set_instance_shader_parameter(key_scroll, material_mapping.scroll_index);
		}

		meshes.push_back(mesh_inst);
	}

	return meshes;
}

/*
====================================
Main loading function

====================================
*/

Node3D* XacLoader::load_xac_model(Ref<FileAccess> const& file, bool is_unit) {
	bool res = _read_xac_header(file);	

	ResourceLoader* loader = ResourceLoader::get_singleton();

	Node3D* base = memnew(Node3D);

	if (!res) {
		return base;
	}

	xac_metadata_v2_t metadata;
	
	bool hierarchy_read = false;
	node_hierarchy_t hierarchy;
	std::vector<node_chunk_t> nodes; //other way of making a bone hierarchy
	
	material_totals_t material_totals = {};
	std::vector<material_definition_t> materials;

	std::vector<mesh_t> meshes;
	std::vector<skinning_t> skinnings;

	bool log_all = false;

	//0xA unknown (ex. Spanish_Helmet1.xac), only 2 bytes long...
	//0x6 unknown (ex. ...)
	//0x8 junk data (ie. old versions of the file duplicated in append mode)
	//	(ex. Port_Empty.xac)
	//0xC morphtargets (not used in vic2)

	while (!file->eof_reached()) {
		chunk_header_t header = {};
		if (!read_chunk_header(file, header)) {
			break;
		}

		if (log_all) {
			UtilityFunctions::print(
			vformat("XAC chunk: type = %x, length = %x, version = %d at %x", header.type, header.length, header.version, file->get_position())
			);
		}

		if (header.type == 0x7 && header.version == 2) {
			_read_xac_metadata(file, metadata);
		}
		else if (header.type == 0xB && header.version == 1) {
			hierarchy_read = _read_node_hierarchy(file, hierarchy);
		} 
		else if (header.type == 0x0 && header.version == 3) {
			node_chunk_t node;
			_read_node_chunk(file, node);
			nodes.push_back(node);
		}
		else if (header.type == 0x1 && header.version == 1) {
			mesh_t mesh;
			_read_mesh(file, mesh);
			meshes.push_back(mesh);
		}
		else if (header.type == 0x2) {
			skinning_t skin;
			_read_skinning(file, skin, meshes, header.version);
			skinnings.push_back(skin);
		}
		else if (header.type == 0x3) {
			material_definition_t mat;
			_read_material_definition(file, mat, header.version);
			materials.push_back(mat);
		}
		else if (header.type == 0x4 && header.version == 2) {
			material_layer_t layer;
			_read_layer(file, layer);
			if (layer.packed.material_id < materials.size()) {
				materials[layer.packed.material_id].layers.push_back(layer);
			}
			else{
				Logger::error("No material of id ", layer.packed.material_id, " to attach layer to");
			}
		}
		else if (header.type == 0xD && header.version == 1) {
			_read_material_totals(file, material_totals);
		}
		else if (header.type == 0x8 || header.type == 0xA) { //skip junk data and whatever 0xA is...
			if (header.length + file->get_position() < file->get_length()) {
				file->get_buffer(header.length);
			}
			else {
				res = false;
				break;
			} 
		}
		else {
			UtilityFunctions::print(
			vformat("Unsupported XAC chunk: type = %x, length = %x, version = %d at %x", header.type, header.length, header.version, file->get_position())
			);
			log_all = true;
			// Skip unsupported chunks by using get_buffer, make sure this doesn't break anything, since chunk length can be wrong
			if (header.length + file->get_position() < file->get_length()) {
				file->get_buffer(header.length);
			}
			else {
				res = false;
				break;
			} 
		}
	}
	
	base->set_name(metadata.original_file_name.get_file().get_basename());

	//Setup skeleton
	Skeleton3D* skeleton = nullptr;
	if(!skinnings.empty()) {
		if (hierarchy_read) {
			skeleton = _build_armature_hierarchy(hierarchy);
			base->add_child(skeleton);
		}
		else if (!nodes.empty()) {
			skeleton = _build_armature_nodes(nodes);
			base->add_child(skeleton);
		}
	}

	//Setup materials
	std::vector<material_mapping> const& material_mappings = _build_materials(materials);

	bool has_specular = false;
	for(material_mapping const& material : material_mappings) {
		if(material.specular_texture_index != -1){
			has_specular = true;
			break;
		}
	}

	//don't set the unit script if we don't have one of:
	// -specular (unit colours) texture
	// -skeleton with animation
	static const Ref<godot::Script> unit_script = loader->load("res://src/Game/Model/UnitModel.gd");
	if(is_unit || has_specular) {
		base->set_script(unit_script);
	}

	//setup mesh
	for(mesh_t const& mesh : meshes) {
		if (mesh.packed.is_collision_mesh) { continue; } //we'll use our own collision meshes where needed

		skinning_t* mesh_skin = nullptr;
		for(uint32_t i=0; i < skinnings.size(); i++) {
			skinning_t& skin = skinnings[i];
			if (skin.node_id == mesh.packed.node_id && skin.packed.is_for_collision == mesh.packed.is_collision_mesh) {
				mesh_skin = &skin;
				break;
			}
		}

		if (skeleton != nullptr && mesh_skin == nullptr) { continue; }

		std::vector<MeshInstance3D*> mesh_instances = _build_mesh(mesh, mesh_skin, material_mappings);
		for(MeshInstance3D* mesh_inst : mesh_instances){
			base->add_child(mesh_inst);
			mesh_inst->set_owner(base);
			
			if (skeleton != nullptr) {
				mesh_inst->set_skeleton_path(mesh_inst->get_path_to(skeleton));
				int32_t node_id = mesh.packed.node_id;
				if (hierarchy_read && node_id < hierarchy.node_data.size()) {
					mesh_inst->set_name(hierarchy.node_data[node_id].name);
				}
				else if (!nodes.empty() && node_id < nodes.size()) {
					mesh_inst->set_name(nodes[node_id].name);
				}
			}
		}
		/*base->add_child(mesh_inst);
		mesh_inst->set_owner(base);
		
		if (skeleton != nullptr) {
			mesh_inst->set_skeleton_path(mesh_inst->get_path_to(skeleton));
			int32_t node_id = mesh.packed.node_id;
			if (hierarchy_read && node_id < hierarchy.node_data.size()) {
				mesh_inst->set_name(hierarchy.node_data[node_id].name);
			}
			else if (!nodes.empty() && node_id < nodes.size()) {
				mesh_inst->set_name(nodes[node_id].name);
			}
		} */
	}

	return base;
}