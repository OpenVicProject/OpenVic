#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <openvic-simulation/interface/GFXObject.hpp>
#include <openvic-simulation/types/OrderedContainers.hpp>

//billboards, projections, and progress bar
//for now though, only billboards

namespace OpenVic {
	class MapItemSingleton : public godot::Object {
		GDCLASS(MapItemSingleton, godot::Object)

		static inline MapItemSingleton* singleton = nullptr;

	protected:
		static void _bind_methods();

	public:
		static MapItemSingleton* get_singleton();

		MapItemSingleton();
		~MapItemSingleton();

	private:
		GFX::Billboard const* get_billboard(std::string_view name, bool error_on_fail = true) const;
		bool add_billboard_dict(std::string_view name, godot::TypedArray<godot::Dictionary>& billboard_dict_array) const;
		godot::TypedArray<godot::Dictionary> get_billboards() const;
		godot::PackedVector2Array get_province_positions() const;
		int32_t get_max_capital_count() const;
		godot::PackedVector2Array get_capital_positions() const;

		godot::PackedByteArray get_crime_icons() const;
		godot::PackedByteArray get_rgo_icons() const;
		godot::PackedByteArray get_national_focus_icons() const;
	};
}
