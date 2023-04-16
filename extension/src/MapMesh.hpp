#pragma once

#include <godot_cpp/classes/primitive_mesh.hpp>

namespace OpenVic2 {
	//Note: this class inherits from godot::PrimitiveMesh, should retain godot-style function names
	class MapMesh : public godot::PrimitiveMesh {
		GDCLASS(MapMesh, godot::PrimitiveMesh)

		float aspect_ratio = 2.0f, repeat_proportion = 0.5f;
		int subdivide_w = 0, subdivide_d = 0;

	protected:
		static void _bind_methods();
		void _request_update();

	public:
		void set_aspect_ratio(const float ratio);
		float get_aspect_ratio() const;

		void set_repeat_proportion(const float proportion);
		float get_repeat_proportion() const;

		void set_subdivide_width(const int divisions);
		int get_subdivide_width() const;

		void set_subdivide_depth(const int divisions);
		int get_subdivide_depth() const;

		godot::AABB get_core_aabb() const;
		bool is_valid_uv_coord(godot::Vector2 const& uv) const;

		godot::Array _create_mesh_array() const override;
	};
}
