#include "XACLoader.hpp"
//#include "Utilities.hpp"

#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/resource_preloader.hpp"
#include "godot_cpp/classes/script.hpp"
#include "godot_cpp/classes/shader.hpp"
#include "godot_cpp/classes/shader_material.hpp"
#include "godot_cpp/classes/skeleton3d.hpp"

#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/packed_int32_array.hpp"
#include "godot_cpp/variant/packed_int64_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/packed_vector4_array.hpp"
#include "godot_cpp/variant/transform3d.hpp"
#include "godot_cpp/variant/variant.hpp"

#include "openvic-simulation/utility/Logger.hpp"

using namespace::OpenVic;
using namespace::godot;

bool XacLoader::read_xac_header(Ref<FileAccess> const& file) {
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

bool XacLoader::read_xac_metadata(godot::Ref<godot::FileAccess> const& file, xac_metadata_v2_t& metadata) {
	bool ret = read_struct(file, metadata.packed);
	ret &= read_string(file, metadata.source_app, false);
	ret &= read_string(file, metadata.original_file_name, false);
	ret &= read_string(file, metadata.export_date, false);
	ret &= read_string(file, metadata.actor_name, true);
	return ret;
}

bool XacLoader::read_node_data(godot::Ref<godot::FileAccess> const& file, node_data_t& node_data) {
	bool ret = read_struct(file, node_data.packed);
	ret &= read_string(file, node_data.name);
	return ret;
}

bool XacLoader::read_node_hierarchy(godot::Ref<godot::FileAccess> const& file, node_hierarchy_t& hierarchy) {
	bool ret = read_struct(file, hierarchy.packed);
	for(int32_t i=0; i<hierarchy.packed.node_count; i++) {
		node_data_t node;
		ret &= read_node_data(file, node);
		hierarchy.node_data.push_back(node);
	}
	return ret;
}

bool XacLoader::read_material_totals(godot::Ref<godot::FileAccess> const& file, material_totals_t& totals) {
	return read_struct(file, totals);
}

bool XacLoader::read_layer(godot::Ref<godot::FileAccess> const& file, material_layer_t& layer) {
	bool ret = true;
	ret &= read_struct(file, layer.packed);
	ret &= read_string(file, layer.texture, false);
	//Logger::info(Utilities::godot_to_std_string(layer.texture));
	return ret;
}

bool XacLoader::read_material_definition(godot::Ref<godot::FileAccess> const& file, material_definition_t& def, int32_t version) {
	bool ret = read_struct(file, def.packed);
	ret &= read_string(file, def.name, false);
	if (version > 1) { //in version 1, the layers are defined in separate chunks of type 0x4
		for(uint32_t i=0; i<def.packed.layers_count; i++) {
			material_layer_t layer;
			ret &= read_layer(file, layer);
			def.layers.push_back(layer);
		}
	}
	return ret;
}

bool XacLoader::read_vertices_attribute(godot::Ref<godot::FileAccess> const& file, vertices_attribute_t& attribute, int32_t vertices_count) {
	bool ret = read_struct(file, attribute.packed);
	ret &= read_struct_array(file, attribute.data, vertices_count*attribute.packed.attribute_size);
	return ret;
}

bool XacLoader::read_submesh(godot::Ref<godot::FileAccess> const& file, submesh_t& submesh) {
	bool ret = read_struct(file, submesh.packed);
	ret &= read_struct_array(file, submesh.relative_indices, submesh.packed.indices_count);
	ret &= read_struct_array(file, submesh.bone_ids, submesh.packed.bones_count);
	return ret;
}

bool XacLoader::read_mesh(godot::Ref<godot::FileAccess> const& file, mesh_t& mesh) {
	bool ret = read_struct(file, mesh.packed);
	for(uint32_t i=0; i<mesh.packed.attribute_layers_count; i++) {
		vertices_attribute_t attribute;
		ret &= read_vertices_attribute(file, attribute, mesh.packed.vertices_count);
		mesh.vertices_attributes.push_back(attribute);
	}
	for(uint32_t i=0; i<mesh.packed.submeshes_count; i++) {
		submesh_t submesh;
		ret &= read_submesh(file, submesh);
		mesh.submeshes.push_back(submesh);
	}
	return ret;
}

bool XacLoader::read_skinning(godot::Ref<godot::FileAccess> const& file, skinning_t& skin, std::vector<mesh_t> const& meshes, int32_t version) {
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

bool XacLoader::read_node_chunk(godot::Ref<godot::FileAccess> const& file, node_chunk_t& node) {
	bool ret = read_struct(file, node.packed);
	ret &= read_string(file, node.name);
	//Logger::info(Utilities::godot_to_std_string(node.name));
	return ret;
}

/*
====================================
Skeleton helper functions

====================================
*/

//TODO: Verify
static godot::Transform3D make_transform(vec3d_t const& position, quat_v1_t const& quaternion, vec3d_t const& scale) {
	godot::Transform3D transform = godot::Transform3D();

	godot::Basis basis = godot::Basis();
	basis.set_quaternion(quat_v1_to_godot(quaternion));
	basis.scale(vec3d_to_godot(scale));

	transform.set_basis(basis);
	transform.translate_local(vec3d_to_godot(position, true));

	return transform;
}

//TODO: do we want to use node's bone id instead of current_id?
godot::Skeleton3D* XacLoader::build_armature_hierarchy(node_hierarchy_t const& hierarchy) {
	static const godot::StringName skeleton_name = "skeleton";
	godot::Skeleton3D* skeleton = memnew(godot::Skeleton3D);
	skeleton->set_name(skeleton_name);

	uint32_t current_id = 0;

	for(node_data_t const& node : hierarchy.node_data) {
		skeleton->add_bone(node.name);
		skeleton->set_bone_parent(current_id, node.packed.parent_node_id);
		
		godot::Transform3D transform = make_transform(node.packed.position, node.packed.rotation, node.packed.scale);
		skeleton->set_bone_rest(current_id, transform);
		skeleton->set_bone_pose(current_id, transform);

		current_id += 1;
	}

	return skeleton;
}

godot::Skeleton3D* XacLoader::build_armature_nodes(std::vector<node_chunk_t> const& nodes) {
	static const godot::StringName skeleton_name = "skeleton";
	godot::Skeleton3D* skeleton = memnew(godot::Skeleton3D);
	skeleton->set_name(skeleton_name);

	uint32_t current_id = 0;

	for(node_chunk_t const& node : nodes) {
		skeleton->add_bone(node.name);
		skeleton->set_bone_parent(current_id, node.packed.parent_node_id);
		
		godot::Transform3D transform = make_transform(node.packed.position, node.packed.rotation, node.packed.scale);
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

//TODO: remove if unnecessary
/*static void set_model_texture_set_texture(godot::String const& layer_name, godot::String* name) {
	if (name->is_empty()) {
		*name = layer_name;
	}
	else {
		Logger::error("Multiple diffuse layers in material: ",
			Utilities::godot_to_std_string(*name),
			" and ",
			Utilities::godot_to_std_string(layer_name)
		);
	}
	return;
}*/

XacLoader::model_texture_set XacLoader::get_model_textures(material_definition_t const& material) {
	static const godot::StringName Texture_skip_nospec = "nospec";
	static const godot::StringName Texture_skip_flag = "unionjacksquare";
	static const godot::StringName Texture_skip_diff = "test256texture";

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
				/*godot::UtilityFunctions::print(
					godot::vformat("Shadow layer: %s", layer.texture)
				);*/
				break;
			default:
				godot::UtilityFunctions::print(
					godot::vformat("Unknown layer type: %x, texture: %s", layer.packed.map_type, layer.texture)
				);
				break;
		}
	}
	if(texture_set.specular_name.is_empty()) { texture_set.specular_name = Texture_skip_nospec; }
	return texture_set;
}

//unit colours not working, likely due to improper setup of the unit script
std::vector<XacLoader::material_mapping> XacLoader::build_materials(std::vector<material_definition_t> const& materials) {

	// Scrolling textures (smoke, tank tracks)
	static const godot::StringName tracks = "TexAnim";
	static const godot::StringName smoke = "Smoke";

	//General
	godot::ResourceLoader* loader = godot::ResourceLoader::get_singleton();		
	std::vector<material_mapping> mappings;
	ModelSingleton* model_singleton = ModelSingleton::get_singleton();


	for(material_definition_t const& mat : materials) {
		model_texture_set texture_names = get_model_textures(mat);
		material_mapping mapping;

		// *** Determine the correct material to use, and set it up ***
		// Note all 3 of these materials correspond to different compile targets of avatar.fx
		//  it has techniques for TexAnim (tracks), Smoke, Shadow, Flag, Standard (we combine TexAnim+smoke, shadow+standard),
		// in the original shader, shadow = just sample the color, smoke and flag share speed properties

		//flag TODO: perhaps flag should be determined by hard-coding so that other models can have a normal texture?
		//There shouldn't be a specular texture
		if (!texture_names.normal_name.is_empty() && texture_names.diffuse_name.is_empty()) {
			static const godot::StringName Param_texture_normal = "texture_normal";
			static const godot::Ref<godot::ShaderMaterial> flag_shader = loader->load("res://src/Game/Model/flag_mat.tres");
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
			godot::UtilityFunctions::push_warning(
				godot::vformat("Material %s did not have a diffuse texture! Skipping", mat.name)
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

template<typename T>
struct byte_array_wrapper : std::span<const T> {
	byte_array_wrapper() = default;
	byte_array_wrapper(std::vector<uint8_t> const& source) :
		std::span<const T>(reinterpret_cast<T const* const>(source.data()), source.size() / sizeof(T)) {}
};

godot::MeshInstance3D* XacLoader::build_mesh(mesh_t const& mesh_chunk, skinning_t* skin, std::vector<material_mapping> const& materials) {
	static const uint32_t EXTRA_CULL_MARGIN = 2;

	godot::MeshInstance3D* mesh_inst = memnew(godot::MeshInstance3D);
	mesh_inst->set_extra_cull_margin(EXTRA_CULL_MARGIN);

	godot::Ref<godot::ArrayMesh> mesh = godot::Ref<godot::ArrayMesh>();
	mesh.instantiate();

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

	//if (skin != nullptr) Logger::info("skin infdat size: ", skin->influence_data.size(), " ", skin->packed.influences_count);
	//if (skin != nullptr) Logger::info("skin infran size: ", skin->influence_ranges.size(), " ", mesh_chunk.packed.influence_ranges_count);
	//else Logger::info("no skinning chunk");
	

	uint32_t vert_total = 0;
	static const godot::StringName key_diffuse = "tex_index_diffuse";
	static const godot::StringName key_specular = "tex_index_specular";
	static const godot::StringName key_scroll = "scroll_tex_index_diffuse";

	//for now we treat a submesh as a godot mesh surface
	for(submesh_t const& submesh : mesh_chunk.submeshes) {
		godot::Array arrs; //surface vertex data arrays
		ERR_FAIL_COND_V(arrs.resize(godot::Mesh::ARRAY_MAX) != godot::OK, {});

		godot::PackedVector3Array verts_submesh = {};
		godot::PackedVector3Array normals_submesh = {};
		godot::PackedFloat32Array tangents_submesh = {};
		godot::PackedVector2Array uv1_submesh = {};
		godot::PackedVector2Array uv2_submesh = {};
		godot::PackedInt32Array bone_ids = {};
		godot::PackedFloat32Array weights = {};
		//godot uses a fixed 4 bones influencing a vertex, so size the array accordingly
		//TODO: should this actually be verts.size()?
		bone_ids.resize(submesh.relative_indices.size()*4);
		weights.resize(submesh.relative_indices.size()*4);

		//1 vertex is in the surface per relative index
		for(uint32_t const& rel_ind : submesh.relative_indices) {
			uint32_t index = rel_ind + vert_total;

			verts_submesh.push_back(vec3d_to_godot(verts[index], true));
			if (normals.size() > index) {
				normals_submesh.push_back( vec3d_to_godot(normals[index],true));
			}
			
			//TODO: verify if we need to invert the x for each tangent
			if (tangents.size() > index) {
				vec4d_t const& tan = tangents[index];
				tangents_submesh.push_back(-tan.x);
				tangents_submesh.push_back(tan.y);
				tangents_submesh.push_back(tan.z);
				tangents_submesh.push_back(tan.w);
			}
			
			if (uv1.size() > index) {
				uv1_submesh.push_back(vec2d_to_godot(uv1[index]));
			}
			if (uv2.size() > index) {
				uv2_submesh.push_back(vec2d_to_godot(uv2[index]));
			}

			if (skin != nullptr && influence_range_indices.size() > index) {
				uint32_t const& influence_range_ind = influence_range_indices[index];
				influence_range_t const& range = skin->influence_ranges[influence_range_ind];

				for(uint32_t i = 0; i < godot::Math::min(range.influences_count,4); i++) {
					influence_data_t const& inf_data = skin->influence_data[range.first_influence_index + i];
					bone_ids[rel_ind*4 + i] = inf_data.bone_id;
					weights[rel_ind*4 + i] = inf_data.weight;
				}
			}
		}

		if (!verts_submesh.is_empty()) {
			arrs[godot::Mesh::ARRAY_VERTEX] = std::move(verts_submesh);
		}
		if (!normals_submesh.is_empty()) {
			arrs[godot::Mesh::ARRAY_NORMAL] = std::move(normals_submesh);
		}
		if (!tangents_submesh.is_empty()) {
			arrs[godot::Mesh::ARRAY_TANGENT] = std::move(tangents_submesh);
		}
		if (!uv1_submesh.is_empty()) {
			arrs[godot::Mesh::ARRAY_TEX_UV] = std::move(uv1_submesh);
		}
		if (!uv2_submesh.is_empty()) {
			arrs[godot::Mesh::ARRAY_TEX_UV2] = std::move(uv2_submesh);
		}
		if (skin != nullptr && influence_range_indices.size() > 0) {
			arrs[godot::Mesh::ARRAY_BONES] = std::move(bone_ids);
			arrs[godot::Mesh::ARRAY_WEIGHTS] = std::move(weights);
		}
		mesh->add_surface_from_arrays(godot::Mesh::PRIMITIVE_TRIANGLES, arrs);

		//setup materials for the surface
		material_mapping const& material_mapping = materials[submesh.packed.material_id];
		mesh->surface_set_material(mesh->get_surface_count()-1, material_mapping.godot_material);
		if (material_mapping.diffuse_texture_index != -1) {
			mesh_inst->set_instance_shader_parameter(key_diffuse, material_mapping.diffuse_texture_index);
		}
		if (material_mapping.specular_texture_index != -1) {
			mesh_inst->set_instance_shader_parameter(key_specular, material_mapping.specular_texture_index);
		}
		if (material_mapping.scroll_index != -1) {
			mesh_inst->set_instance_shader_parameter(key_scroll, material_mapping.scroll_index);
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

godot::Node3D* XacLoader::_load_xac_model(godot::Ref<godot::FileAccess> const& file) {
	bool res = read_xac_header(file);

	// TODO: gets the unit script if it has the specular/unit colours texture or
	
	/*is_unit = is_unit or (
		# Needed for animations
		idle_key in model_dict or move_key in model_dict or attack_key in model_dict
		# Currently needs UnitModel's attach_model helper function
		or attachments_key in model_dict
	)*/
	
	godot::ResourceLoader* loader = godot::ResourceLoader::get_singleton();

	//TODO: start with the assumption that all units will use this, then reduce later
	static const godot::Ref<godot::Script> unit_script = loader->load("res://src/Game/Model/UnitModel.gd");

	godot::Node3D* base = memnew(godot::Node3D);

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
			godot::UtilityFunctions::print(
			godot::vformat("XAC chunk: type = %x, length = %x, version = %d at %x", header.type, header.length, header.version, file->get_position())
			);
		}

		if (header.type == 0x7 && header.version == 2) {
			read_xac_metadata(file, metadata);
		}
		else if (header.type == 0xB && header.version == 1) {
			hierarchy_read = read_node_hierarchy(file, hierarchy);
		} 
		else if (header.type == 0x0 && header.version == 3) {
			node_chunk_t node;
			read_node_chunk(file, node);
			nodes.push_back(node);
		}
		else if (header.type == 0x1 && header.version == 1) {
			mesh_t mesh;
			read_mesh(file, mesh);
			meshes.push_back(mesh);
		}
		else if (header.type == 0x2) {
			skinning_t skin;
			read_skinning(file, skin, meshes, header.version);
			skinnings.push_back(skin);
		}
		else if (header.type == 0x3) {
			material_definition_t mat;
			read_material_definition(file, mat, header.version);
			materials.push_back(mat);
		}
		else if (header.type == 0x4 && header.version == 2) {
			material_layer_t layer;
			read_layer(file, layer);
			if (layer.packed.material_id < materials.size()) {
				materials[layer.packed.material_id].layers.push_back(layer);
			}
			else{
				Logger::error("No material of id ", layer.packed.material_id, " to attach layer to");
			}
		}
		else if (header.type == 0xD && header.version == 1) {
			read_material_totals(file, material_totals);
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
			godot::UtilityFunctions::print(
			godot::vformat("Unsupported XAC chunk: type = %x, length = %x, version = %d at %x", header.type, header.length, header.version, file->get_position())
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
	//TODO: don't set the unit script if we don't have one of:
	// -specular (unit colours) texture
	// -skeleton with animation
	base->set_script(unit_script);

	//Setup skeleton
	//TODO: if no skinning chunk, skip
	godot::Skeleton3D* skeleton = nullptr;
	if(!skinnings.empty()) {
		if (hierarchy_read) {
			skeleton = build_armature_hierarchy(hierarchy);
			base->add_child(skeleton);
		}
		else if (!nodes.empty()) {
			skeleton = build_armature_nodes(nodes);
			base->add_child(skeleton);
		}
	}

	//Setup materials
	std::vector<material_mapping> const& material_mappings = build_materials(materials);

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

		godot::MeshInstance3D* mesh_inst = build_mesh(mesh, mesh_skin, material_mappings);
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
	//base->call("unit_init", true);

	return base;
}