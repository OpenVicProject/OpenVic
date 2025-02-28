#pragma once
#include <cstdint>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
//#include "openvic-simulation/utility/Logger.hpp"
//#include "openvic-extension/utility/Utilities.hpp"
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/quaternion.hpp>

namespace OpenVic {
	#pragma pack(push)
	#pragma pack(1)

	struct chunk_header_t {
		int32_t type;
		int32_t length;
		int32_t version;
	};

	struct vec2d_t { //not a real datatype in the files. Just using it for convenience
		float x;
		float y;
	};

	struct vec3d_t {
		float x;
		float y;
		float z;
	};

	struct vec4d_t {
		float x;
		float y;
		float z;
		float w;
	};

	struct quat_v1_t {
		float x;
		float y;
		float z;
		float w;
	};

	struct quat_v2_t { // divide by 32767 to get proper quat
		int16_t x;
		int16_t y;
		int16_t z;
		int16_t w;
	};

	struct matrix44_t {
		vec4d_t col1;
		vec4d_t col2;
		vec4d_t col3;
		vec4d_t col4;
	};

	struct color_32_t {
		int8_t r;
		int8_t g;
		int8_t b;
		int8_t a;
	};

	struct color_128_t {
		int32_t r;
		int32_t g;
		int32_t b;
		int32_t a;
	};

	#pragma pack(pop)

	static bool read_string(godot::Ref<godot::FileAccess> const& file, godot::String& str, bool replace_chars = true, bool log = false) {
		//string = uint32 len, char[len]
		uint32_t length = file->get_32();
		if (file->get_length() - file->get_position() < length) {
			return false;
		}

		str = file->get_buffer(length).get_string_from_ascii();
		if (log) {
			godot::UtilityFunctions::print(vformat("before %s", str));
		}
		if (replace_chars) {
			str = str.replace(":", "_");
			str = str.replace("\\", "_");
			str = str.replace("/", "_");
		}
		if (log) {
			godot::UtilityFunctions::print(vformat("after %s", str));
		} 
		return true;
	}

	template<typename T>
	static bool read_struct(godot::Ref<godot::FileAccess> const& file, T& t) {
		if (file->get_length() - file->get_position() < sizeof(T)) {
			return false;
		}
		bool res = file->get_buffer(reinterpret_cast<uint8_t*>(&t), sizeof(t)) == sizeof(t);
		return res;
	}

	//Warning: works on the assumption of it being a packed struct being loaded into the array
	template<typename T>
	static bool read_struct_array(godot::Ref<godot::FileAccess> const& file, std::vector<T>& t, uint32_t size) {
		if (file->get_length() - file->get_position() < size*sizeof(T)) {
			return false;
		}
		t.resize(size*sizeof(T));
		bool res = file->get_buffer(reinterpret_cast<uint8_t*>(t.data()), sizeof(T)*size) == sizeof(t);
		return res;
	}

	static bool read_chunk_header(godot::Ref<godot::FileAccess> const& file, chunk_header_t& header) {
		//ERR_FAIL_COND_V(!read_struct(file, header), false);
		bool res = read_struct(file, header);

		//godot::UtilityFunctions::print(
		//	vformat("XAC/XSM chunk: type = %x, length = %x, version = %d, successful? %s", header.type, header.length, header.version, res)
		//);

		return res;
	}

	static godot::Vector2 vec2d_to_godot(vec2d_t const& vec2_in) {
		godot::Vector2 vec2_out = {
			vec2_in.x,
			vec2_in.y
		};
		return vec2_out;
	}

	static godot::Vector3 vec3d_to_godot(vec3d_t const& vec3_in, bool is_position = false) {
		godot::Vector3 vec3_out = {
			vec3_in.x,
			vec3_in.y,
			vec3_in.z
		};
		if (is_position) {
			vec3_out.x *= -1;
		}
		return vec3_out;
	}

	static godot::Vector4 vec4d_to_godot(vec4d_t const& vec4_in) {
		godot::Vector4 vec4_out = {
			vec4_in.x,
			vec4_in.y,
			vec4_in.z,
			vec4_in.w
		};
		return vec4_out;
	}

	static godot::Quaternion quat_v1_to_godot(quat_v1_t const& quat_in) {
		godot::Quaternion quat_out = godot::Quaternion(
			quat_in.x,
			-quat_in.y,
			-quat_in.z,
			quat_in.w
		);
		if (!quat_out.is_normalized()) {
			quat_out = godot::Quaternion();
		}
		return quat_out;
	}

	static godot::Quaternion quat_v2_to_godot(quat_v2_t const& quat_in) {
		static const float scale = 32767;
		godot::Quaternion quat_out = godot::Quaternion(
			static_cast<float>(quat_in.x) / scale,
			static_cast<float>(-quat_in.y) / scale,
			static_cast<float>(-quat_in.z) / scale,
			static_cast<float>(quat_in.w) / scale
		);
		if (!quat_out.is_normalized()) {
			quat_out = godot::Quaternion();
		}
		
		return quat_out;
	}

	static godot::Color color_32_to_godot(color_32_t const& color_in) {
		static const float scale = 256;
		godot::Color color_out = {
			(float)color_in.r / scale, 
			(float)color_in.g / scale, 
			(float)color_in.b / scale, 
			(float)color_in.a / scale
		};
		return color_out;
	}

	//TODO: verify this conversion is correct >> don't think it is
	static godot::Color color_128_to_godot(color_128_t const& color_in) {
		static const double scale = 2147483647;
		godot::Color color_out = {
			static_cast<float>(color_in.r / scale),
			static_cast<float>(color_in.g / scale),
			static_cast<float>(color_in.b / scale),
			static_cast<float>(color_in.a / scale)
		};
		return color_out;
	}

	static godot::Color vec4d_to_godot_color(vec4d_t const& color_in) {
		godot::Color color_out = {
			color_in.x,
			color_in.y,
			color_in.z,
			color_in.w
		};
		return color_out;
	}
}