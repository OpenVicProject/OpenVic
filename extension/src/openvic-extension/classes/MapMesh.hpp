#pragma once

#include <godot_cpp/classes/primitive_mesh.hpp>

namespace OpenVic {
	class MapMesh : public godot::PrimitiveMesh {
		GDCLASS(MapMesh, godot::PrimitiveMesh);

		float aspect_ratio = 2.0f, repeat_proportion = 0.5f;
		int32_t subdivide_w = 0, subdivide_d = 0;

	protected:
		static void _bind_methods();

	public:
		void set_aspect_ratio(float ratio);
		float get_aspect_ratio() const;

		void set_repeat_proportion(float proportion);
		float get_repeat_proportion() const;

		void set_subdivide_width(int32_t divisions);
		int32_t get_subdivide_width() const;

		void set_subdivide_depth(int32_t divisions);
		int32_t get_subdivide_depth() const;

		godot::AABB get_core_aabb() const;
		bool is_valid_uv_coord(godot::Vector2 const& uv) const;

		godot::Array _create_mesh_array() const override;
	};
}
