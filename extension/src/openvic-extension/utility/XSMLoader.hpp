#include <cstdint>
//#include <string_view>
#include <vector>
//#include "Utilities.hpp"
#include "XACUtilities.hpp"
#include "godot_cpp/classes/animation.hpp"
#include "godot_cpp/variant/packed_vector3_array.hpp"
#include "godot_cpp/variant/packed_vector4_array.hpp"
//#include "openvic-simulation/utility/Logger.hpp"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "openvic-simulation/utility/Logger.hpp"
//#include "openvic-extension/utility/Utilities.hpp"

using namespace godot;
using namespace OpenVic;

static constexpr uint32_t XSM_FORMAT_SPECIFIER = ' MSX'; /* Order reversed due to litte endian */
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

struct xsm_metadata_v2_pack {
	float unused;
	float max_acceptable_error;
	int32_t fps;
	uint8_t exporter_version_major;
	uint8_t exporter_version_minor;
	uint16_t pad;
};

struct position_key {
	vec3d_t position;
	float time;
};

struct rotation_key_v2 {
	quat_v2_t rotation;
	float time;
};

struct rotation_key_v1 {
	quat_v1_t rotation;
	float time;
};

struct scale_key {
	vec3d_t scale;
	float time;
};

struct scale_rotation_key_v2 {
	quat_v2_t rotation;
	float time;
};

struct scale_rotation_key_v1 {
	quat_v1_t rotation;
	float time;
};

struct skeletal_submotion_v2_pack {
	quat_v2_t pose_rotation;
	quat_v2_t bind_pose_rotation;
	quat_v2_t pose_scale_rotation;
	quat_v2_t bind_pose_scale_rotation;
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

struct skeletal_submotion_v1_pack {
	quat_v1_t pose_rotation;
	quat_v1_t bind_pose_rotation;
	quat_v1_t pose_scale_rotation;
	quat_v1_t bind_pose_scale_rotation;
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

#pragma pack(pop)

struct xsm_metadata {
	xsm_metadata_v2_pack packed;
	String source_app;
	String original_file_name;
	String export_date;
	String motion_name;
};

// structs which can't be read directly
struct skeletal_submotion_v2 {
	skeletal_submotion_v2_pack packed;
	String node_name;
	std::vector<position_key> position_keys;
	std::vector<rotation_key_v2> rotation_keys;
	std::vector<scale_key> scale_keys;
	std::vector<scale_rotation_key_v2> scale_rotation_keys;
};

struct skeletal_submotion_v1 {
	skeletal_submotion_v1_pack packed;
	String node_name;
	std::vector<position_key> position_keys;
	std::vector<rotation_key_v1> rotation_keys;
	std::vector<scale_key> scale_keys;
	std::vector<scale_rotation_key_v1> scale_rotation_keys;
};

struct bone_animation_v2 {
	int32_t submotion_count;
	std::vector<skeletal_submotion_v2> submotions;
};

struct bone_animation_v1 {
	int32_t submotion_count;
	std::vector<skeletal_submotion_v1> submotions;
};

static bool read_xsm_header(Ref<FileAccess> const& file) {
	xsm_header_t header;
	ERR_FAIL_COND_V(!read_struct(file, header), false);

	Logger::info("XSM file version: ", header.version_major, ".", header.version_minor," big endian?: ",header.big_endian);

	ERR_FAIL_COND_V_MSG(
		header.format_identifier != XSM_FORMAT_SPECIFIER, false, vformat(
			"Invalid XSM format identifier: %x (should be %x)", header.format_identifier, XSM_FORMAT_SPECIFIER
		)
	);

	ERR_FAIL_COND_V_MSG(
		header.version_major != XSM_VERSION_MAJOR || header.version_minor != XSM_VERSION_MINOR, false, vformat(
			"Invalid XSM version: %d.%d (should be %d.%d)",
			header.version_major, header.version_minor, XSM_VERSION_MAJOR, XSM_VERSION_MINOR
		)
	);

	ERR_FAIL_COND_V_MSG(
		header.big_endian != 0, false, "Invalid XSM endianness: big endian (only little endian is supported)"
	);

	return true;
}

static bool read_xsm_metadata(Ref<FileAccess> const& file, xsm_metadata& metadata) {
	ERR_FAIL_COND_V(!read_struct(file, metadata.packed), false);
	bool res = read_string(file, metadata.source_app);
	res &= read_string(file, metadata.original_file_name);
	res &= read_string(file, metadata.export_date);
	res &= read_string(file, metadata.motion_name);

	return res;
}

static bool read_skeletal_submotion_v1(Ref<FileAccess> const& file, skeletal_submotion_v1& submotion){
	ERR_FAIL_COND_V(!read_struct(file, submotion.packed), false);
	bool res = read_string(file, submotion.node_name);
	res &= read_struct_array(file,submotion.position_keys,submotion.packed.position_key_count);
	res &= read_struct_array(file,submotion.rotation_keys,submotion.packed.rotation_key_count);
	res &= read_struct_array(file,submotion.scale_keys,submotion.packed.scale_key_count);
	res &= read_struct_array(file,submotion.scale_rotation_keys,submotion.packed.scale_rotation_key_count);
	return res;
}

//TODO: something is wrong here
static bool read_xsm_bone_animation_v1(Ref<FileAccess> const& file, bone_animation_v1& bone_animation){
	bone_animation.submotion_count = file->get_32();
	bool ret = true;

	for(int i=0; i<bone_animation.submotion_count; i++){
		skeletal_submotion_v1 submotion;
		ret &= read_skeletal_submotion_v1(file, submotion);
		bone_animation.submotions.push_back(submotion);
	}

	return true;
}

static bool read_skeletal_submotion_v2(Ref<FileAccess> const& file, skeletal_submotion_v2& submotion){
	ERR_FAIL_COND_V(!read_struct(file, submotion.packed), false);
	bool res = read_string(file, submotion.node_name);
	res &= read_struct_array(file,submotion.position_keys,submotion.packed.position_key_count);
	res &= read_struct_array(file,submotion.rotation_keys,submotion.packed.rotation_key_count);
	res &= read_struct_array(file,submotion.scale_keys,submotion.packed.scale_key_count);
	res &= read_struct_array(file,submotion.scale_rotation_keys,submotion.packed.scale_rotation_key_count);
	return res;
}

static bool read_xsm_bone_animation_v2(Ref<FileAccess> const& file, bone_animation_v2& bone_animation){
	bone_animation.submotion_count = file->get_32();
	bool ret = true;

	for(int i=0; i<bone_animation.submotion_count; i++){
		skeletal_submotion_v2 submotion;
		ret &= read_skeletal_submotion_v2(file, submotion);
		bone_animation.submotions.push_back(submotion);
	}

	return true;
}

//static const String SKELETON_NODE_PATH = "./skeleton:%s";


static Ref<Animation> _load_xsm_animation(Ref<FileAccess> const& file){
	read_xsm_header(file);

	xsm_metadata metadata;
	std::vector<bone_animation_v1> anims_v1;
	std::vector<bone_animation_v2> anims_v2;

	bool res = true;

	while(!file->eof_reached()){
		chunk_header_t header;
		if(!read_chunk_header(file, header)) break;

		if(header.type == 0xC9 && header.version == 2) read_xsm_metadata(file, metadata);
		
		else if(header.type == 0xCA && header.version == 2) {
			bone_animation_v2 anim;
			read_xsm_bone_animation_v2(file, anim);
			anims_v2.push_back(anim); //push_back
		} 
		else if(header.type == 0xCA && header.version == 1) {
			bone_animation_v1 anim;
			read_xsm_bone_animation_v1(file, anim);
			anims_v1.push_back(anim);
		} 
		else {
			UtilityFunctions::print(
			vformat("Unsupported XSM chunk: type = %x, length = %x, version = %d", header.type, header.length, header.version)
			);
			// Skip unsupported chunks
			if(header.length + file->get_position() < file->get_length()) file->get_buffer(header.length);
			else {
				res = false;
				break;
			} 
		}
	}

	float animation_length = 0.0;
	Ref<Animation> anim = Ref<Animation>();
	anim.instantiate();

	if(res == false) return anim; //exit early if reading the chunks in failed

	//NOTE: godot uses ':' to specify properties, so we replaced such characters with '_' when we read them in
	//TODO: perhaps experiment was just adding pose keys every time?

	for(bone_animation_v1 bone_anim : anims_v1){
		for(skeletal_submotion_v1 submotion : bone_anim.submotions){
			String skeleton_path = vformat("./skeleton:%s", submotion.node_name);
			// POSITION
			if(submotion.packed.position_key_count > 0) {
				int id = anim->add_track(Animation::TYPE_POSITION_3D);
				anim->track_set_path(id, skeleton_path);
				for(position_key key : submotion.position_keys){
					anim->position_track_insert_key(id, key.time, vec3d_to_godot(key.position, true));
					if(key.time > animation_length) {
						animation_length = key.time;
					}
				}
			}
			else {
				int id = anim->add_track(Animation::TYPE_POSITION_3D);
				anim->track_set_path(id, skeleton_path);
				anim->position_track_insert_key(id, 0, vec3d_to_godot(submotion.packed.pose_position, true));
			}

			// ROTATION
			if(submotion.packed.rotation_key_count > 0) {
				int id = anim->add_track(Animation::TYPE_ROTATION_3D);
				anim->track_set_path(id, skeleton_path);
				for(rotation_key_v1 key : submotion.rotation_keys){
					anim->rotation_track_insert_key(id, key.time, quat_v1_to_godot(key.rotation));
					if(key.time > animation_length) animation_length = key.time;
				}
			}
			else {
				int id = anim->add_track(Animation::TYPE_ROTATION_3D);
				anim->track_set_path(id, skeleton_path);
				anim->rotation_track_insert_key(id, 0, quat_v1_to_godot(submotion.packed.pose_rotation));
			}

			// SCALE
			if(submotion.packed.scale_key_count > 0) {
				int id = anim->add_track(Animation::TYPE_SCALE_3D);
				anim->track_set_path(id, skeleton_path);
				for(scale_key key : submotion.scale_keys){
					anim->scale_track_insert_key(id, key.time, vec3d_to_godot(key.scale));
					if(key.time > animation_length) animation_length = key.time;
				}
			}
			else {
				int id = anim->add_track(Animation::TYPE_SCALE_3D);
				anim->track_set_path(id, skeleton_path);
				anim->scale_track_insert_key(id, 0, vec3d_to_godot(submotion.packed.pose_scale));
			}
			// TODO: SCALEROTATION
		}
	}
	//Do it all again for v2 (ie. int16 style quaternions)

	for(bone_animation_v2 bone_anim : anims_v2){
		for(skeletal_submotion_v2 submotion : bone_anim.submotions){
			String skeleton_path = vformat("./skeleton:%s",submotion.node_name);
			// POSITION
			if(submotion.packed.position_key_count > 0) {
				int id = anim->add_track(Animation::TYPE_POSITION_3D);
				anim->track_set_path(id, skeleton_path);
				for(position_key key : submotion.position_keys){
					anim->position_track_insert_key(id, key.time, vec3d_to_godot(key.position, true));
					if(key.time > animation_length) animation_length = key.time;
				}
			}
			else {
				int id = anim->add_track(Animation::TYPE_POSITION_3D);
				anim->track_set_path(id, skeleton_path);
				anim->position_track_insert_key(id, 0, vec3d_to_godot(submotion.packed.pose_position, true));
			}

			// ROTATION
			if(submotion.packed.rotation_key_count > 0) {
				int id = anim->add_track(Animation::TYPE_ROTATION_3D);
				anim->track_set_path(id, skeleton_path);
				for(rotation_key_v2 key : submotion.rotation_keys){
					anim->rotation_track_insert_key(id, key.time, quat_v2_to_godot(key.rotation));
					if(key.time > animation_length) animation_length = key.time;
				}
			}
			else {
				int id = anim->add_track(Animation::TYPE_ROTATION_3D);
				anim->track_set_path(id, skeleton_path);
				anim->rotation_track_insert_key(id, 0, quat_v2_to_godot(submotion.packed.pose_rotation));
			}

			// SCALE
			if(submotion.packed.scale_key_count > 0) {
				int id = anim->add_track(Animation::TYPE_SCALE_3D);
				anim->track_set_path(id, skeleton_path);
				for(scale_key key : submotion.scale_keys){
					anim->scale_track_insert_key(id, key.time, vec3d_to_godot(key.scale));
					if(key.time > animation_length) animation_length = key.time;
				}
			}
			else {
				int id = anim->add_track(Animation::TYPE_SCALE_3D);
				anim->track_set_path(id, skeleton_path);
				anim->scale_track_insert_key(id, 0, vec3d_to_godot(submotion.packed.pose_scale));
			}
			// TODO: SCALEROTATION
		}
	}

	anim->set_length(animation_length);
	anim->set_loop_mode(Animation::LOOP_LINEAR);

	return anim;
}