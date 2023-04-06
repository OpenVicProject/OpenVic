#pragma once
#include "Good.hpp"

#include <vector>
#include <godot_cpp/classes/object.hpp>

namespace OpenVic2 {
	class LoadGoods {
		private:
			static bool extract_property_from_json(godot::Variant const& variant, std::vector<Good>& goods, int32_t index);

		public:
			static std::vector<Good> load_goods_from_disk(godot::String const& file_path);
	};
}