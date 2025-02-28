#pragma once
#include <cstdint>
#include <vector>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector4_array.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "XACUtilities.hpp"

namespace OpenVic {
	static constexpr uint32_t XSM_FORMAT_SPECIFIER = ' MSX'; /* Order reversed due to little endian */
	static constexpr uint8_t XSM_VERSION_MAJOR = 1, XSM_VERSION_MINOR = 0;
	
	// Pack structs of data that can be read directly
	
	#pragma pack(push)
	#pragma pack(1)
	
	struct xsm_header_t {
		uint32_t format_identifier;
		uint8_t version_major;
		uint8_t version_minor;
		uint8_t big_endian;
		uint8_t pad;
	};
	
	//v2 of the header adds these 2 properties at the start
	struct xsm_metadata_v2_pack_extra_t {
		float unused;
		float max_acceptable_error;
	};
	
	struct xsm_metadata_pack_t {
		int32_t fps;
		uint8_t exporter_version_major; //these 3 properties are uncertain
		uint8_t exporter_version_minor;
		uint16_t pad;
	};
	
	struct position_key_t {
		vec3d_t position;
		float time;
	};
	
	struct rotation_key_v2_t {
		quat_v2_t rotation;
		float time;
	};
	
	struct rotation_key_v1_t {
		quat_v1_t rotation;
		float time;
	};
	
	struct scale_key_t {
		vec3d_t scale;
		float time;
	};
	
	//the quaternions come at the start of the skeletal submotion
	struct submotion_rot_v2_pack_t {
		quat_v2_t pose_rotation;
		quat_v2_t bind_pose_rotation;
		quat_v2_t pose_scale_rotation;
		quat_v2_t bind_pose_scale_rotation;
	};
	
	struct submotion_rot_v1_pack_t {
		quat_v1_t pose_rotation;
		quat_v1_t bind_pose_rotation;
		quat_v1_t pose_scale_rotation;
		quat_v1_t bind_pose_scale_rotation;
	};
	
	struct skeletal_submotion_pack_t {
		vec3d_t pose_position;
		vec3d_t pose_scale;
		vec3d_t bind_pose_position;
		vec3d_t bind_pose_scale;
		int32_t position_key_count;
		int32_t rotation_key_count;
		int32_t scale_key_count;
		int32_t scale_rotation_key_count;
		float max_error;
	};
	
	struct node_submotion_pack_t {
		vec3d_t pose_position;
		vec3d_t pose_scale;
		vec3d_t bind_pose_position;
		vec3d_t bind_pose_scale;
		int32_t position_key_count;
		int32_t rotation_key_count;
		int32_t scale_key_count;
		int32_t scale_rotation_key_count;
	};
	
	#pragma pack(pop)
	
	struct xsm_metadata_t {
		bool has_v2_data = false;
		xsm_metadata_v2_pack_extra_t v2_data = {};
		xsm_metadata_pack_t packed = {};
		godot::String source_app;
		godot::String original_file_name;
		godot::String export_date;
		godot::String motion_name;
	};
	
	struct rotation_key_t {
		godot::Quaternion rotation;
		float time;
	};
	
	struct skeletal_submotion_t {
		godot::Quaternion pose_rotation;
		godot::Quaternion bind_pose_rotation;
		godot::Quaternion pose_scale_rotation;
		godot::Quaternion bind_pose_scale_rotation;
		skeletal_submotion_pack_t packed = {};
		godot::String node_name;
		std::vector<position_key_t> position_keys;
		std::vector<rotation_key_t> rotation_keys;
		std::vector<scale_key_t> scale_keys;
		std::vector<rotation_key_t> scale_rotation_keys;
	};
	
	struct bone_animation_t {
		int32_t submotion_count = 0;
		std::vector<skeletal_submotion_t> submotions;
	};
	
	/*
	0xC8, v1 chunk documentation: (Let's call it node_submotion)
		4*8x 32 bit sections = 32
			4x quat32 = 16
			4x vec3   = 12
			4x keycount 04
						32
		string node_name
		data...
	
		so this chunk is the same as a skeletal submotion v1
		but without the fMaxError
	*/
	
	struct node_submotion_t {
		godot::Quaternion pose_rotation;
		godot::Quaternion bind_pose_rotation;
		godot::Quaternion pose_scale_rotation;
		godot::Quaternion bind_pose_scale_rotation;
		node_submotion_pack_t packed = {};
		godot::String node_name;
		std::vector<position_key_t> position_keys;
		std::vector<rotation_key_t> rotation_keys;
		std::vector<scale_key_t> scale_keys;
		std::vector<rotation_key_t> scale_rotation_keys;
	};
	
	static bool read_xsm_header(godot::Ref<godot::FileAccess> const& file) {
		xsm_header_t header = {};
		ERR_FAIL_COND_V(!read_struct(file, header), false);
	
		//Logger::info("XSM file version: ", header.version_major, ".", header.version_minor," big endian?: ",header.big_endian);
	
		ERR_FAIL_COND_V_MSG(
			header.format_identifier != XSM_FORMAT_SPECIFIER, false, godot::vformat(
				"Invalid XSM format identifier: %x (should be %x)", header.format_identifier, XSM_FORMAT_SPECIFIER
			)
		);
	
		ERR_FAIL_COND_V_MSG(
			header.version_major != XSM_VERSION_MAJOR || header.version_minor != XSM_VERSION_MINOR, false, godot::vformat(
				"Invalid XSM version: %d.%d (should be %d.%d)",
				header.version_major, header.version_minor, XSM_VERSION_MAJOR, XSM_VERSION_MINOR
			)
		);
	
		ERR_FAIL_COND_V_MSG(
			header.big_endian != 0, false, "Invalid XSM endianness: big endian (only little endian is supported)"
		);
	
		return true;
	}
	
	static bool read_xsm_metadata(godot::Ref<godot::FileAccess> const& file, xsm_metadata_t& metadata, int32_t version) {
		bool res = true;
		if (version != 1) {
			res &= read_struct(file, metadata.v2_data);
			metadata.has_v2_data = true;
		} 
		res &= read_struct(file, metadata.packed);
		res &= read_string(file, metadata.source_app);
		res &= read_string(file, metadata.original_file_name);
		res &= read_string(file, metadata.export_date);
		res &= read_string(file, metadata.motion_name);
	
		return res;
	}
	
	static bool read_rot_keys(godot::Ref<godot::FileAccess> const& file, std::vector<rotation_key_t>* keys_out, int32_t count, int32_t version) {
		bool res = true;
	
		if (version == 1) {
			std::vector<rotation_key_v1_t> rot_keys_v1;
			res &= read_struct_array(file,rot_keys_v1,count);
			for(rotation_key_v1_t key : rot_keys_v1) {
				keys_out->push_back({quat_v1_to_godot(key.rotation), key.time});
			}
		}
		else {
			std::vector<rotation_key_v2_t> rot_keys_v2;
			res &= read_struct_array(file,rot_keys_v2,count);
			for(rotation_key_v2_t key : rot_keys_v2) {
				keys_out->push_back({quat_v2_to_godot(key.rotation), key.time});
			}
		}
	
		return res;
	}
	
	static bool read_skeletal_submotion(godot::Ref<godot::FileAccess> const& file, skeletal_submotion_t& submotion, int32_t version) {
		bool res = true;
		if (version == 1) { //float component quats (v1)
			submotion_rot_v1_pack_t rot_comps = {};
			res &= read_struct(file,rot_comps);
			submotion.pose_rotation = quat_v1_to_godot(rot_comps.pose_rotation);
			submotion.bind_pose_rotation = quat_v1_to_godot(rot_comps.bind_pose_rotation);
			submotion.pose_scale_rotation = quat_v1_to_godot(rot_comps.pose_scale_rotation);
			submotion.bind_pose_scale_rotation = quat_v1_to_godot(rot_comps.bind_pose_scale_rotation);
		}
		else { //int16 component quats (v2)
			submotion_rot_v2_pack_t rot_comps = {};
			res &= read_struct(file,rot_comps);
			submotion.pose_rotation = quat_v2_to_godot(rot_comps.pose_rotation);
			submotion.bind_pose_rotation = quat_v2_to_godot(rot_comps.bind_pose_rotation);
			submotion.pose_scale_rotation = quat_v2_to_godot(rot_comps.pose_scale_rotation);
			submotion.bind_pose_scale_rotation = quat_v2_to_godot(rot_comps.bind_pose_scale_rotation);
		}
			
		ERR_FAIL_COND_V(!read_struct(file, submotion.packed), false);
		res &= read_string(file, submotion.node_name,true);
		res &= read_struct_array(file,submotion.position_keys,submotion.packed.position_key_count);
		res &= read_rot_keys(file,&submotion.rotation_keys,submotion.packed.rotation_key_count, version);
		res &= read_struct_array(file,submotion.scale_keys,submotion.packed.scale_key_count);
		res &= read_rot_keys(file,&submotion.scale_rotation_keys,submotion.packed.scale_rotation_key_count, version);
	
		return res;
	}
	
	static bool read_xsm_bone_animation(godot::Ref<godot::FileAccess> const& file, bone_animation_t& bone_animation, int32_t version) {
		bone_animation.submotion_count = file->get_32();
		bool ret = true;
	
		for(int i=0; i<bone_animation.submotion_count; i++) {
			skeletal_submotion_t submotion;
			ret &= read_skeletal_submotion(file, submotion, version);
			bone_animation.submotions.push_back(submotion);
		}
	
		return true;
	}
	
	static bool read_node_submotion(godot::Ref<godot::FileAccess> const& file, std::vector<node_submotion_t>* submotions) {
		bool res = true;
		node_submotion_t submotion;
		
		submotion_rot_v1_pack_t rot_comps = {};
		res &= read_struct(file,rot_comps);
		submotion.pose_rotation = quat_v1_to_godot(rot_comps.pose_rotation);
		submotion.bind_pose_rotation = quat_v1_to_godot(rot_comps.bind_pose_rotation);
		submotion.pose_scale_rotation = quat_v1_to_godot(rot_comps.pose_scale_rotation);
		submotion.bind_pose_scale_rotation = quat_v1_to_godot(rot_comps.bind_pose_scale_rotation);
		
		res &= read_struct(file, submotion.packed);
		res &= read_string(file, submotion.node_name);
		res &= read_struct_array(file,submotion.position_keys,submotion.packed.position_key_count);
		res &= read_rot_keys(file,&submotion.rotation_keys,submotion.packed.rotation_key_count, 1);
		res &= read_struct_array(file,submotion.scale_keys,submotion.packed.scale_key_count);
		res &= read_rot_keys(file,&submotion.scale_rotation_keys,submotion.packed.scale_rotation_key_count, 1);
		submotions->push_back(submotion);
	
		return res;
	}
	
	//returns highest time in the submotion
	template<typename T>
	float add_submotion(godot::Ref<godot::Animation> anim, T submotion) {
		static const godot::StringName SKELETON_NODE_PATH = "./skeleton:%s";
		float max_time = 0;
	
		//NOTE: godot uses ':' to specify properties, so we replaced such characters with '_' when we read them in
		godot::String skeleton_path = godot::vformat(SKELETON_NODE_PATH, submotion.node_name);
	
		int pos_id = anim->add_track(godot::Animation::TYPE_POSITION_3D);
		int rot_id = anim->add_track(godot::Animation::TYPE_ROTATION_3D);
		int scale_id = anim->add_track(godot::Animation::TYPE_SCALE_3D);
	
		anim->track_set_path(pos_id, skeleton_path);
		anim->track_set_path(rot_id, skeleton_path);
		anim->track_set_path(scale_id, skeleton_path);
	
		for(position_key_t key : submotion.position_keys) {
			anim->position_track_insert_key(pos_id, key.time, vec3d_to_godot(key.position, true));
			if (key.time > max_time) {
				max_time = key.time;
			}
		}
	
		for(rotation_key_t key : submotion.rotation_keys) {
			anim->rotation_track_insert_key(rot_id, key.time, key.rotation);
			if (key.time > max_time) {
				max_time = key.time;
			}
		}
		
		//not needed for vic2 animations, but we can still support it
		for(scale_key_t key : submotion.scale_keys) {
			anim->scale_track_insert_key(scale_id, key.time, vec3d_to_godot(key.scale));
			if (key.time > max_time) {
				max_time = key.time;
			}
		}
		
		// TODO: SCALEROTATION
		for(rotation_key_t key : submotion.scale_rotation_keys) {
			if (key.time > max_time) {
				max_time = key.time;
			}
		}
	
		//TODO: why is this hack needed to make the animations work?
		if (anim->track_find_key(pos_id, 0) != -1) {
			anim->track_remove_key_at_time(pos_id, 0);
		}
		if (anim->track_find_key(rot_id, 0) != -1) {
			anim->track_remove_key_at_time(rot_id, 0);
		}
		if (anim->track_find_key(scale_id, 0) != -1) {
			anim->track_remove_key_at_time(scale_id, 0);
		}
		//TODO: if you pay close attention, there's still a small jump along the loop point
		// though this is visible in vic2 aswell
	
		//add the "default" positions/rotation/... as keys
		anim->position_track_insert_key(pos_id, 0, vec3d_to_godot(submotion.packed.pose_position, true));
		anim->rotation_track_insert_key(rot_id, 0, submotion.pose_rotation);
		anim->scale_track_insert_key(scale_id, 0, vec3d_to_godot(submotion.packed.pose_scale));
	
	
		return max_time;
	}
	
	//a variable string or stringname with %s makes godot fail silently if placed outside a function
	
	static godot::Ref<godot::Animation> _load_xsm_animation(godot::Ref<godot::FileAccess> const& file) {
	
		read_xsm_header(file);
	
		xsm_metadata_t metadata;
		bone_animation_t bone_anim;
		std::vector<node_submotion_t> node_submotions;
	
		bool res = true;
	
		while(!file->eof_reached()) {
			chunk_header_t header = {};
			if (!read_chunk_header(file, header)) { break; }
			if (header.type == 0xC9) {
				read_xsm_metadata(file, metadata, header.version); //in zulu_moving.xsm, this is v1
			}
			else if (header.type == 0xC8) {
				read_node_submotion(file,&node_submotions);
			}
			else if (header.type == 0xCA) {
				read_xsm_bone_animation(file, bone_anim, header.version);
			}
			else {
				godot::UtilityFunctions::print(
				godot::vformat("Unsupported XSM chunk: file: %s, type = %x, length = %x, version = %d at %x", file->get_path(), header.type, header.length, header.version, file->get_position())
				);
				
				// Skip unsupported chunks
				if (header.length + file->get_position() < file->get_length()) {
					godot::PackedByteArray buf = file->get_buffer(header.length);
					godot::UtilityFunctions::print(buf);
				}
				else {
					res = false;
					break;
				} 
			}
		}
	
		float animation_length = 0.0;
		godot::Ref<godot::Animation> anim = godot::Ref<godot::Animation>();
		anim.instantiate();
	
		anim->set_step(1.0/metadata.packed.fps);
		anim->set_loop_mode(godot::Animation::LOOP_LINEAR);
	
		if (res == false) {
			return anim; //exit early if reading the chunks in failed
		}
	
		for(skeletal_submotion_t submotion : bone_anim.submotions) {
			float submotion_len = add_submotion(anim, submotion);
			if (submotion_len > animation_length) {
				animation_length = submotion_len;
			}
		}
	
		for(node_submotion_t submotion : node_submotions) {
			float submotion_len = add_submotion(anim, submotion);
			if (submotion_len > animation_length) {
				animation_length = submotion_len;
			}
		}
	
		anim->set_length(animation_length);
	
		return anim;
	}
}