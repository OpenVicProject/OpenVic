#include "MapMesh.hpp"

#include <godot_cpp/templates/vector.hpp>

#include "openvic-extension/utility/ClassBindings.hpp"

using namespace godot;
using namespace OpenVic;

void MapMesh::_bind_methods() {
	OV_BIND_METHOD(MapMesh::set_aspect_ratio, { "ratio" });
	OV_BIND_METHOD(MapMesh::get_aspect_ratio);

	OV_BIND_METHOD(MapMesh::set_repeat_proportion, { "proportion" });
	OV_BIND_METHOD(MapMesh::get_repeat_proportion);

	OV_BIND_METHOD(MapMesh::set_subdivide_width, { "divisions" });
	OV_BIND_METHOD(MapMesh::get_subdivide_width);

	OV_BIND_METHOD(MapMesh::set_subdivide_depth, { "divisions" });
	OV_BIND_METHOD(MapMesh::get_subdivide_depth);

	OV_BIND_METHOD(MapMesh::get_core_aabb);
	OV_BIND_METHOD(MapMesh::is_valid_uv_coord);

	ADD_PROPERTY(
		PropertyInfo(Variant::FLOAT, "aspect_ratio", PROPERTY_HINT_NONE, "suffix:m"), "set_aspect_ratio", "get_aspect_ratio"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::FLOAT, "repeat_proportion", PROPERTY_HINT_NONE, "suffix:m"), "set_repeat_proportion",
		"get_repeat_proportion"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::INT, "subdivide_width", PROPERTY_HINT_RANGE, "0,100,1,or_greater"), "set_subdivide_width",
		"get_subdivide_width"
	);
	ADD_PROPERTY(
		PropertyInfo(Variant::INT, "subdivide_depth", PROPERTY_HINT_RANGE, "0,100,1,or_greater"), "set_subdivide_depth",
		"get_subdivide_depth"
	);
}

void MapMesh::_request_update() {
	// Hack to trigger _update_lightmap_size and _request_update in PrimitiveMesh
	set_add_uv2(get_add_uv2());
}

void MapMesh::set_aspect_ratio(float ratio) {
	aspect_ratio = ratio;
	_request_update();
}

float MapMesh::get_aspect_ratio() const {
	return aspect_ratio;
}

void MapMesh::set_repeat_proportion(float proportion) {
	repeat_proportion = proportion;
	_request_update();
}

float MapMesh::get_repeat_proportion() const {
	return repeat_proportion;
}

void MapMesh::set_subdivide_width(int32_t divisions) {
	subdivide_w = divisions > 0 ? divisions : 0;
	_request_update();
}

int32_t MapMesh::get_subdivide_width() const {
	return subdivide_w;
}

void MapMesh::set_subdivide_depth(int32_t divisions) {
	subdivide_d = divisions > 0 ? divisions : 0;
	_request_update();
}

int32_t MapMesh::get_subdivide_depth() const {
	return subdivide_d;
}

AABB MapMesh::get_core_aabb() const {
	const Vector3 size { aspect_ratio, 0.0f, 1.0f };
	return AABB { size * -0.5f, size };
}

bool MapMesh::is_valid_uv_coord(Vector2 const& uv) const {
	return 0.0f <= uv.y && uv.y <= 1.0f;
}

Array MapMesh::_create_mesh_array() const {
	Array arr;
	ERR_FAIL_COND_V(arr.resize(Mesh::ARRAY_MAX) != OK, {});

	const int32_t vertex_count = (subdivide_w + 2) * (subdivide_d + 2);
	const int32_t indice_count = (subdivide_w + 1) * (subdivide_d + 1) * 6;

	PackedVector3Array points;
	PackedVector3Array normals;
	PackedFloat32Array tangents;
	PackedVector2Array uvs;
	PackedInt32Array indices;

	ERR_FAIL_COND_V(points.resize(vertex_count) != OK, {});
	ERR_FAIL_COND_V(normals.resize(vertex_count) != OK, {});
	ERR_FAIL_COND_V(tangents.resize(vertex_count * 4) != OK, {});
	ERR_FAIL_COND_V(uvs.resize(vertex_count) != OK, {});
	ERR_FAIL_COND_V(indices.resize(indice_count) != OK, {});

	static const Vector3 normal { 0.0f, 1.0f, 0.0f };
	const Size2 uv_size { 1.0f + 2.0f * repeat_proportion, 1.0f };
	const Size2 size { aspect_ratio * uv_size.x, uv_size.y }, start_pos = size * -0.5f;

	int32_t point_index = 0, thisrow = 0, prevrow = 0, indice_index = 0;
	Vector2 subdivide_step { 1.0f / (subdivide_w + 1.0f), 1.0f / (subdivide_d + 1.0f) };
	Vector3 point { 0.0f, 0.0f, start_pos.y };
	Vector2 point_step = subdivide_step * size;
	Vector2 uv {}, uv_step = subdivide_step * uv_size;

	for (int32_t j = 0; j <= subdivide_d + 1; ++j) {
		point.x = start_pos.x;
		uv.x = -repeat_proportion;

		for (int32_t i = 0; i <= subdivide_w + 1; ++i) {
			points[point_index] = point;
			normals[point_index] = normal;
			tangents[point_index * 4 + 0] = 1.0f;
			tangents[point_index * 4 + 1] = 0.0f;
			tangents[point_index * 4 + 2] = 0.0f;
			tangents[point_index * 4 + 3] = 1.0f;
			uvs[point_index] = uv;
			point_index++;

			if (i > 0 && j > 0) {
				indices[indice_index + 0] = prevrow + i - 1;
				indices[indice_index + 1] = prevrow + i;
				indices[indice_index + 2] = thisrow + i - 1;
				indices[indice_index + 3] = prevrow + i;
				indices[indice_index + 4] = thisrow + i;
				indices[indice_index + 5] = thisrow + i - 1;
				indice_index += 6;
			}

			point.x += point_step.x;
			uv.x += uv_step.x;
		}

		point.z += point_step.y;
		uv.y += uv_step.y;
		prevrow = thisrow;
		thisrow = point_index;
	}

	arr[Mesh::ARRAY_VERTEX] = std::move(points);
	arr[Mesh::ARRAY_NORMAL] = std::move(normals);
	arr[Mesh::ARRAY_TANGENT] = std::move(tangents);
	arr[Mesh::ARRAY_TEX_UV] = std::move(uvs);
	arr[Mesh::ARRAY_INDEX] = std::move(indices);

	return arr;
}
